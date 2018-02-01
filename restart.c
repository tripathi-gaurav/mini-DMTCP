#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include "ckpt_helper.h"
#include <ucontext.h>
#include <sys/mman.h>

#define ADDR 0x5300000
#define SIZE 0x100000

void unmapCurrentStack();
void restore_memory();
struct MemoryRegion* getMemoryRegionOfRestartStack(int fd);
void restoreCheckpoint();
int restoreMemoryFromImage(int fd);

//globals
ucontext_t ucp_ref;	//context to switch
struct MemoryRegion memoryRegion;
char chkpt_img[1000];
int fdOfRestoreImage;	//fd to checkpoint image
char input_c;


int main(int argc, char* argv[]){
	if( argc == 1){
		printf("mini-DMTCP restart program. Usage ./restart <checkpoint_image>");
	}
	sprintf(chkpt_img, "%s", argv[1]);
	void *ptr = mmap((void *)ADDR, SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE, -1, 0);
	void * new_ptr = ptr + SIZE;
	asm volatile ("mov %0,%%rsp" : : "g" (new_ptr) : "memory");
	//asm volatile ("mov %0,%%rsp" : : "g" (ptr) : "memory");
	restore_memory();
	return 0; //will never be called
}

void restore_memory(){
	//unmap current stack as it could conflict with the restored stack
	unmapCurrentStack();
	restoreCheckpoint();
}

void restoreCheckpoint(){
	int fdOfCkptImage = open(chkpt_img, O_RDONLY);
	if( fdOfCkptImage < 0){
		printf("Error accessing checkpoint image.");
		exit(-1);
	}
	// read context of image
	int res = read(fdOfCkptImage, &ucp_ref, sizeof(ucp_ref));
	if(res < 0){
		printf("Error reading context from image.");
		//printf(strerr(errno));
		exit(-1);
	}
	res = restoreMemoryFromImage(fdOfCkptImage);
	if(res<0){
		printf("The hell with it.");
		exit(-1);
	}
	//proceed to context switch
	res = close(fdOfCkptImage);
	if(res<0){
		printf("Failed to close chkpt file. -.-");
		exit(-1);
	}
	setcontext(&ucp_ref);

}

int restoreMemoryFromImage(int fd){
	int res;

	while(read(fd, &input_c, 1) > 0){
		lseek(fd, -1, SEEK_CUR);

		res = read(fd, &memoryRegion, sizeof(struct MemoryRegion));
		unsigned long long int length = memoryRegion.endAddr - memoryRegion.startAddr;
		printf("==new==\nlen=%llu\n", length);
		printf("startAddr: %llu\n", (unsigned long long int) memoryRegion.startAddr);
		printf("endAddr: %llu\n", (unsigned long long int) memoryRegion.endAddr);
		printf("read perm: %d\n", memoryRegion.isReadable);

		//OK till here
		char *p = mmap(memoryRegion.startAddr, length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE, -1, 0);
		/*
		char *p;
		if(strstr(memoryRegion.location, "stack") == NULL){
			printf("mapping others");
			p = mmap(memoryRegion.startAddr, length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE, -1, 0);
		}else{
			printf("mapping the stack");
			p = mmap(memoryRegion.startAddr, length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE | MAP_GROWSDOWN , -1, 0);
		}
		*/
		printf("%p-%p\t%p\t%s\n",memoryRegion.startAddr,memoryRegion.endAddr,p,memoryRegion.location);
		fflush(stdout);
		if(res <0){
			printf("Error mapping chkpt memory.\n");
			printf("%s\n", strerror(errno));
			//don't map to res. map to void* or char*
			exit(-1);
		}


		res = read(fd, memoryRegion.startAddr, length);
		printf("aaya.");
		if(res < 0){
			printf("Error _reading_ chkpt memory.");
			printf("%s\n", strerror(errno));
			//exit(-1);
			break;
		}
		int permissions = 0;
		if(memoryRegion.isReadable){
			permissions |= PROT_READ;
		}
		if(memoryRegion.isWriteable){
			permissions |= PROT_WRITE;
		}
		if(memoryRegion.isExecutabl){
			permissions |= PROT_EXEC;
		}

		res = mprotect(memoryRegion.startAddr, length, permissions);
		if(res < 0){
			printf("Error protecting memory region.");
			exit(-1);

		}
	}
	return res;
}

void unmapCurrentStack(){

	int fdForRestartProg;
	char filename[1000];
	struct MemoryRegion* mr;

	unsigned long long int length;

	fdForRestartProg = open("/proc/self/maps", O_RDONLY);
	if(fdForRestartProg < 0){
		printf("Error opening restart process's maps.");
		return;
	}

	//fetch the address region of old stack
	mr = getMemoryRegionOfRestartStack(fdForRestartProg);
	//unmap the old stack to avoid conflict
	close(fdForRestartProg); //maybe for safety, could do if(close(fd)<0){exit(-1);}
	length = (unsigned long long int) mr->endAddr - (unsigned long long int) mr->startAddr;

	printf("start addr: %llu\nend addr: %llu\nlength: %llu\n", mr->startAddr, mr->endAddr,length);
	//exit(0);
	int res = munmap(mr->startAddr, length);

	printf("unmap old stack: %d", res);

}

struct MemoryRegion* getMemoryRegionOfRestartStack(int fd){
	//TODO: it's just looking for a line with stack word in it.
	// need to target the last column
	//function is always returning a memoryregion. could be dangerous
	struct MemoryRegion* mr;
	char* lineToParse = getLine(fd);
	while( lineToParse[0] != '\0'){
		printf("%s", lineToParse);
		mr = parseLineToMemoryRegion(lineToParse);

		if(strstr(lineToParse, "[stack]") != NULL){

			return mr;
		}
		lineToParse = getLine(fd);
	}
	return mr;
}
