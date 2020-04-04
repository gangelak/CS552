/* Here implement the save register logic*/

	.globl int_table
	.globl timer_irq
	.globl _sleep

	.section .bss
	.align 0x4
	.comm system_timer_fractions,4
	.comm system_timer_ms, 4
	.comm IRQO_fractions, 4
	.comm IRQ0_ms, 4
	.comm IRQO_frequency, 4
	.comm PIT_reload_value,2




	.section .data
	.align 0x4

	CountDown .word 0


int_table:
	.long exception_handler0
	.long exception_handler1
	.long exception_handler2
	.long exception_handler3
	.long exception_handler4
	.long exception_handler5
	.long exception_handler6
	.long exception_handler7
	.long exception_handler8
	.long exception_handler9
	.long exception_handler10
	.long exception_handler11
	.long exception_handler12
	.long exception_handler13
	.long exception_handler14
	.long exception_handler15
	.long exception_handler16
	.long exception_handler17
	.long exception_handler18
	.long exception_handler19
	.long 0  //20
	.long 0  //21
	.long 0  //22
	.long 0  //23
	.long 0  //24
	.long 0  //25
	.long 0  //26
	.long 0  //27
	.long 0  //28
	.long 0  //29
	.long 0  //30
	.long 0  //31

	/* 0x20 - 0x2f PIC IRQs */
	.long timer //32
	.long unhandled_interrupt  //33
	.long unhandled_interrupt  //34
	.long unhandled_interrupt  //35
	.long unhandled_interrupt  //36
	.long unhandled_interrupt  //37
	.long unhandled_interrupt  //38
	.long unhandled_interrupt  //39
	.long unhandled_interrupt  //40
	.long unhandled_interrupt  //41
	.long unhandled_interrupt  //42
	.long unhandled_interrupt  //43
	.long unhandled_interrupt  //44
	.long unhandled_interrupt  //45
	.long unhandled_interrupt  //46
	.long unhandled_interrupt  //47

.section .text
.align 0x4
/* TODO: implement the rest of the timer handler  */
timer:
	pushl %eax
	pushl %ebx

	movl %eax,IRQ0_fractions
	movl %ebx, IRQ0_ms
	add system_timer_fractions, %eax
	adc system_timer_ms, %ebx


	movb $0x20, %al
	outb %al, $0x20

	popl %ebx
	popl %eax

	iretd






init_pit:
	pushad

	movl 0x010000, %eax
	cmp  $18, %ebx
	jbe gotReloadValue

	mov $1, %eax
	cmp $1193181, %ebx
	jae gotReloadValue

	mov $3579545, %eax
	mov $0, %edx
	div %ebx
	cmp $3579545 / 2, %edx
	jb l1
	inc %eax
l1:
	mov $3, %ebx
	mov $0, %edx
	div %ebx
	cmp 3 / 2, %edx
	jb l2
	inc %eax
l2:


gotReloadValue:
	push %eax
	mov %ax, PIT_reload_value
	mov %eax, %ebx

	mov $3579545, %eax
	mov $0, %edx
	div %ebx
	cmp $3579545 / 2, %edx
	jb l3
l3:
	mov $3, %ebx
	mov $0, %edx
	div %ebx
	cmp $3 / 2, %edx
	jb l4
	inc %eax
l4:
	mov %eax, IRQ0_frequency


	popl %ebx
	mov $0xDBB3A062, %eax
	mul %ebx
	shrd %eax,10(%edx)
	shrl %edx, $10

	mov %edx, IRQ0_ms
	mov %eax, IRQ0_fractions

	pushfd

	cli
	mov $00110100b, %al
	out %al, $0x43
	mov PIT_reload_value, %ax
	out %al, $0x40
	mov %ah, %al
	out %al, $0x40

	popfd
	popad
	
	ret


timer_irq:
	push %eax
	mov CountDown, %eax
	or %eax, %eax
	jz timer_done
	dec CountDown
timer_done:
	pop %eax
	iretd


_sleep:
	push %ebp
	mov %esp, %ebp
	push %eax
	mov 8(%ebp), %eax
	mov %eax, CountDown
sleep_loop:
	cli
	mov CountDown, %eax
	or %eax,%eax
	jz sleep_done
	sti
	nop
	nop
	nop
	nop
	nop
	nop
	jmp sleep_loop
sleep_done:
	sti
	pop %eax
	pop %ebp
	ret $8


