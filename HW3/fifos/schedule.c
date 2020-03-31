#include "types.h"
#include "threads.h"
#include "vga.h"
#include "schedule.h"

extern pcb fifos_threads[MAX_THREADS];
extern pcb * runqueue;

static uint32_t dstack[1024]; 			//dummy stack for the 1st context switch


pcb * get_current_thread()
{
	return current;
}
void schedule () {
	int prev_tid = -1;
	

	/*TODO*/
	/*
	 * One case for the initial context switch : current_tid == -1
	 * One case when traversing the runqeue
	 * One case when the last thread exits and there are no more threads to run
	 */

	for (;;)
	{
		if ( current == 0 ) // we haven't chosen one yet or nothing in the queue anymore
		{
			current = runqueue->next; // the one that is going to run now

			// Create a dummy first context to pass to swtch
			struct context *dummy = (struct context *) (&dstack[1021] - sizeof(struct context *));
			// Check if we have an available in the queue
			if (current != 0)
			{
				print_s("Context switch to first thread\n");
				swtch(&dummy, fifos_threads[current->tid].ctx);
			}
			// We' never get here
		}
		else {
			int prev_id = current->tid;
			if ( current->next == 0 )
			{
				print_s("Going to the start of the queue\n");
				// we reached the end of list
				//  go back to the beginning again
				current = runqueue->next;
			}
			else
			{
				print_s("Going to the next node in the queue\n");
				current = current->next;
			}
			// It means the thread was killed!!!
			while ( current != 0 && current->status == 1)
				runqueue_remove(current->tid);
			
			if (current !=0){
				print_s("Context switching to the next thread\n");
				swtch(&fifos_threads[prev_tid].ctx, fifos_threads[current->tid].ctx);
			}
	
		}
	
	}
}
