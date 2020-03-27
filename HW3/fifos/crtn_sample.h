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

#define MAX_THREADS 1

typedef enum {
  FALSE,
  TRUE
} bool;

typedef struct runqueue {
  int (*task)();
  int tid; // thread ID
  struct runqueue *next;
  struct runqueue *prev;
} rq;

static rq head;
static bool done[MAX_THREADS];

int (*f[MAX_THREADS])(void);

int thread1 (void) {

  int i; 
  static int j;

  crBegin;
  while (1) {
    for (i = 0; i < 10; i++) {
      print ("1");
      msleep (1000);
  //    fflush (stdout);
    }
    print ("\n");
    crReturn (1); // Let's cooperate and yield

    if (++j == 6)
      break;
  }
  done[0] = TRUE;

  print ("Done 1\n");

  crFinish;

  return 1;
}


int thread2 (void) {

  int i;
  static int j;

  crBegin;
  while (1) {
    for (i = 0; i < 5; i++) {
      print ("2");
      msleep (1000);
      //fflush (stdout);
    }
    print("\n");
    crReturn (2); // Time to yield

    if (++j == 10)
      break;
  }
  done[1] = TRUE;

  print ("Done 2\n");

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


