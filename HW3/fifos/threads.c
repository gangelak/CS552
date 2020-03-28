/* 
 * Simple stackless thread example
 * Copyright Rich West, 2011
 */
#include "helper.h"
#include "threads.h"
#include "types.h"
extern pcb fifos_threads[MAX_THREADS];


int (*f[MAX_THREADS])(void);


int get_pcb(){
	
	int i;
	for (i =0; i< MAX_THREADS; i++){
		if (fifos_threads[i].idle == 1){
//			fifos_threads[i].in_use = 1;
			pcb temp;
			temp.idle = 0;
			fifos_threads[i] = temp;
			return i;
		}
	}
	return -1;
}

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

/*TODO add a stack*/

int thread_create(void *func, void *stack){
	int new_pcb = -1;

	new_pcb = get_pcb();
	if (new_pcb == -1){
		print_s("No PCB available!\n");
		return -1;
	}
	
	fifos_threads[new_pcb].tid = new_pcb;
	fifos_threads[new_pcb].task = (int) func;
	fifos_threads[new_pcb].flag = 0;
	fifos_threads[new_pcb].next = 0;
	fifos_threads[new_pcb].prev = 0;
	fifos_threads[new_pcb].sp = (unsigned int )&stack;
	fifos_threads[new_pcb].idle = 0;


	return 0;
}




void init_threads(void){
	// make all the threads idle
	for (int i = 0 ; i < MAX_THREADS ; i++)
	{
		fifos_threads[i].idle = 1;
	}
	print_s("creating the threads\n");
   	thread_create(thread1, 0);
	thread_create(thread2, 0);

}



