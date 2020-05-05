CC := gcc
CFLAGS := -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -I. -c
pcr: CFLAGS := -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -I. -c
AS := as
SRCS := threads.c vga.c pic.c helper.c schedule.c mem.c file_ops.c
ASM := swtch.s
OBJS = $(SRCS:.c=.o)

all: discos

pcr: discos


discos: stub.o discos.o discos.ld swtch.o $(OBJS) int_handler.o 
	ld -m elf_i386 -T discos.ld -o discos discos.o stub.o swtch.o $(OBJS) int_handler.o 

discos.o:discos.c $(OBJS)
	$(CC) $(CFLAGS) -o discos.o discos.c

.c.o:
	$(CC) $(CFLAGS) $< -o $@
int_handler.o: int_handler.s
	$(AS) --32 int_handler.s -o int_handler.o
swtch.o: swtch.s
	$(AS) --32 swtch.s -o swtch.o

stub.o: stub.s
	$(AS) --32 stub.s -o stub.o

clean:
	rm -rf discos *.o

