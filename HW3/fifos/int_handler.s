/* Here implement the save register logic*/

.global int_table

.section .data
.align 0x4

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

timer:
	IRQ_CTX_SAVE

	/* TODO: implement the rest of the timer handler  */
