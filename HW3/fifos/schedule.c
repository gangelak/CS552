#include "types.h"
#include "threads.h"
#include "vga.h"
#include "schedule.h"

extern pcb fifos_threads[MAX_THREADS];
extern pcb * runqueue;

static uint32_t dstack[1024]; 			//dummy stack for the 1st context switch


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

/*void yield (){*/
	/*pcb* cur = get_current_stack();*/
	/*pcb* next = get_next_stack();*/
	/*print_s("yielding from ");*/
	/*char tmp[10];*/
	/*itoa(tmp, 'd', cur->tid);*/
	/*print_s(tmp);*/
	/*print_s(" to ");*/
	/*itoa(tmp, 'd', next->tid);*/
	/*print_s(tmp);*/
	/*print_s("\n");*/
	/*swtch(cur->sp, next->sp);*/
/*}*/

void schedule () {
	int prev_tid;
	//__asm volatile ("mov %%esp, %0": "=r"(num));
//	char str[10] ;
//	itoa(str, '10', num);
//	print_s("esp value is: ");
//	print_s(str);
//	print_s("\n");
	

	/*TODO*/
	/*
	 * One case for the initial context switch : current_tid == -1
	 * One case when traversing the runqeue
	 * One case when the last thread exits and there are no more threads to run
	 */

	pcb* tmp = runqueue->next;
	for (;runqueue->next != 0;)
	{

		
		/* First context switch ever */
		if (current_tid == -1){
			// Create a dummy first context to pass to swtch
			struct context *dummy = (struct context *) (&dstack[1023] - sizeof(struct context *));
			// Check if we have an available in the queue
			if (tmp != 0){
				current_tid = tmp->tid;
				swtch(&dummy, fifos_threads[current_tid].ctx);
			}
			// We' never get here

		}
		else{
			
			prev_tid = current_tid;
			
			// Go to the next thread in the queue
			if( tmp->next == 0 )
			{
				// we reached end of list
				// so we are gonna go to the head again
				tmp = runqueue->next;
			}
			else{
				tmp = tmp->next;
			}
			
			current_tid = tmp->tid;
			
			print_s("Context switching to next thread\n");
			//(tmp->entry)();
			
			swtch(&fifos_threads[prev_tid].ctx,fifos_threads[current_tid].ctx);
			//runqueue_remove(tmp->tid);
			

		}
	}
//	print_s("schedule: nothing to run\n");
	return;
}
