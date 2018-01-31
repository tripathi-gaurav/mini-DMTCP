hello: hello.o
	gcc -g -o -fno-stack-protector hello hello.o

hello.c:
	gcc -Wall -Werror -g -o -fno-stack-protector hello.o hello.c

libckpt.so: ckpt.o
	gcc -shared -g -fno-stack-protector -o libckpt.so ckpt.o

ckpt.o: ckpt.c 
	gcc -c -g -fno-stack-protector -Wall -Werror -fPIC -o ckpt.o ckpt.c

clean:
	rm libckpt.so ckpt.o memoryMap_miniDMTCP.bin

check:	libckpt.so hello
	#gcc -o hello hello.c
	(sleep 3 && kill -12 `pgrep -n hello` ) &
	LD_PRELOAD=`pwd`/libckpt.so ./hello	
