CC := gcc
CFLAGS := -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -I. -c
pcr: CFLAGS := -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -I. -c -D PCR
AS := as
SRCS := threads.c vga.c pic.c helper.c schedule.c
ASM := swtch.s
OBJS = $(SRCS:.c=.o)

all: fifos

pcr: fifos


fifos: stub.o fifos.o fifos.ld swtch.o $(OBJS) int_handler.o 
	ld -m elf_i386 -T fifos.ld -o fifos fifos.o stub.o swtch.o $(OBJS) int_handler.o 

fifos.o:fifos.c $(OBJS)
	$(CC) $(CFLAGS) -o fifos.o fifos.c

.c.o:
	$(CC) $(CFLAGS) $< -o $@
int_handler.o: int_handler.s
	$(AS) --32 int_handler.s -o int_handler.o
swtch.o: swtch.s
	$(AS) --32 swtch.s -o swtch.o

stub.o: stub.s
	$(AS) --32 stub.s -o stub.o

clean:
	rm -rf fifos *.o

