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
	
	
	# Zero out the total memory accumulator
	mov $0x6000, %si
	movl $0x0, %es:(%di)

# Probe memory by using int 0x15 and eax=0xE820 -- Get Memory regions starting from mmap_ent

do_e820:
	mov $0x5000, %di
	xorl %ebx, %ebx
	xorw %bp, %bp
next_entry:
	movl $0x0534D4150, %edx
	movl $0xe820, %eax
	movl $1, %es:20(%di)
	movl  $24, %ecx
	int $0x15
	jc  error
	cmpl $0x0534D4150, %eax
	jne error
	testl %ebx, %ebx
	je e820f
	
	# Save %cx because we are going to modify it
	push %cx

	# Print Address range ...

	leaw addr_range, %si
	movw addr_range_len, %cx
	call print_string
	
	clc
	movl %es:4(%di), %edx   # Base address 4 MSBs
	mov %edx, %eax
	call print_4_bytes
	movl %es:(%di), %eax    # Base address 4 LSBs
	call print_4_bytes
	
	# Don't ruin %eax
	pusha
	mov $':, %al
	call print_ascii_char
	popa
	
	# Add the 4 LSBs of length to the
	# 4 LSBs of base address
	addl %es:8(%di), %eax  
	push %eax 	         # Store the result on stack

	
	# Add the 4 MSBs of length to the
	# 4 MSBs of base address and print them
	adcl %es:12(%di), %edx
	mov %edx, %eax
	call print_4_bytes
	
	# Print the 4 LSBs
	pop %eax 
	call print_4_bytes

	# Print status message
	leaw status, %si
	movw status_len, %cx
	call print_string
	
	# Print the status number
	mov %es:16(%di), %al
	call print_byte
	call print_newl


	# Sum the length 
	# We assume that RAM < 4GB
	# so we need only the 4 LSBs
	mov %es:8(%di), %eax
	
	# Save %di
	push %di
	mov $0x6000, %di
	addl %eax, %es:(%di)
	# Restore %di
	pop %di
	
	# Restore %cx
	pop %cx
	
	jne next_entry
e820f:
	# The final length is not added
	# So add it now that we finished
	mov %es:8(%di), %eax
	
	# Save %di
	push %di
	mov $0x6000, %di
	addl %eax, %es:(%di)
	# Restore %di
	pop %di
	
	jmp print_total_mem
error:
	leaw welcome, %si
	movw welcome_len, %cx
	call print_string
	ret
	
print_total_mem:
	pusha
	
	# Print welcome message
	leaw welcome, %si
	movw welcome_len, %cx
	call print_string
	
	movw $0x6000, %di    # Address where the length is stored at
	
	# Get length from memory to %eax
	movl %es:(%di), %eax  
	
	# We need to convert the result into 
	# MB so we shift right by 20 -> division by 1MB
	shrd $20,%eax,%eax 
	and  $0xfff, %eax
	
	incl %eax              # We are off by 1 byte -> correct that
	
	# Save %eax
	push %eax
	shr $8, %eax           # Print MSB first
	call print_byte        
	
	pop %eax
	call print_byte        # Print LSB

	popa

# Print the MB string
print_MB:
	leaw munits, %si
	movw u_len, %cx
	call print_string
	call print_newl

	jmp end
	
# Print 1 ascii character
print_ascii_char:
	movb $0x0E, %ah 	# Write character to the screen
	int $0x10
	ret

# Print newline
print_newl:
	movb $'\n, %al
	call print_ascii_char
	movb $'\r, %al
	call print_ascii_char
	ret

# Print the contents of a register
print_4_bytes:
	push %eax
	pusha
	mov $4, %cx
loop_shifts:
	shld $8, %eax, %eax
	call print_byte
	loop loop_shifts
	popa
	pop %eax
	ret

# Print a whole string
print_string:
	lodsb                	# Load a byte from DS:SI
	call print_ascii_char
	loop print_string
	ret

# Print a byte -- Copied from vga16.s
print_byte:	
	pusha
	pushw %dx
	movb %al, %dl
	shrb $4, %al
	cmpb $10, %al
	jge 1f
	addb $0x30, %al
	jmp 2f
1:	addb $55, %al	 	#Add ASCII 'A' - 10 offset	
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
	popa
	ret

# Memory address to store the memory region
# counter and the regions
mmap_ent: .word 0x4ffc

total_mem: .word 0x6000

welcome: .asciz "MemOS: Welcome *** System Memory is: 0x"
welcome_len: .word . - welcome -1

addr_range:.asciz "Address range ["
addr_range_len:.word . - addr_range -1

status:.asciz "] status: "
status_len:.word . - status -1

error_msg:.asciz "******Error******"
error_msg_len:.word . - error_msg -1

munits:.asciz "MB"
u_len:.word . - munits -1

end:
	hlt
# This is going to be in our MBR for Bochs, so we need a valid signature
	.org 0x1fe
	.byte 0x55
	.byte 0xAA

