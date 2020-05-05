/* 
 * Copyright Rich West, 2011
 */

#include "helper.h"
#include "threads.h"
#include "types.h"
#include "vga.h"
#include "schedule.h"
#include "pic.h"
#include "include/mem.h"
#include "include/file_ops.h"
#define CREAT   rd_creat

static bool pcb_in_use[MAX_THREADS];
static uint32_t stacks[MAX_THREADS][1024];
 uint32_t dstack[1024]; 			//dummy stack for the dummy thread

#ifdef PCR
extern int time;
rpl repl_pool[MAX_REPLS];
#endif
extern pcb fifos_threads[MAX_THREADS];
extern pcb dum_dum;

/* Function to add some delay */

void sleep (){
	int j;
#ifndef PCR
	for ( j=0; j < 100000000; j++ )
#else
	for ( j=0; j < 10000000; j++ )
#endif
		nop();
}


/* 
 * Task preemption function 
 * Called when an IRQ0 occurs 
 */
void preempt_thread(){
	/* Acknowledge that the interrupt is serviced */
	PIC_sendEOI();
#ifdef PCR
	time += 1;        // Increment the time counter by one
#endif
	schedule();
}

/* Get an available pcb spot from the array */
int get_pcb(){
	
	int i;
	for (i =0; i< MAX_THREADS; i++){
		if (pcb_in_use[i] == 0){
			pcb temp;
			pcb_in_use[i] = 1;
			fifos_threads[i] = temp;
			return i;
		}
	}
	return -1;
}


/* Thread functions */

void thread_yield() {
	
	/* Just set status and call scheduler */
	//fifos_threads[get_current_thread()->tid].status = 0;
	char buf[50];
	itoa(buf,'d',current->tid);
	print_s("(Y");
	print_s(buf);
	print_s(")");
	schedule();
	return;
}
void exit_thread() {
	
	pcb * tmp = get_current_thread();
	
	pcb_in_use[tmp->tid] = 0; 			// This PCB is not use anymore

	tmp->status = 1; // means killed

/*#ifdef PCR*/
	/*while(1);*/
/*#else*/
	/* 
	 * Current running thread is done 
	 * Need to change status -> Status.killed
	 */
	//print_s("Exit!\n");
	schedule();
/*#endif*/
}

/* Dummy Thread's function */
void do_nothing(){
	while(1){
		print_s("Doing nothing!!");
		sleep();
	}
}

void thread_func() 
{
	int i;
	int j = 0;
	int cnt = current->tid + 100;
	char name[10];
	char counter[50];
	itoa(name,'d',current->tid);

	/*print_s("Executing Thread <");*/
	/*print_s(name);*/
	/*print_s(">\n");*/
	char pathname[80];
	for (i = 0; i < 300; i++)
	{
		pathname[0] = '/';
		itoa((pathname+1),'d', (1+current->tid)*300 + i ) ;
		int retval = CREAT (pathname, RO);
		if (retval < 0)
		{
			print_s("Thread");
			print_s(name);
			print_s(" creation: Failed creation error\n");
		//	asm volatile("hlt");
		}
		memset (pathname, '\0', 79);                 
	}
	show_inode_info(0);
	print_s ("Done <");
	print_s(name);
	print_s(">!");

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
			prev_node = current;
			if (tmp->next != 0)
				current = tmp->next;
			else
				current = runqueue->next;
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
//		print_s("add to runqueue: first time\n");	
		runqueue->next = t;
	}
	else{
//		print_s("add to runqueue: not first time\n");
		// need to iterate through pcbs to find the last one
		pcb* tmp = runqueue->next;
		while(tmp->next != 0)
		{
//			print_s("iterating...\n");
			tmp = tmp->next;
		}
		// the last one is gonna point to t
		tmp->next = t;
	}
}

int thread_create(void *stack, void *func)
{
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
	fifos_threads[new_pcb].ctx->esi = 0;
	fifos_threads[new_pcb].ctx->edi = 0;
	fifos_threads[new_pcb].ctx->gs = gs;
	fifos_threads[new_pcb].ctx->fs = fs;
	fifos_threads[new_pcb].ctx->es = es;
	fifos_threads[new_pcb].ctx->ds = ds;

	/* Fake an initial context for the new thread */
	fifos_threads[new_pcb].sp = (uint32_t) (((uint32_t *) stack));
#ifdef PCR

	fifos_threads[new_pcb].ci = new_pcb + 1;
	fifos_threads[new_pcb].ti = 10;
	fifos_threads[new_pcb].ai = 0;
	fifos_threads[new_pcb].rpl_list = 0;
#endif

#ifdef MEM
	fifos_threads[new_pcb].file_desc = (file_obj*)file_desc_pool; 
#endif
	// add to the run queue
	runqueue_add(&fifos_threads[new_pcb]);
	return 0;
}


void init_threads(void){
	runqueue->next = 0; // set up runqueue
	current = 0; // set up current running to null
	int i;
	for (i = 0; i < MAX_THREADS; i++){
		thread_create(&(stacks[i][1023]), thread_func);
	}

	/* Initialize the dummy thread */
	void *stk = &dstack[1023];

	*(((uint32_t*) stk) - 0) = 0;
	
	dum_dum.tid = -1;
	dum_dum.bp = (uint32_t) ((uint32_t*)stk -1);
	dum_dum.entry = (uint32_t) do_nothing;
	dum_dum.status = 0;
	dum_dum.next = 0;
	dum_dum.prev = 0;
	
	stk = (void*) (stk - sizeof(struct context));
	dum_dum.ctx = (struct context*) stk;

	dum_dum.ctx->eip = dum_dum.entry;
	dum_dum.ctx->ebp = (uint32_t) dum_dum.bp;
	dum_dum.ctx->ebx = 0;
	dum_dum.ctx->esi = 0;
	dum_dum.ctx->edi = 0;
	dum_dum.ctx->gs = 0x10;
	dum_dum.ctx->fs = 0x10;
	dum_dum.ctx->es = 0x10;
	dum_dum.ctx->ds = 0x10;

	dum_dum.sp = (uint32_t) (((uint32_t *) stk));

}



