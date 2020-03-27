/* 
 * Simple stackless thread example
 * Copyright Rich West, 2011
 */

#include "helper.h"
#include "vga.h"

#define crBegin static int state=0; switch(state) { case 0:

/*
 * ANSI C provides the __LINE__ macro to return the current source line number
 * We can use the line number as a simple form of programmatic state!
 */
#define crReturn(x) do { state=__LINE__; return x; \
                         case __LINE__:; } while (0)
#define crFinish }

/* Create an array of PCBs with size as the MAX_THREADS*/
pcb fifos_threads[MAX_THREADS];

static pcb *head;
static bool done[MAX_THREADS];

int (*f[MAX_THREADS])(void);

int thread1 (void) {

  int i; 
  static int j;

  crBegin;
  while (1) {
    for (i = 0; i < 10; i++) {
      print_s ("1");
      msleep (1000);
  //    fflush (stdout);
    }
    print_s ("\n");
    crReturn (1); // Let's cooperate and yield

    if (++j == 6)
      break;
  }
  done[0] = TRUE;

  print_s ("Done 1\n");

  crFinish;

  return 1;
}


int thread2 (void) {

  int i;
  static int j;

  crBegin;
  while (1) {
    for (i = 0; i < 5; i++) {
      print_s ("2");
      msleep (1000);
      //fflush (stdout);
    }
    print_s("\n");
    crReturn (2); // Time to yield

    if (++j == 10)
      break;
  }
  done[1] = TRUE;

  print_s ("Done 2\n");

  crFinish;

  return 2;
}


void schedule (void) {

  rq *current; // Current thread in runqueue
  rq *finished; // A thread that's finished execution

  int threads = MAX_THREADS;
  
  current = &head;

  while (current) {
    (current->task)();
    
    if (done[current->tid]) { // Remove from runqueue
      
      if (threads == 1) // We've finished last one
	return;

      finished = current;
      finished->prev->next = current->next;
      current = current->next;
      current->prev = finished->prev;

      if (current->next == finished) { // Down to last thread
	current->next = finished->next;
      }

      if (finished != &head)
	//kfree (finished);
      threads--;
    }
    else
      current = current->next;
  }
}

/*TODO add a stack*/

int thread_create(void){
	int i;

	rq *ptr, *pptr;


	f[0] = thread1;
	f[1] = thread2;

	head.task = f[0];
	head.tid = 0;
	head.next = 0;
	head.prev = 0;


	for (i = 1; i < MAX_THREADS; i++){
		ptr = (rq *) my_malloc((int)sizeof(rq));
		
		// my_malloc failed to return a memory chunk
		if (ptr == 0){
			return -1;
		}

		if (i == 1){
			head.next = ptr;
			pptr = &head;
		}
		else{
			pptr = pptr->next;
			pptr->next = ptr;
		}

		ptr->prev = pptr;
		ptr->task = f[i];
		ptr->tid = i;
		ptr->next = &head;
		head.prev = ptr;
	
	}

	schedule ();

	return 0;
}




void init_threads(void){
	int ret;
	ret = thread_create();

	if (ret == -1){
		print_s ("Error with my_malloc\n");
		asm volatile ("hlt\n\t");
	}
}



