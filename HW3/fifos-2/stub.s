
	.globl stack
	.globl idt

	.bss
	.align 0x1000
	.comm stack, 0x1000  # Setup stack are in bss
	.comm idt, 0x800


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



idt_ptr:
	.short 0x7ff      #Length in bytes - 256 descriptors
	.long idt         #Linear address

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

	# IDT stuff
	movl $idt, %edi
	movl $0x30, %ecx          # 48 Entries
	movl $0x080000, %edx
	movl $int_table, %esi

1:
	lodsl
	movw %ax, %dx

	movw $0xEE00, %ax         # DPL = ring 3
	movl %edx, (%edi)

	addl $4, %edi
	stosl

	loop 1b

	lidt idt_ptr


	call kmain 		# Start of C code
	cli
	hlt
loop:
	jmp loop
