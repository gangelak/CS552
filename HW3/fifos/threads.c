/* 
 * Copyright Rich West, 2011
 */

#include "helper.h"
#include "threads.h"
#include "types.h"
#include "vga.h"
#include "schedule.h"

static bool in_use[MAX_THREADS] = {0,0,0};
static uint32_t stacks[MAX_THREADS][1024];

extern pcb fifos_threads[MAX_THREADS];
extern pcr schedule_const[MAX_THREADS];
/* Get an available pcb spot from the array */

int get_pcb(){
	
	int i;
	for (i =0; i< MAX_THREADS; i++){
		if (in_use[i] == 0){
			pcb temp;
			in_use[i] = 1;
			fifos_threads[i] = temp;
			return i;
		}
	}
	return -1;
}


/* Thread functions */

void thread_yield() {
	// save state and call scheduler
	// our scheduler is wrong for now
	// we should make a runqueue like west
	// and add the threads to the end of it
	// and in the scheduler do round robin
	

	fifos_threads[get_current_thread()->tid].status = 0;
	schedule();
	return;
}
void exit_thread() {
	/* current running thread is done 
	 * so need to change status -> Status.killed
	 */
	print_s("Calling exit thread!!!\n");
	pcb * tmp = get_current_thread();
	
	in_use[tmp->tid] = 0; 			// This PCB is not use anymore

	tmp->status = 1; // means killed
	schedule();

}


static 
void thread1 () 
{
	/*int i; */
          /*int j = 0;*/
	
	print_s("Executing Thread1!\n");
  	while (1) 
  	{
	      	print_s ("<1>\n"); 
		/* Yield at this point */
	      	print_s ("Thread 1 yielding mathafuckaaaaaaaaaaaa\n"); 
		thread_yield();
		
		/*if (++j> 6)*/
		/*{*/
			/*break;*/
		/*}*/
		break;
	}

	//  done[0] = TRUE;
	print_s ("Done 1\n");
	return;
}

static
void thread2 () 
{
	/*int i;*/
	/*int j = 0;*/

	print_s("Executing Thread2!\n");
	while (1) 
	{
		print_s ("<2>\n");
		
		/* Yield at this point */
	      	print_s ("Thread 2 yielding mathafuckaaaaaaaaaaaa\n"); 
		thread_yield();
		
		break;
		/*if (++j == 10)*/
			/*break;*/
  	}
	// done[1] = TRUE;
	print_s ("Done 2\n");

	return;
}


static void thread3 () 
{
	print_s("Executing Thread 3!\n");
	while (1) 
	{
		print_s ("<3><3>\n");
		
		/* Yield at this point */
	      	print_s ("Thread 3 yielding\n"); 
		thread_yield();
		
		break;
		/*if (++j == 10)*/
			/*break;*/
  	}
	// done[1] = TRUE;
	print_s ("Done 3\n");

	return;
}
/* For debugging purposes only */

void print_context(uint32_t *stack, int tid){
	uint32_t bp = (uint32_t)fifos_threads[tid].bp;

	uint32_t *cur;
	int i = 0;
	for (cur = (uint32_t*)stack; cur <= bp; cur++){
		char buf[16];
		memset((uint32_t)buf,0,4);
		itoa(buf,'x',*cur);
		print_s(buf);
		print_s(" ");
		i++;
		if (i%4 == 0){
			print_s("\n");
			i = 0;

		}
	
	}
}


// remove the pcb with the passed tid from runqueue

void runqueue_remove(int tid)

{
	// tmp points to the thread that its next thread has tid equal to passed argument
	pcb* tmp = runqueue->next;
	pcb* btmp = runqueue;
	while (tmp != 0)
	{
		if ( tmp->tid == tid )
		{
			btmp->next = tmp->next;
			current = tmp->next;
			break;
		}

		tmp = tmp->next;
		btmp = btmp->next;
	}
	// btmp points to the thread before tmp
	// so we want the thread before temp to point to the thread after tmp
	// technically remove tmp from the linked list
	//print_s("runqueue_remove: the tid has been removed\n");
	
}

// add a thread to the runqueue
void runqueue_add(pcb* t)
{
	// runqueue is pointing to the head 
	// first time it is NULL
	if(runqueue->next == 0)
	{
		print_s("add to runqueue: first time\n");	
		runqueue->next = t;
	}
	else{
		print_s("add to runqueue: not first time\n");
		// need to iterate through pcbs to find the last one
		pcb* tmp = runqueue->next;
		while(tmp->next != 0)
		{
			print_s("iterating...\n");
			tmp = tmp->next;
		}
		// the last one is gonna point to t
		tmp->next = t;
	}
}
/*TODO add a stack*/

int thread_create(void *stack, void *func){
	int new_pcb = -1;
	
	uint16_t ds=0x10, es = 0x10, fs = 0x10, gs = 0x10;

	new_pcb = get_pcb();
	if (new_pcb == -1){
		print_s("No PCB available!\n");
		return -1;
	}
	
	/* Follow the actual layout of a process' activation record
	 * First the arguments to the function
	 * Secondly the return address of the function
	 */


	*(((uint32_t*) stack) - 0) = (uint32_t) exit_thread;
	//stack = (void*) (stack - 4);

	
	/* Found new PCB */
	fifos_threads[new_pcb].tid = new_pcb;
	fifos_threads[new_pcb].bp = (uint32_t) ((uint32_t*)stack -1);
	fifos_threads[new_pcb].entry = (uint32_t) func;
	fifos_threads[new_pcb].status = 0;
	fifos_threads[new_pcb].next = 0;
	fifos_threads[new_pcb].prev = 0;
	
	/* Create a fake initial context for the process  */

	stack = (void*) (stack - sizeof(struct context));
	fifos_threads[new_pcb].ctx = (struct context*) stack;

	fifos_threads[new_pcb].ctx->eip = fifos_threads[new_pcb].entry;
	fifos_threads[new_pcb].ctx->ebp = (uint32_t) fifos_threads[new_pcb].bp;
	fifos_threads[new_pcb].ctx->ebx = 0;
	/*fifos_threads[new_pcb].ctx->eax = 0;*/
	/*fifos_threads[new_pcb].ctx->ecx = 0;*/
	/*fifos_threads[new_pcb].ctx->edx = 0;*/
	fifos_threads[new_pcb].ctx->esi = 0;
	fifos_threads[new_pcb].ctx->edi = 0;
//	fifos_threads[new_pcb].ctx->flags = 0 | (1<<9);
	fifos_threads[new_pcb].ctx->gs = gs;
	fifos_threads[new_pcb].ctx->fs = fs;
	fifos_threads[new_pcb].ctx->es = es;
	fifos_threads[new_pcb].ctx->ds = ds;

	/* Fake an initial context for the new thread */
	fifos_threads[new_pcb].sp = (uint32_t) (((uint32_t *) stack));
	
//	print_s("Printing the context\n");
//	print_context((uint32_t*)fifos_threads[new_pcb].sp,new_pcb);
//	print_s("\n");
	// add to the run queue
	runqueue_add(&fifos_threads[new_pcb]);
	return 0;
}


void init_threads(void){
	
	runqueue->next = 0; // set up runqueue
	current = 0; // set up current running to null
	int i;
//	print_s("creating the threads\n");
	void* threads[MAX_THREADS] = {(void*)thread1, (void*)thread2, (void*)thread3};	
	for (i = 0; i < MAX_THREADS; i++){

		thread_create(&(stacks[i][1023]), threads[i]);
		
		// setup pcr for the thread
		schedule_const[i].t = 15;
		schedule_const[i].c = 5;
		schedule_const[i].rc = 0;
	}	
}



