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


# Probe memory:
	movw $0xE801, %ax
	int $0x15
	
	#Account for lower 1MB memory in item
	addw $0x400, %ax 

	movw %ax, %cx 
	movw %dx, %bx

	mov %ah, %al
	call print

	mov %cl, %al
	call print

	leaw munits, %si
	movw u_len, %cx

write_byte2:

	lodsb 		# Load a byte from DS:SI
	movb $0x0E, %ah # Write character to the screen
	int $0x10
	loop write_byte2

	jmp end

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


msg: 	.asciz "MemOS: Welcome **** System Memory is: "
msg_len:.word . - msg

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
	
