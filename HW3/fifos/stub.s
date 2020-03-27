
	.globl stack

	.bss
	.align 0x1000
	.comm stack, 0x1000  # Setup stack are in bss


	.data
	.align 0x4
	.skip 16384

gdt:

	.word 0,0,0,0

	.word 0x7fff     	# Max limit 
	.word  0x0000             # Base 1st 16 bits          
	.word  0x9A00
	.word  0x00C0

	.word  0x7fff
	.word  0x0000
	.word  0x9200
	.word  0x00C0

gdt_ptr:
	.word  0x800   	# Length in bytes - 3 descriptors but space for 256 linear address
	.long gdt


	.text
	.globl _start


_start:
	jmp real_start
	.align 4
	# multiboot stuff
	.long 0x1BADB002
	.long 0x00000003
	.long 0xE4524FFB 

real_start:
	lgdt gdt_ptr
	ljmp $0x08, $1f
	
1:
	movw $0x10, %ax 	# Setup 2nd descr selector from GDT
	movw %ax,%ss
	movw %ax,%ds
	movw %ax,%es
	movw %ax,%fs
	movw %ax,%gs
	
	movl $stack+0x1000, %esp     # setup 4KB stack
	
	push %ebx

	call kmain 		# Start of C code
	cli
	hlt
loop:
	jmp loop
