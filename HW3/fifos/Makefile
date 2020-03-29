CC := gcc
CFLAGS := -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -I. -c
AS := as
SRCS := threads.c vga.c helper.c schedule.c 
OBJS = $(SRCS:.c=.o)
all: fifos

fifos: stub.o fifos.o fifos.ld $(OBJS)
	ld -m elf_i386 -T fifos.ld -o fifos fifos.o stub.o $(OBJS)
fifos.o:fifos.c $(OBJS)
	$(CC) $(CFLAGS) -o fifos.o fifos.c $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@
#helper.o: helper.c
#	gcc -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -c -o helper.o helper.c

#thread.o: crtn_sample.c
#	gcc -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -c -o thread.o crtn_sample.c

stub.o: stub.s
	$(AS) --32 stub.s -o stub.o

clean:
	rm -rf fifos *.o

