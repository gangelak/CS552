#include "types.h"
#include "threads.h"
#include "vga.h"

extern pcb fifos_threads[MAX_THREADS];
extern pcb * runqueue;
static int current_tid;

pcb * get_current_stack() {
	pcb* tmp = runqueue->next;
	for (;tmp->next != 0; )
	{
		if (current_tid == tmp->tid)
			return &tmp;
		tmp = tmp->next;
	}
}
pcb * get_next_stack() {
	pcb* tmp = runqueue->next;
	for (;tmp->next != 0; )
	{
		if (current_tid == tmp->tid)
			return (tmp->next);
		tmp = tmp->next;
	}
}

void yield (){
	pcb* cur = get_current_stack();
	pcb* next = get_next_stack();
	swtch(cur->sp, next->sp);
}

void schedule () {
	int num;
	__asm volatile ("mov %%esp, %0": "=r"(num));
//	char str[10] ;
//	itoa(str, '10', num);
//	print_s("esp value is: ");
//	print_s(str);
//	print_s("\n");
//	

	pcb* tmp = runqueue->next;
	for (;runqueue->next != 0;)
	{
//		print_s("here in scheduler\n");
//		swtch(num, tmp->sp);
		current_tid = tmp->tid;
		(tmp->entry)();
		print_s("running the thread\n");
		runqueue_remove(tmp->tid);
		if( tmp->next == 0 )
		{
			// we reached end of list
			// so we are gonna go to the head again
			tmp = runqueue->next;
		}
		else{
			tmp = tmp->next;
		}	
	}
//	print_s("schedule: nothing to run\n");
	return;
}
