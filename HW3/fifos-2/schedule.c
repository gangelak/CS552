#include "types.h"
#include "threads.h"
#include "vga.h"
#include "schedule.h"

extern pcb fifos_threads[MAX_THREADS];
extern pcb * runqueue;

static uint32_t dstack[1024]; 			//dummy stack for the 1st context switch
int prev_tid = -1;


pcb * get_current_thread()
{
	return current;
}
void schedule () 
{
	
//	__asm__ volatile("cli");

	/*TODO*/
	/*
	 * One case for the initial context switch : current_tid == -1
	 * One case when traversing the runqeue
	 * One case when the last thread exits and there are no more threads to run
	 */
	
	for(;;){
		if ( current == 0 ) // we haven't chosen one yet or nothing in the queue anymore
		{
			current = runqueue->next; // the one that is going to run now

			// Create a dummy first context to pass to swtch
			struct context *dummy = (struct context *) (&dstack[1023] - sizeof(struct context *));
			// Check if we have an available in the queue
			if (current != 0)
			{
//				__asm__ volatile("sti");
	//			print_s("Context switch to first thread\n");
				swtch(dummy, fifos_threads[current->tid].ctx);
				break;
			}

			
		}
		else {
			prev_tid = current->tid;
			if ( current->next == 0 )
			{
	//			print_s("Going to the start of the queue\n");
				// we reached the end of list
				//  go back to the beginning again
				current = runqueue->next;
			}
			else
			{
	//			print_s("Going to the next node in the queue\n");
				current = current->next;
			}
			// It means the thread was killed!!!
			while ( current != 0 && current->status == 1){
	//			print_s("Removing thread from the queue\n");
				runqueue_remove(current->tid);
				if (runqueue->next == 0){
					current = 0;
					break;
				}
			}
			
			if (current !=0){

//				__asm__ volatile("sti");
	//			print_s("Context switching to the next thread\n");
				swtch(&fifos_threads[prev_tid].ctx, fifos_threads[current->tid].ctx);
				break;
				// after thread-yield we have to go back
			}

		}
		
//		__asm__ volatile("sti");
		
		// No more threads in our queue
		if (runqueue->next == 0)
			__asm__ volatile("hlt");
			

		
	}
	return ;
}

/*
void pcrschedule () 
{
	

	//TODO
	
	// One case for the initial context switch : current_tid == -1
	// One case when traversing the runqeue
	// One case when the last thread exits and there are no more threads to run
	
	
	for(;;){
		if ( current == 0 ) // we haven't chosen one yet or nothing in the queue anymore
		{
			current = runqueue->next; // the one that is going to run now

			// Create a dummy first context to pass to swtch
			struct context *dummy = (struct context *) (&dstack[1023] - sizeof(struct context *));
			// Check if we have an available in the queue
			if (current != 0)
			{
				print_s("Context switch to first thread\n");
				swtch(dummy, fifos_threads[current->tid].ctx);
				break;
			}

			
		}
		else {
			prev_tid = current->tid;
			if ( current->next == 0 )
			{
			//	print_s("Going to the start of the queue\n");
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
			while ( current != 0 && current->status == 1){
				print_s("Removing thread from the queue\n");
				runqueue_remove(current->tid);
				if (runqueue->next == 0){
					current = 0;
					break;
				}
			}
			
			if (current !=0){
				print_s("Context switching to the next thread\n");
				swtch(&fifos_threads[prev_tid].ctx, fifos_threads[current->tid].ctx);
				break;
				// after thread-yield we have to go back
			}

		}
		
		if (runqueue->next == 0)
			asm ("hlt");
			

		
	}
	return ;
}
*/
