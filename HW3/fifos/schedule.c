#include "types.h"
#include "threads.h"
#include "vga.h"

extern pcb fifos_threads[MAX_THREADS];
 
void schedule () {
	int i;
	for (i =0; i < MAX_THREADS; i++)
	{
		if (fifos_threads[i].in_use & !fifos_threads[i].done)
		{
			print_s("scheduling threads\n");
			fifos_threads[i].task();
			fifos_threads[i].in_use = 0;
			fifos_threads[i].done = 1;
		}
	}
	return;
}
