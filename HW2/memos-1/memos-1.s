# ** Copyright Rich West, Boston University **
#	
# Test vga settings using real mode
# This dumps various register values used for setting up a video
# mode such as 16 color 640x480.
#	
# Originally developed as a bootup routine to reverse-engineer a VGA-based
# video driver for the Quest OS, to support Pacman.
# If it can't play Pacman it's not a proper OS!	
 

	.globl _start
	
	.code16

_start:
	movw $0x9000, %ax
	movw %ax, %ss
	xorw %sp, %sp


	movw $0x7C0, %dx # Load the MBR ar 0x7C00
	movw %dx, %ds    # BIOS loads the MBR at 0x7C00

	leaw msg, %si
	movw msg_len, %cx

write_byte:
	
	lodsb 		# Load a byte from DS:SI
	movb $0x0E, %ah # Write character to the screen
	int $0x10
	loop write_byte


# Write "0x" to screen....
	movb $'0, %al  # '0 is ASCII(0)
	movb $0x0E, %ah
	int $0x10
	movb $'x, %al  # 'x is ASCII(x)
	movb $0x0E, %ah
	int $0x10
	

# Probe memory by using int 0x15 and eax=0xE820   !!!!!! Not yet complete

do_e820:
	movw $0x8004, %di
	xorl %ebx, %ebx
	xorw %bp, %bp
	movl $0x0534D4150, %edx
	movl $0xe820, %eax
	movl $1, %es:20(%di)
	movl  $24, %ecx
	int $0x15
	jc  error
	movl $0x0534D4150, %edx
	cmpl %edx, %eax
	jne error
	testl %ebx,%ebx
	je error
	jmp start

next_entry:
	movl $0xe820, %eax
	movl $1, %es:20(%di)
	movl $24, %ecx
	int $0x15
	jc e820f
	movl $0x0534D4150, %edx
	movl $0xe820, %eax

start:
	jcxz skip_entry
	cmpb $20,%cl
	jbe notext
	testb $1, %es:20(%di)
	je skip_entry
notext:
	mov %ecx, %es:8(%di)
	or %ecx, %es:12(%di)
	jz skip_entry
	incw %bp
	addw $24, %di
skip_entry:
	testl %ebx, %ebx
	jne next_entry
e820f:
	mov %bp, MMAP_ENT
	clc
	jmp calc_mem
error:
	stc
	ret
	
calc_mem:
	xorl %eax,%eax
	movw $0x8004, %di
	movw %es:16(%di), %ax
	#movl %eax, %ebx
	
	#andl $0xffff0000, %eax
	#shr $4, %eax

	movw %ax, %cx
	
	mov %ah, %al
	call print

	mov %cl, %al
	call print

	#movl %ebx, %eax
	#andl $0x0000ffff, %eax

	#movw %ax, %cx 
	
	#mov %ah, %al
	#call print

	#mov %cl, %al
	#call print

	jmp end

#	;leaw munits, %si
#	;movw u_len, %cx

#;write_byte2:

#	;lodsb 		# Load a byte from DS:SI
#	;movb $0x0E, %ah # Write character to the screen
#	;int $0x10
#	;loop write_byte2


print:	pushw %dx
	movb %al, %dl
	shrb $4, %al
	cmpb $10, %al
	jge 1f
	addb $0x30, %al
	jmp 2f
1:	addb $55, %al	 #Add ASCII 'A' - 10 offset	
2:	movb $0x0E, %ah
	movw $0x07, %bx
	int $0x10

	movb %dl, %al
	andb $0x0f, %al
	cmpb $10, %al
	jge 1f
	addb $0x30, %al
	jmp 2f
1:	addb $55, %al		
2:	movb $0x0E, %ah
	movw $0x07, %bx
	int $0x10
	popw %dx
	ret


MMAP_ENT: .word 0x8000

msg: 	.asciz "MemOS: Welcome **** System Memory is: "
msg_len:.word . - msg

addr_range: .asciz "Address range "
addr_range_len:.word . - addr_range

status: .asciz "status"
sta_len:.word . - status

munits: .asciz "KB"
u_len:  .word . - munits



end:
	hlt
# This is going to be in our MBR for Bochs, so we need a valid signature
	.org 0x1FE

	.byte 0x55
	.byte 0xAA

# To test:	
# as --32 vga16.s -o vga16.o
# ld -T vga.ld vga16.o -o vga16
# dd bs=1 if=vga16 of=vga16_test skip=4096 count=512
# bochs -qf bochsrc-vga
	
