CC := gcc
CFLAGS := -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -c
AS := as

all: fifos

fifos: stub.o fifos.o fifos.ld
	ld -m elf_i386 -T fifos.ld -o fifos fifos.o stub.o

fifos.o: fifos.c
	$(CC) $(CFLAGS) -o fifos.o fifos.c

#helper.o: helper.c
#	gcc -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -c -o helper.o helper.c

#thread.o: crtn_sample.c
#	gcc -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -c -o thread.o crtn_sample.c

stub.o: stub.s
	$(AS) --32 stub.s -o stub.o

clean:
	rm -rf fifos *.o

