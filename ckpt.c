#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include "ckpt_helper.h"
#include <ucontext.h>


int my_func();


//signal handler
void my_sig_handler(int sig);

__attribute__((constructor)) void sig_handler_constructor(){
	signal(SIGUSR2, my_sig_handler);
}

void my_sig_handler(int sig){
	signal(sig, SIG_DFL);
	printf("caught this");
	my_func();
	return;
}


int my_func(){
	char filePath[] =  "/proc/";

	my_strconcat(filePath,"self/maps");
	//get context first and then serialize
	ucontext_t ucp_ref;
	getcontext(&ucp_ref);

	int fdOfInputFile = open(filePath, O_RDONLY);
	int fdOfOutputFile = open("./memoryMap_miniDMTCP.bin", O_CREAT | O_WRONLY | O_APPEND, 0666);

	if(fdOfInputFile < 0 || fdOfOutputFile <0){
		exit(1);
	}

	write(fdOfOutputFile, &ucp_ref, sizeof(ucp_ref));
	//initialize the struct
	struct MemoryRegion* memoryRegion;

	//read the file line by line and parse
	char* lineToParse = getLine(fdOfInputFile);
	while(  lineToParse[0] != '\0' ){
		//parse the line
		printf("%s", lineToParse);
		memoryRegion = parseLineToMemoryRegion(lineToParse);

		//store into ckpt file
		//printf("isReadable? %d and location=%s\n", memoryRegion->isReadable, memoryRegion->location);
		if(memoryRegion->isReadable==1){
			//char* location = memoryRegion->location;
		//	printf("location = %s\n", location);
			if(strstr(lineToParse, "vvar") == NULL && strstr(lineToParse, "vdso") == NULL && strstr(lineToParse, "vsyscall") == NULL ){
				unsigned long int length = memoryRegion->endAddr - memoryRegion->startAddr;
				printf("Length = %lu\n", length);
				printf("writing for: %s", memoryRegion->location);
				printf("readable: %d\n", memoryRegion->isReadable);
				printf("%p-%p\n", memoryRegion->endAddr, memoryRegion->startAddr);
				ssize_t ret = write(fdOfOutputFile, memoryRegion, sizeof(struct MemoryRegion));
				assert( ret == sizeof(struct MemoryRegion));
				ret = write(fdOfOutputFile, memoryRegion->startAddr, memoryRegion->endAddr - memoryRegion->startAddr);
				assert( ret == (memoryRegion->endAddr - memoryRegion->startAddr));
			}
		}
		lineToParse = getLine(fdOfInputFile);
	}


	close(fdOfOutputFile);
	close(fdOfInputFile);

	return 0;
}
