/* 
 * Simple stackless thread example
 * Copyright Rich West, 2011
 */

#include "helper.h"

#define crBegin static int state=0; switch(state) { case 0:

/*
 * ANSI C provides the __LINE__ macro to return the current source line number
 * We can use the line number as a simple form of programmatic state!
 */
#define crReturn(x) do { state=__LINE__; return x; \
                         case __LINE__:; } while (0)
#define crFinish }

/* Create an array of PCBs with size as the MAX_THREADS*/
pcb *fifos_threads[MAX_THREADS];

static bool done[MAX_THREADS];
static bool in_use[MAX_THREADS] = {0,0};


int (*f[MAX_THREADS])(void);


int get_pcb(){
	
	int i;
	for (i =0; i< MAX_THREADS; i++){
		if (in_use[i] == 0){
			in_use[i] = 1;
			pcb temp;
			fifos_threads[i] = &temp;
			return i;
		}
	}
	return -1;
}


int thread1 (void) {

  int i; 
  static int j;

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


int thread2 (void) {

  int i;
  static int j;

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
		fifos_threads[i]->task();
	}

	return;
}

/*TODO add a stack*/

int thread_create(void *func){
	int new_pcb = -1;

	new_pcb = get_pcb();
	if (new_pcb == -1){
		print_s("No PCB available!\n");
		return -1;
	}
	
	fifos_threads[new_pcb]->tid = new_pcb;
	fifos_threads[new_pcb]->task = (int) func;
	fifos_threads[new_pcb]->flag = 0;
	fifos_threads[new_pcb]->next = 0;
	fifos_threads[new_pcb]->prev = 0;
	
	return 0;
}




void init_threads(void){
	thread_create(thread1);
	thread_create(thread2);
}



