#include "types.h"
#include "threads.h"
#include "vga.h"
#include "schedule.h"

extern pcb fifos_threads[MAX_THREADS];
extern pcb * runqueue;

static uint32_t dstack[1024]; 			//dummy stack for the 1st context switch
int prev_tid = -1;
int Time = 0;
extern pcr schedule_const[MAX_THREADS];

pcb * get_current_thread()
{
	return current;
}




void schedule () 
{
	

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


void update_time()
{
	Time += 1;
	// for instance
	// if T_1 = 10 and C_1 = 3
	// and thread1 only ran for 2 sec previous time slot -> rc = 1
	// this means when the timer reaches second 20:
	// we have to add 3 second to rc so that for this time slot 
	// thread_1 would have 4 sec to run
	for ( int i =0 ; i < MAX_THREADS; i++ )
	if (Time % schedule_const[i].t == 0)
	{
		// the new time slot has begun so we add c to the rc
		schedule_const[i].rc += schedule_const[i].c;
	}
}

void timer ()
{
	// it is called when the time of a process has ended
	// there will be an interrupt to handle this and jump here
	// based on current_tid I know which time has ended
	schedule_const[current->tid].rc = 0;
	
	// the thread's time has ended we update rc and call pcrschedule again
	pcrschedule();

}

void pcrschedule () 
{
	

	//TODO
	
	// One case for the initial context switch : current_tid == -1
	// One case when traversing the runqeue
	// One case when the last thread exits and there are no more threads to run
	
	// need to track time the interrupt handler for timer update Time so
	// that I know what time it is in during shceudling
	// we should set a timer interrupt for each thread that is gonna run
	// based on the remaining time in rc
	
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
				// set the timer here
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
				// set the timer here before switching the context
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

