#include "types.h"
#include "threads.h"
#include "vga.h"
#include "schedule.h"
#include "helper.h"
extern pcb fifos_threads[MAX_THREADS];
extern pcb * runqueue;

static uint32_t dstack[1024]; 			//dummy stack for the 1st context switch
int prev_tid = -1;
uint32_t Time=0;
extern pcr schedule_const[MAX_THREADS];

pcb * get_current_thread()
{
	return current;
}


update_time()
{
	
	__asm__ volatile("cli");
	Time +=1;
	// if T_1 = 10 and C_1 = 3
	// and thread1 only ran for 2 sec previous time slot -> rc = 1
	// this means when the timer reaches second 10:
	// we have to add 3 second to rc so that for this time slot 
	// thread_1 would have 4 sec to run
	
	for ( int i =0 ; i < MAX_THREADS; i++  )
	{
	if (Time % schedule_const[i].t == 0)
	{	
	        // the new time slot has begun so we add c to the rc
		print_s("|");
		schedule_const[i].rc += schedule_const[i].c;
	}
	}
	// look out for the dummy one
	if ( current->tid > 0 )
	{
		schedule_const[current->tid].rc -= 1;
	
		if ( schedule_const[current->tid].rc <= 0)
		{
			print_s("TU");
			schedule_const[current->tid].rc = 0;
			intrp = 1;
			outb(0x20,0x20);
			asm volatile("sti");
			schedule();
		}
		else {
			outb(0x20,0x20);
			asm volatile("sti");
		}
	}
	
//	if ((Time - schedule_const[current->tid].start) == schedule_const[current->tid].rc)
//	{
		//print_s("the time is up for this thread\n");
		// means the time is up for this thread and we should switch

		schedule_const[current->tid].rc = 0;
//	}
}

void schedule () 
{
	asm volatile("cli");
	
	//TODO
	// One case for the initial context switch : current_tid == -1
	// One case when traversing the runqeue
	// One case when the last thread exits and there are no more threads to run
	
	if (runqueue->next == 0 )
		asm volatile("hlt");

	if ( current == 0 ) // we haven't chosen one yet or nothing in the queue anymore
	{
		current = runqueue->next; // the one that is going to run now

		// Create a dummy first context to pass to swtch
		struct context *dummy = (struct context *) (&dstack[1023] - sizeof(struct context *));
		// Check if we have an available in the queue
		if (current != 0)
		{
			// set the time that the threads has started
			schedule_const[current->tid].start = Time;
			outb(0x20,0x20);
			asm volatile("sti");
			swtch(dummy, fifos_threads[current->tid].ctx);	
		}

	}
	else {
		prev_tid = current->tid;
		if ( current->next == 0 )
		{	
			// we reached the end of list
			//  go back to the beginning again
			current = runqueue->next;
		}
		else
		{
			current = current->next;
		}
			// It means the thread was killed!!!
		while ( current != 0 && current->status == 1)
		{
			runqueue_remove(current->tid);
			if (runqueue->next == 0){
				current = 0;
				asm volatile("hlt");
			}
		}
		while (schedule_const[current->tid].rc == 0)
		{
			if ( current->next ==0  )
				current = runqueue->next;
			else
				current = current->next;
		}
		if (current != 0 & prev_tid != current->tid)
		{ 
			schedule_const[current->tid].start = Time;
//				char tmp[10];
//				itoa(tmp,'d', schedule_const[current->tid].rc);
//				print_s("(");
//				print_s(tmp);
//				print_s(")");

			outb(0x20,0x20);
			asm volatile("sti");
			swtch(&fifos_threads[prev_tid].ctx, fifos_threads[current->tid].ctx);	
		}
	}	
	
	return ;
}

