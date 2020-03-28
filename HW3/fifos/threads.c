/* 
 * Copyright Rich West, 2011
 */

#include "helper.h"
#include "threads.h"
#include "types.h"

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
//			fifos_threads[i].in_use = 1;
			pcb temp;
			in_use[i] = 1;
			fifos_threads[i] = temp;
			return i;
		}
	}
	return -1;
}
/* Thread functions */


static int thread1 () {

  int i; 
  static int j;
	
  print_s("Executing Thread1!\n");
  while (1) {
    for (i = 0; i < 5; i++) {
      print_s ("1");
   //   msleep (1000);
  //    fflush (stdout);
    }
    print_s ("\n");

    if (++j == 3)
      break;
  }
//  done[0] = TRUE;

  print_s ("Done 1\n");


  return 1;
}

static
int thread2 () {

  int i;
  static int j;

  print_s("Executing Thread2!\n");
  while (1) {
    for (i = 0; i < 5; i++) {
      print_s ("2");
   //   msleep (1000);
      //fflush (stdout);
    }
    print_s("\n");

    if (++j == 10)
      break;
  }
// done[1] = TRUE;

  print_s ("Done 2\n");

  return 2;
}

/*
void schedule (void) {
	int i;
	for (i =0; i< MAX_THREADS; i++){
		if (in_use[i] & !done[i])
		{
			print_s("scheduling thread\n");
			fifos_threads[i].task();
			in_use[i] = 0;
			done[i] = 1;
		}
	}

	return;
}
*/

void thread_yield() {
	// save state and call scheduler
	// our scheduler is wrong for now
	// we should make a runqueue like west
	// and add the threads to the end of it
	// and in the scheduler do round robin
}
void exit_thread() {
	

}

// remove the current head of runqueue bc it finished
void runqueue_remove()
{
	if (runqueue->next != 0 )
	{
		// this was the last one in the runqueue
		print_s("remove from runqueue: this was the last one to remove\n");
		runqueue = 0;
		return;
	}
	// maybe I didn't need head variable
	print_s("remove from runqueue: not the last one\n");
	pcb* head = runqueue->next;
	
	runqueue->prev->next = head;
	head->prev = runqueue->prev;
	
	//  the new runqueue is the next one in the linked list
	runqueue = runqueue->next;

	return;
}

// add a thread to the runqueue
void runqueue_add(pcb* t)
{
	if(runqueue)
	{
		print_s("add to runqueue: not first time\n");

		// we don't wanna change where runqueue points to
		// we get tail
		// and change next and prev in tail
		// we wanna add t between tail and [what is running now] kinda:
		// [what is running now] is what runqueue point to
		//  tail [what is running now] next to run
		
		pcb * tail = runqueue->prev;
		t->next = tail->next;
		t->prev = tail;
		tail->next = t;

		// now update runqueue to also point to the new tail
		t->next->prev = t;

	}
	else{
		print_s("add to runqueue: first time\n");
		runqueue = t;
		runqueue->next = runqueue;
		runqueue->prev = runqueue;

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


	*(((uint32_t*) stack) - 0) = (uint32_t) new_pcb;
	*(((uint32_t*) stack) - 1) = (uint32_t) exit_thread;
	stack = (void*) (((uint32_t*) stack) - 2);

	
	/* Found new PCB */
	fifos_threads[new_pcb].tid = new_pcb;
	fifos_threads[new_pcb].bp = (uint32_t) stack;
	fifos_threads[new_pcb].entry = (int) func;
	fifos_threads[new_pcb].status = 0;
	fifos_threads[new_pcb].next = 0;
	fifos_threads[new_pcb].prev = 0;

	/* Fake an initial context for the new thread */
	fifos_threads[new_pcb].sp = (uint32_t) (((uint16_t *) stack) - 24);
	
	
	/* Fake initial context */

	/* EIP */ *(((uint32_t *)stack) - 0) = (uint32_t) fifos_threads[new_pcb].entry; 
	/* FLG */ *(((uint32_t *)stack) - 1) = 0 ;   //Set IF for preemption  (Not now)
	/* EAX */ *(((uint32_t *)stack) - 2) = 0; 
	/* ECX */ *(((uint32_t *)stack) - 3) = 0; 
	/* EDX */ *(((uint32_t *)stack) - 4) = 0; 
	/* EBX */ *(((uint32_t *)stack) - 5) = 0; 
	/* ESP */ *(((uint32_t *)stack) - 6) = (uint32_t) (((uint32_t *) stack) - 3);
	/* EBP */ *(((uint32_t *)stack) - 7) = (uint32_t) (((uint32_t *) stack) - 3);
	/* ESI */ *(((uint32_t *)stack) - 8) = 0; 
	/* EDI */ *(((uint32_t *)stack) - 9) = 0; 
	
	/* DS */ *(((uint16_t *)stack) - 21) = 0; 
	/* ES */ *(((uint16_t *)stack) - 22) = 0; 
	/* FS */ *(((uint16_t *)stack) - 23) = 0; 
	/* GS */ *(((uint16_t *)stack) - 24) = 0; 

	// add to the run queue
	runqueue_add(&fifos_threads[new_pcb]);

}


void init_threads(void){
	
	int i;
	print_s("creating the threads\n");
	
	for (i = 0; i < MAX_THREADS; i++){
		thread_create(&stacks[i], thread);
	}
}



