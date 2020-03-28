/* 
 * Copyright Rich West, 2011
 */

#include "helper.h"
#include "threads.h"
#include "types.h"

pcb fifos_threads[MAX_THREADS];

static bool done[MAX_THREADS];
static bool in_use[MAX_THREADS] = {0,0};
static uint32_t stacks[MAX_THREADS];

/* 
 * Create an array of stacks to be used by the 
 * different hreads
*/

void create_stack(void){
	int i;

	for (i = 0; i< MAX_THREADS; i++ ){
		uint32_t stack[1024];
		stacks[i] = stack;
	}
}

/* Get an available pcb spot from the array */

int get_pcb(){
	
	int i;
	for (i =0; i< MAX_THREADS; i++){
		if (in_use[i] == 0){
			in_use[i] = 1;
			pcb temp;
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
    for (i = 0; i < 10; i++) {
      print_s ("1");
   //   msleep (1000);
  //    fflush (stdout);
    }
    print_s ("\n");

    if (++j == 6)
      break;
  }
  done[0] = TRUE;

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
  done[1] = TRUE;

  print_s ("Done 2\n");

  return 2;
}


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

	/* EIP */ *(((uint32_t *)stack) - 0) = fifos_threads[new_pcb].entry; 
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


	return 0;
}




void init_threads(void){
	
	int i;
	print_s("creating the threads\n");
	
	create_stack();
	
	for (i = 0; i < MAX_THREADS; i++){
		thread_create(&stacks[i].stack[1023], thread);
	}
}



