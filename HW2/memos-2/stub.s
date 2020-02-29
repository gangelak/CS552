.data
.align 16
.skip 16384
stack_top:

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
	# init the stack
	mov $stack_top, %esp
	push %eax
	push %ebx
	call kmain
	hlt
