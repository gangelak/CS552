/* 
 * Copyright Rich West, 2011
 */

#include "helper.h"
#include "threads.h"
#include "types.h"
#include "vga.h"
#include "schedule.h"
static bool done[MAX_THREADS];
static bool in_use[MAX_THREADS] = {0,0};
static uint32_t stacks[MAX_THREADS][1024];

// our runqueue works on this

/* 
 * Create an array of stacks to be used by the 
 * different hreads
*/
extern pcb fifos_threads[MAX_THREADS];

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


static int thread1 () 
{
	int i; 
  	int j = 0;
	
	print_s("Executing Thread1!\n");
  	while (1) 
  	{
		for (i = 0; i < 10; i++) 
    		{
	      	print_s ("<1>"); 
    		}
		print_s("\n");
		//yield();
		if (++j> 6)
		{
			break;
		}
	}

	//  done[0] = TRUE;
	print_s ("Done 1\n");
	return 1;
}

static
int thread2 () 
{
	int i;
	int j = 0;

	print_s("Executing Thread2!\n");
	while (1) 
	{
		for (i = 0; i < 5; i++) 
		{
			print_s ("<2>");
		}
		print_s("\n");

		if (++j == 10)
			break;
  	}
	// done[1] = TRUE;
	print_s ("Done 2\n");

	return 2;
}


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
	pcb * tmp = get_current_thread();
	tmp->status = 1; // means killed
	schedule();

}

/* For debugging purposes only */

void print_context(uint32_t *stack, int tid){
	uint32_t bp = (uint32_t)fifos_threads[tid].bp;

	uint32_t *cur;
	int i = 0;
	for (cur = (uint32_t*)stack; cur <= bp; cur++){
		char buf[10];
		itoa(buf,10,*cur);
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
	if (runqueue->next == 0 )
	{
		//print_s("there is no running thread here!\n");
		return;
	}


	// tmp points to the thread that its next thread has tid equal to passed argument
	pcb* tmp = runqueue->next;
	pcb* btmp = runqueue;
	while (tmp->tid != tid)
	{
		if ( tmp->next == 0 )
		{
			//print_s("the tid not found!!\n");
			return;
		}
		tmp = tmp->next;
		btmp = btmp->next;
	}
	// btmp points to the thread before tmp
	// so we want the thread before temp to point to the thread after tmp
	// technically remove tmp from the linked list
	btmp->next = tmp->next;
	//print_s("runqueue_remove: the tid has been removed\n");
	
}

// add a thread to the runqueue
void runqueue_add(pcb* t)
{
	// runqueue is pointing to the head 
	// first time it is NULL
	if(runqueue->next == 0)
	{
		//print_s("add to runqueue: first time\n");	
		runqueue->next = t;
	}
	else{
		//print_s("add to runqueue: not first time\n");
		// need to iterate through pcbs to find the last one
		pcb* tmp = runqueue->next;
		while(tmp->next != 0)
		{}
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
	stack = (void*) (((uint32_t*) stack) - 1);

	
	/* Found new PCB */
	fifos_threads[new_pcb].tid = new_pcb;
	fifos_threads[new_pcb].bp = (uint32_t) stack;
	fifos_threads[new_pcb].entry = (int*) func;
	fifos_threads[new_pcb].status = 0;
	fifos_threads[new_pcb].next = 0;
	fifos_threads[new_pcb].prev = 0;
	
	/* Create a fake initial context for the process  */

	stack = (void*) (((uint32_t*)stack) - sizeof(fifos_threads[new_pcb].ctx));
	fifos_threads[new_pcb].ctx = (struct context*) stack;

	struct context temp;
	fifos_threads[new_pcb].ctx->eip = (uint32_t) fifos_threads[new_pcb].entry;
	fifos_threads[new_pcb].ctx->ebp = (uint32_t) fifos_threads[new_pcb].bp - 1;
	fifos_threads[new_pcb].ctx->esi = 0;
	fifos_threads[new_pcb].ctx->edi = 0;
	fifos_threads[new_pcb].ctx->flg = 0;
	fifos_threads[new_pcb].ctx->gs = gs;
	fifos_threads[new_pcb].ctx->fs = fs;
	fifos_threads[new_pcb].ctx->es = es;
	fifos_threads[new_pcb].ctx->ds = ds;

	/* Fake an initial context for the new thread */
	fifos_threads[new_pcb].sp = (uint32_t) (((uint32_t *) stack) - 1);
	
//	print_s("Printing the context\n");
	print_context((uint32_t*)fifos_threads[new_pcb].sp,new_pcb);
	// add to the run queue
	runqueue_add(&fifos_threads[new_pcb]);
	return 0;
}


void init_threads(void){
	
	runqueue->next = 0; // set up runqueue
	current->next = 0; // set up current running to null
	int i;
//	print_s("creating the threads\n");
	int* threads[MAX_THREADS] = {(int*)thread1, (int*)thread2};	
	for (i = 0; i < MAX_THREADS; i++){

		thread_create(&stacks[i], threads[i]);
	}
}



