.code16
.global _start
_start:
  cli
  movb $0x0, temp0
  movb $0x0, temp1
  movb $0x0, temp2
  movb $0x0, temp3
  movb $0x0, mem0
  movb $0x0, mem1
  movb $0x0, mem2
  movb $0x0, mem3

  call .do_e820

  mov $content, %si
  call .print_teletype

  call .print_total

  ## print units
  mov $unit, %si
  call .print_teletype

  #mov $newline, %si
  #call .print_teletype

  hlt
    
.do_e820:
  xor %ebx, %ebx
  xor %bp, %bp
1:  
  mov $0x534D4150, %edx
  mov $0xe820, %eax
  movl $1, %es:20(%di)
  mov $24, %ecx
  int $0x15
  jc failed 
  cmp $0x534D4150, %eax # success
  jne failed
#For the first call to the function, 
#point ES:DI at the destination buffer for the list. 
#Clear EBX. Set EDX to the magic number 0x534D4150. 
#Set EAX to 0xE820 (note that the upper 16-bits of EAX should be set to 0). 
#Set ECX to 24. Do an INT 0x15.

  mov $adressC1, %si
  call .print_teletype
  
  ## First uint64_t = Base address
  # higher byte of base address
  mov %es:3(%di), %ax
  call .print_vga16
  mov %es:2(%di), %ax
  call .print_vga16
  mov %es:1(%di), %ax
  call .print_vga16
  # lowest byte of base adress
  mov %es:(%di), %ax
  call .print_vga16

  
  mov $colon, %si
  call .print_teletype

  # Second uint64_t = Length of "region"
#summ length + base
  clc
  mov %es:8(%di), %dl
  mov %es:(%di), %al
  addb %dl, %al
  movb %al, temp0;

  mov %es:9(%di), %dl
  mov %es:1(%di), %al
  adcb %dl, %al
  movb %al, temp1;

  mov %es:10(%di), %dl
  mov %es:2(%di), %al
  adcb %dl, %al
  movb %al, temp2;

  mov %es:11(%di), %dl
  mov %es:3(%di), %al
  adcb %dl, %al
  movb %al, temp3;


  # Second uint64_t = Length of "region"
  movw temp3, %ax
  call .print_vga16
  movw temp2, %ax
  call .print_vga16
  movw temp1, %ax
  call .print_vga16
  movw temp0, %ax
  call .print_vga16

  mov $adressC2, %si
  call .print_teletype

  
  mov $space, %si
  call .print_teletype

  # Next uint32_t = Region "type"
  mov %es:16(%di), %ax
  call .print_vga16
    
  #mov $newline, %si
  #call .print_teletype
    
  # summation 
  movw 0, %dx
  clc
  mov %es:8(%di), %dl
  movb mem0, %al
  addb %dl, %al
  movb %al, mem0
  
  mov %es:9(%di), %dl
  movb mem1, %al
  adcb %dl, %al
  movb %al, mem1

  mov %es:10(%di), %al
  movb mem2, %dl
  adcb %al, %dl
  movb %dl, mem2

  mov %es:11(%di), %al
  movb mem3, %dl
  adcb %al, %dl
  movb %dl, mem3

##check if list is empty
  test %ebx, %ebx
  je fini

  jmp 1b

failed:
  mov $error_message, %si
  call .print_teletype
fini:
  ret

  ## print %si
  ## teletype - interrupt 10, ah=0x0E
.print_teletype:
  pusha
  xor %ax, %ax
  mov %ax, %ds         
  # zero out ds
  mov $0x0E, %ah         
  # choose int 10, E to write to cursor
1:
  lodsb
  cmp $0, %al # if no characther left
  jz ret_print
  mov $0x0E, %ah
  int $0x10
  jmp 1b
ret_print:
  popa
  ret

  ## prints %ax in hex
  ## copied from Rich West - vga16.S
.print_vga16:
  pusha
  pushw %dx

  movb %al, %dl
  shrb $4, %al
  cmpb $10, %al
  jge 1f
  addb $0x30, %al
  jmp 2f
1:  addb $55, %al   
2:  movb $0x0E, %ah
  movw $0x07, %bx
  int $0x10

  movb %dl, %al
  andb $0x0f, %al
  cmpb $10, %al
  jge 1f
  addb $0x30, %al
  jmp 2f
1:  addb $55, %al   
2:  movb $0x0E, %ah
  movw $0x07, %bx
  int $0x10
  popw %dx
  popa
  ret

.print_total:
  pusha
  mov mem3, %ah
  #call .print_vga16
  mov mem2, %al
  shr $4, %ax
  call .print_vga16
  #movw mem1, %ax
  #call .print_vga16
  #movw mem0, %ax
  #call .print_vga16
  popa
  ret


content:
  .asciz "\n\rMemOS: Welcome *** System Memory is: 0x"

error_message:
  .asciz "*****ERROR"

unit:
  .asciz " MB"

newline:
  .asciz "\n\r"

space:
  .asciz " "
colon:
  .asciz " : 0x"

adressC1:
  .asciz "\n\rAddress range [0x"
adressC2:
  .asciz "] status: "
mem0:
  .ascii "0"

mem1:
  .ascii "0"

mem2:
  .ascii "0"

mem3:
  .ascii "0"

temp0:
  .ascii "0"

temp1:
  .ascii "0"

temp2:
  .ascii "0"

temp3:
  .ascii "0"

  .org 0x1FE

  .byte 0x55
  .byte 0xAA
