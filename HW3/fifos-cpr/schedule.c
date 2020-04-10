#include "types.h"
#include "threads.h"
#include "vga.h"
#include "schedule.h"
#include "helper.h"

extern pcb fifos_threads[MAX_THREADS];
extern pcb * runqueue;
extern pcb dum_dum;

#ifdef PCR
extern rpl repl_pool[MAX_REPLS];
#endif




pcb * get_current_thread()
{
	return current;
}



#ifndef PCR
void schedule () 
{
	
	asm volatile("cli");
	
	if ( current == 0 ) // we haven't chosen one yet or nothing in the queue anymore
	{
		prev_node = &dum_dum;
		current = runqueue->next; // the one that is going to run now
	}
	else {
		prev_node = current;
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
				asm volatile("hlt");
			}
		}
		

	}
	
	asm volatile("sti");
	if (current !=0 && prev_node->tid != current->tid ){

//			print_s("Context switching to the next thread\n");
		swtch(&prev_node->ctx, current->ctx);
		// after thread-yield we have to go back
	}
		
	return ;
}

#else
/*TODO: Add these functions*/

// Get a replenishment node from the pool
// Edit the resources on the node
// Check available resources on a node
// Clear and Return the node to the pool

/* Allocate an available replenishment node from the pool */ 

rpl *get_repl(void){
	for (int i = 0; i < MAX_REPLS; i++){
		if (!repl_pool[i].in_use){
			repl_pool[i].in_use = 1;
			repl_pool[i].next = 0;
			return &repl_pool[i];
		}
	}
}

/* Add resources to the list of the current thread at time = when  */

void add_resources(pcb* cur, int when, int hmuch){
	rpl *node = get_repl();
	
	if (node == 0){
		print_s("No resource available\n");
	}

	node->when = when;
	node->hmuch = hmuch;
	node->next = 0;
	
	// If the list is empty add the node at the head
	if (cur->rpl_list == 0){
		cur->rpl_list = node;
		return;
	}
	
	// Else add it to the end of the list
	rpl *tmp = cur->rpl_list;
	while (tmp->next != 0){
		tmp = tmp->next;
	}
	tmp->next = node;
	return;
}

/* Check if the current thread still has resources in order to continue to run */

int still_has_resources(rpl *list, int time, int hmuch){
	rpl *cur = list;
	while (cur != 0){
		//Check if the resource is available timewise and it has 
		// a remaining budget
		if (cur->when <=time && cur->hmuch >= hmuch)
			return 1;
		cur = cur->next;
	}

	return 0;
}

/* Remove resources from the current threads replenish list */

int remove_resources(pcb* cur, int time, int hmuch){
	rpl *tmp = cur->rpl_list;
	while (tmp != 0){
		//Check if the resource is available timewise and it has 
		// a remaining budget
		if (tmp->when <=time && tmp->hmuch >= hmuch){
			tmp->hmuch -= hmuch;
			return 1;

		}
		tmp = tmp->next;
	}

	return 0;
}

void clear_zeroed_repls(pcb *task){
	rpl *prev;
	rpl *cur;

	prev = task->rpl_list;
	cur = task->rpl_list;
	while (cur != 0){
		if (cur->hmuch == 0){
			prev->next = cur->next;
			cur->in_use = 0;
			cur = cur->next;
			continue;
		}
		prev = cur;
		cur = cur->next;
	}
}


void print_resources(){
	pcb *tmp;
	tmp = runqueue->next;
	char buf[50];
	rpl *tmp2;

	while(tmp != 0){
		itoa(buf,'d',tmp->tid);
		print_s("Thread ");
		print_s(buf);
		print_s(" ai:");
		itoa(buf,'d',tmp->ai);
		print_s(buf);
		print_s(" ci:");
		itoa(buf,'d',tmp->ci);
		print_s(buf);
		print_s(" ti:");
		itoa(buf,'d',tmp->ti);
		print_s(buf);
		print_s(" Replenishments (When,Hmuch):");
		tmp2 = tmp->rpl_list;
		while (tmp2 != 0){
			itoa(buf,'d',tmp2->when);
			print_s("(");
			print_s(buf);
			print_s(",");
			itoa(buf,'d',tmp2->hmuch);
			print_s(buf);
			print_s(") ");
			tmp2 = tmp2->next;
		}
		print_s("\n");
		tmp = tmp->next;
	}
}



void schedule () 
{
	
	char buf[50]; 			//For printing time and next scheduled thread

	asm volatile("cli");
	int found = 0; 	                //Flag to check if we have a thread that can run at this point in time
	
	//If the runqueue is empty just hlt the program
	if (runqueue->next == 0)
		asm volatile("hlt");
	
	// It means we have scheduled the dummy thread
	// Lets try again to find a thread that can run
	if (current == &dum_dum){
		prev_node = &dum_dum;
		current = runqueue->next;
	}

	/* This is meant to run only the first time */
	if ( current == 0 ) // we haven't chosen one yet
	{
		// Iterate once through the entire queue and add the resources to each thread
		for (current = runqueue->next; current != 0; current= current->next){
			add_resources(current, 0 , current->ci);
		}
		
		print_resources();
		// For the first switch we use the dummy thread
		prev_node = &dum_dum;
		current = runqueue->next; // the one that is going to run now
		remove_resources(current->rpl_list, Time, current->ai);        //Consume resources for the first thread
		current->ai++;
	}
	else {
		
		/* 
		 * First check if the current thread finished its execution
		 * Then it must be removed from the queue
		 * prev and current pointers are updated automatically
		 * Also remove any other threads that might have finished but
		 * are not removed from the queue
		 */

		while ( current!= 0 && current->status == 1){
//			print_s("Removing thread from the queue\n");
			runqueue_remove(current->tid);
			if (runqueue->next == 0){
				asm volatile("hlt");
			}
		}
		
		/*
		 * TODO Add a case for yielding depending on the ID of the thread and the time 
		 * Could use a case with modulo
		 */
		
		// Check for the current threads resources -> Can it continue to run?
		if (still_has_resources(current->rpl_list, Time, 1)){
			current->ai++; 					//Increment the used resources
		}
		// The thread has not any more resources to run -> it must switch
		else{
			// Case we are coming from a running thread
			if (current->ai > 0){
				// We must add these resources to its next period and then switch
				add_resources(current, Time + current->ti - current->ai , current->ai);
				// We must also remove the resources from its replenishment list
				remove_resources(current->rpl_list, Time, current->ai);
				// Clear any zeroed resources from the current threads replenish list
				clear_zeroed_repls(current->rpl_list);
				// Restart the currently used resources for the next time this thread is scheduled
				current->ai=0;
			}
			// Point to the next thread in line or the start of the runqueue if at the end
			prev_node = current;
			
			while (current->next != prev_node && found != 1){
				if (current->next != 0)
					current = current->next;
				else
					current = runqueue->next;
				if(still_has_resources(current->rpl_list, Time, 1)){
					// We found a thread that can run in this time frame
					found = 1;
					remove_resources(current->rpl_list, Time, 1); //Consume resources for next thread
					current->ai++;
					break;
				}

			}
			
			// No thread can run at this point so we must schedule the dummy one
			if (found == 0){
				current = &dum_dum;	
			}
		}
	}
	
	print_s("\nCurrent Time: ");
	itoa(buf,'d',Time);
	print_s(buf);
	itoa(buf,'d',current->tid);
	print_s("\nNext thread to run: ");
	print_s(buf);
	print_s("\n");
	print_resources();

	// Reenable the interrupts
	asm volatile("sti");
	// Make the switch if we dont have to switch back to ourselves
	if (current !=0 && prev_node->tid != current->tid ){

//			print_s("Context switching to the next thread\n");
		swtch(&prev_node->ctx, current->ctx);
		// after thread-yield we have to go back
	}
		
	return ;
}



#endif
