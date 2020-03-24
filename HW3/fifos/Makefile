all: memos-2

memos-2: stub.o memos2.o memos2.ld
	ld -m elf_i386 -T memos2.ld -o memos-2 memos2.o stub.o

#helper.o: helper.c
#	gcc -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -c -o helper.o helper.c
memos2.o: memos2.c
	gcc -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -c -o memos2.o memos2.c
stub.o: stub.s
	as --32 stub.s -o stub.o
#thread.o: crtn_sample.c
#	gcc -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -c -o thread.o crtn_sample.c

clean:
	rm -rf stub.o memos2.o memos-2

