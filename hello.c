#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int main(int argc, char *argv[]){
	for(int i = 0; ; i++) {
		printf("%d ", i);
		fflush(stdout);
		sleep(1);
	}
}
