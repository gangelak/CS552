/* Taken from Rich West's boot-sample code*/

	.globl stack
	.globl idt

	.bss
	.align 0x1000
	.comm stack, 0x1000  // Setup stack are in bss
	.comm idt, 0x100     //  2KB for IDT


	.data
	.align 0x4


/* Global Descriptor Table setup */ 
gdt:
	/* Checkout Wikipedia */

	// Null descriptor -> First entry
	.quad $0x0
	
	// Code Segment -->  Base 0x04000000
	.short $0xffff     	//Max limit 
	.short $0x0             //Base 1st 16 bits          
	.byte  $0x0
	.byte  $0x9a
	.byte  $0x4f            // Granularity in MBs 
	.byte  $0x04

	// Data segment -->  Base 0x08000000
	.short $0xffff
	.short $0x0
	.byte  $0x0
	.byte  $0x92
	.byte  $0x4f
	.byte  $0x08

gdt_ptr:
	.short $0x7ff    	//Length in bytes - 3 descriptors but space for 256 linear address
	.long gdt

idt_prt:
	.short $0x7ff 		//Length - 256 descriptors
	.long idt

	.text
	.global _start


_start:
	jmp real_start
	.align 4
	# multiboot stuff
	.long 0x1BADB002
	.long 0x00000003
	.long 0xE4524FFB 

real_start:
	
	/* Courtecy of Rich West */

	/*
		Switch to our own GDT
	*/
	
	lgdt gdt_ptr
	ljmp $0x08, $1f
	
1:
	movw $0x10, %ax 	//Setup 2nd descr selector from GDT
	movw %ax,%ss
	movw %ax,%ds
	movw %ax,%es
	movw %ax,%fs
	movw %ax,%gs
	
	/*Setup the stack*/
	movl $stack_top+0x1000, %esp     //setup 4KB stack
	
	pushl %ebx


	/*Set up IDT by constructing 64-bit interrupt descriptors*/
	movl $idt, %edi
	movl $0x30, %ecx 	//Set counter for 48 IDT entries used
	movl $0x080000, %edx 	//Used to index kernel CS
	movl $int_table,%esi

	/* 
	For now populate int_table with interrupt gate descriptors
	These clear the IF flag when invoked
	*/

1:
	lodsl 			//Value at address DS:ESI --> EAX
	movw %ax,%dx 		//Lower 16-bits of int_table address 

	movw $0xEE00, %ax 	//DPL = (ring) 3

	movl %edx,(%edi) 	//Store bottom 32-bits of descriptor in EDX into IDT

	addl $4, %edi
	stosl 			//Store top 32-bits of descriptor held in EAX into IDT;
				// EAX --> address ES:EDI

	loop 1b

	lidt idt_ptr
	call kmain 		//Start of C code
	cli
	hlt
loop:
	jmp loop
