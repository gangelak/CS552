CC := gcc
CFLAGS := -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -I. -c
AS := as
SRCS := threads.c vga.c helper.c schedule.c 
ASM := swtch.s
OBJS = $(SRCS:.c=.o)
all: fifos

fifos: stub.o fifos.o fifos.ld swtch.o $(OBJS)
	ld -m elf_i386 -T fifos.ld -o fifos fifos.o stub.o swtch.o $(OBJS)

fifos.o:fifos.c $(OBJS)
	$(CC) $(CFLAGS) -o fifos.o fifos.c

.c.o:
	$(CC) $(CFLAGS) $< -o $@
swtch.o: swtch.s
	$(AS) --32 swtch.s -o swtch.o

stub.o: stub.s
	$(AS) --32 stub.s -o stub.o

clean:
	rm -rf fifos *.o

