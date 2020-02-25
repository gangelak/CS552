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
	leaw msg, %si
	movw msg_len, %cx
	call write_byte

 #Write "0x" to screen....
	movb $'0, %al  # '0 is ASCII(0
	call write_1_byte
	movb $'x, %al  # 'x is ASCII(x)
	call write_1_byte

# Probe memory by using int 0x15 and eax=0xE820   !!!!!! Not yet complete

do_e820:
	mov $0x5000, %di
	xorl %ebx, %ebx
	xorw %bp, %bp
	movl $0x0534D4150, %edx
	movl $0xe820, %eax
	#movl $1, %es:20(%di)
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
	#movl $1, %es:20(%di)
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
	mov %bp, MMAP_ENT
	clc
	jmp calc_mem_megs
error:
	stc
	ret
	
calc_mem_megs:
	push %ebx
	mov MMAP_ENT, %ax
	movw $24, %cx           
	mul %cx             # Store how many memory regions we have as 24 byte entries
	add $0x4FE8, %ax      # Go to the beginning of our last entry (We just need the sum = base_addr + length)
	movw %ax, %di 	    # Start of base address for the last entry is in di
	# Start with MSBs
	movl %es:4(%di), %ebx # LSBs of length to ebx (little endian -> needs rotate 3 bytes)
	#Start with LSBs
	movl %es:(%di), %eax # $ MSBs of base address to ecx (little endian -> needs rotate 3 bytes)
	
	# We need to convert the result into MB so we shift right by 20 -> division by 1MB
	shrd $20,%eax,%eax 
	and  $0xfff, %eax

	# Temp store upper bits to edx
	#mov %ebx, %edx
	shld $12,%ebx, %ebx
	and $0xfffff000, %ebx
	or %ebx,%eax        # Lower 4 bytes ready
	
	add $1, %eax        # We are off by 1 byte -> correct that
	#shrd $20, %edx, %eax
	#and $0xfff, %eax
	#adc $0, %eax

	#call print_4_bytes
	#mov %ebx, %eax
	call print_4_bytes
	pop %ebx

print_MB:
	leaw munits, %si
	movw u_len, %cx
	call write_byte
	call print_newl
	
print_map:
	push %di
	push %bx
	mov MMAP_ENT, %cx
	#movw $0, %dx
	#movw $24, %cx           
	#mul %cx             # Store how many memory regions we have as 24 byte entries
	#mov %cx, %ax 
	movw $0x5000, %di
	cld
repeat:
	cmpw $0, %cx
	je done
	# Print Address range ...
	push %cx
	leaw addr_range, %si
	movw addr_range_len, %cx
	call write_byte
	pop %cx

	movl %es:4(%di), %edx
	mov $'[, %al
	call write_1_byte
	mov %edx, %eax
	call print_4_bytes
	movl %es:(%di), %eax
	call print_4_bytes
	mov $':, %al
	call write_1_byte
	movl %es:28(%di), %eax
	call print_4_bytes
	movl %es:24(%di), %eax
	call print_4_bytes
	mov $'], %al
	call write_1_byte

	# Print status....
	push %cx
	leaw status, %si
	movw status_len, %cx
	call write_byte
	pop %cx

	mov %es:16(%di), %al
	call print
	call print_newl
	dec %cx
	add $24, %di
	jmp repeat
done:
	pop %bx
	pop %di
	jmp end


# Print 1 ascii character
write_1_byte:
	movb $0x0E, %ah # Write character to the screen
	int $0x10
	ret

# Print newline
print_newl:
	movb $'\n, %al
	call write_1_byte
	movb $'\r, %al
	call write_1_byte
	ret

# Print the contents of a register
print_4_bytes:
	push %eax
	push %ecx
	mov $4, %cx
loop_shifts:
	shld $8, %eax, %eax
	call print
	loop loop_shifts
	pop %ecx
	pop %eax
	ret

# Print a whole string
write_byte:
	lodsb 		# Load a byte from DS:SI
	call write_1_byte
	loop write_byte
	ret

# Print a byte
print:	
	push %eax
	pushw %dx
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
	pop %eax
	ret


MMAP_ENT: .word 0x4ffc

msg: 	.asciz "MemOS: Welcome **** System Memory is: "
msg_len:.word . - msg

addr_range: .asciz "Address range "
addr_range_len:.word . - addr_range

status: .asciz " status: "
status_len:.word . - status

munits: .asciz "MB"
u_len:  .word . - munits

end:
	hlt
# This is going to be in our MBR for Bochs, so we need a valid signature
	.org 0x1fe
	.byte 0x55
	.byte 0xAA

# To test:	
# as --32 vga16.s -o vga16.o
# ld -T vga.ld vga16.o -o vga16
# dd bs=1 if=vga16 of=vga16_test skip=4096 count=512
# bochs -qf bochsrc-vga
