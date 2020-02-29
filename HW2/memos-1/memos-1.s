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
	
	# Print welcome message
	leaw welcome, %si
	movw welcome_len, %cx
	call print_string

# Probe memory by using int 0x15 and eax=0xE820 -- Get Memory regions starting from mmap_ent

do_e820:
	mov $0x5000, %di
	xorl %ebx, %ebx
	xorw %bp, %bp
	movl $0x0534D4150, %edx
	movl $0xe820, %eax
	movl $1, %es:20(%di)
	movl  $24, %ecx
	int $0x15
	jc  error
	cmpl $0x0534D4150, %eax
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
	dec %bp
	mov %bp, mmap_ent
	clc
	jmp calc_total_mem
error:
	stc
	ret
	
calc_total_mem:
	pusha
	mov mmap_ent, %ax
	movw $24, %cx           
	mul %cx               # Store how many memory regions we have as 24 byte entries
	add $0x4FE8, %ax      # Go to the beginning of our last entry (We just need the sum = base_addr + length)
	movw %ax, %di 	      # Start of base address for the last entry is in di
	
	# MSBs
	movl %es:4(%di), %ebx # Get MSBs to %ebx
	# LSBs
	movl %es:(%di), %eax  # Get LSBs to %eax 
	
	# We need to convert the result into 
	# MB so we shift right by 20 -> division by 1MB
	shrd $20,%eax,%eax 
	and  $0xfff, %eax

	# Temp store upper bits to edx
	shld $12,%ebx, %ebx
	and $0xfffff000, %ebx
	or %ebx,%eax           # Lower 4 bytes ready -- RAM < 4 billion MBs
	
	incl %eax              # We are off by 1 byte -> correct that
	
	call print_4_bytes
	popa

# Print the MB string
print_MB:
	leaw munits, %si
	movw u_len, %cx
	call print_string
	call print_newl
	
print_map:
	pusha
	mov mmap_ent, %cx       # Move the memory region counter from memory in cx
	movw $0x5000, %di       # Start of the memory region information is in 0x7c005000
repeat:
	cmpw $0, %cx
	je done
	push %cx
	clc
	# Print Address range ...
	leaw addr_range, %si
	movw addr_range_len, %cx
	call print_string
	
	movl %es:8(%di), %edx   # Base address 4 MSBs
	mov %edx, %eax
	call print_4_bytes
	movl %es:12(%di), %eax    # Base address 4 LSBs
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
	
	#movl %es:28(%di), %eax
	#call print_4_bytes
	#movl %es:24(%di), %eax
	#call print_4_bytes

	# Print status message
	leaw status, %si
	movw status_len, %cx
	call print_string
	
	# Print the status number
	mov %es:16(%di), %al
	call print_byte
	call print_newl
	# Restore counter to %cx
	pop %cx
	dec %cx
	add $24, %di
	jmp repeat
done:
	popa
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

welcome: .asciz "MemOS: Welcome *** System Memory is: 0x"
welcome_len:.word . - welcome

addr_range: .asciz "Address range ["
addr_range_len:.word . - addr_range

status: .asciz "] status: "
status_len:.word . - status

munits: .asciz "MB"
u_len:  .word . - munits

end:
	hlt
# This is going to be in our MBR for Bochs, so we need a valid signature
	.org 0x1fe
	.byte 0x55
	.byte 0xAA

