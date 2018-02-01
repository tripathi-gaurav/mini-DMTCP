hello.o: hello.c
	gcc -c -g -fno-stack-protector -Wall -Werror -fpic -o hello.o hello.c

hello:	hello.o
	gcc -g -fno-stack-protector -o hello hello.o

libckpt.so: ckpt.o
	gcc -shared -g -fno-stack-protector -o libckpt.so ckpt.o

ckpt.o:
	gcc -c -g -fno-stack-protector -Wall -Werror -fPIC -o ckpt.o ckpt.c

restart: restart.c
	gcc -g -fno-stack-protector -static -Wl,-Ttext-segment=5000000 -Wl,-Tdata=5100000 -Wl,-Tbss=5200000 -o restart restart.c

res: restart
	./restart memoryMap_miniDMTCP.bin

gdb:
		gdb --args ./restart memoryMap_miniDMTCP.bin

clean:
	rm -rf libckpt.so ckpt.o memoryMap_miniDMTCP.bin hello hello.o restart

check:	clean libckpt.so hello restart
		(sleep 3 && kill -12 `pgrep -n hello` && sleep 2 && pkill -9 hello) &
		LD_PRELOAD=`pwd`/libckpt.so ./hello
		(sleep 2 &&  pkill -9 restart) &
		make res

dist:
		dir=`basename $$PWD`; cd ..; tar cvf $$dir.tar ./$$dir; gzip $$dir.tar
