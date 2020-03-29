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
			return tmp;
		tmp = tmp->next;
	}
	return tmp;
}
pcb * get_next_stack() {
	pcb* tmp = runqueue->next;
	for (;tmp->next != 0; )
	{
		if (current_tid == tmp->tid)
			return (tmp->next);
		tmp = tmp->next;
	}
	return tmp;
}

void yield (){
	pcb* cur = get_current_stack();
	pcb* next = get_next_stack();
	print_s("yielding from ");
	char tmp[10];
	itoa(tmp, 'd', cur->tid);
	print_s(tmp);
	print_s(" to ");
	itoa(tmp, 'd', next->tid);
	print_s(tmp);
	print_s("\n");
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

//		swtch(num, tmp->sp);

		current_tid = tmp->tid;
		print_s("running the thread\n");
		(tmp->entry)();
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
