#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int main(int argc, char *argv){
	while(1){
		printf(".");
		fflush(stdout);
		sleep(1);		
	}
}
