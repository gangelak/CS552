#include "threads.h"
#include "helper.h"
#include "multiboot.h"
#include "vga.h"
#include "schedule.h"
#include "types.h"
#include "pic.h"
/*
	Store the usable memory regions in an array for
	future use. Used the same methodology as memos-2
*/


#define PIT_FREQ 1193181 /* Frequency in hertz */

unsigned long base_addr[10];
unsigned long end_addr[10];
int mem_pointer[10];
int cnt = 0;   // Memory Region Counter

pcb fifos_threads[MAX_THREADS];

/*
 * 	We assume that the total memory is below 4GB
 */ 
unsigned long get_mem_regs(multiboot_info_t* mbt, mmap_entry_t* ent){
	
	while( ent < mbt->mmap_addr + mbt->mmap_length){
		// Case we hit the 4GB boundary
		if (ent->base_low + ent->len_low == 0)
			break;

		if (ent->type == 1)// Usable memory
		{
			base_addr[cnt] = ent->base_low;
			end_addr[cnt] = ent->base_low + ent->len_low;
			mem_pointer[cnt] = ent->base_low;
			cnt++;
		}

		ent = (mmap_entry_t*) ((unsigned int) ent + ent->size + sizeof(ent->size));
	}

	
	cnt--;
	return cnt; 
}



/*
	Custom malloc implementation
*/

void *my_malloc(int size){
	int j;
	unsigned long temp;

	for (j = cnt; j >= 0; j--){
		if ( (mem_pointer[j] + size) <= end_addr[j]){
			temp = mem_pointer[j];
			mem_pointer[j] += size;
			return (void*)temp;
		}
	}

	return 0;
}

/*void init_pit(void){*/
	/*pit_init();*/

/*}*/


void init_pit(void)
{
  outb(0x34, 0x43); //00 11 010 0 to command port 0x43

  outb((PIT_FREQ / 1000000) & 0xFF, 0x40); //counter 0 low byte written to channel 0 data port 0x40
  outb((PIT_FREQ / 1000000) >> 8, 0x40); //counter 0 high byte
}


void kmain (multiboot_info_t* mbt, unsigned long magic) {
	//int avail_regs;          // # of available memory regions
	
	int i=0;

	/*if (mbt->flags & 0b1000000){*/
		/*mmap_entry_t* ent = (mmap_entry_t*) mbt->mmap_addr;*/
		/*avail_regs = get_mem_regs(mbt,ent);*/
	/*}*/
	
	/* Initialize the PCB pool*/
//	pcb_init();
	

	/* Initialize 8259 PIC */
	 init_pic();

	/* Initialize 8254 PIT */
	 init_pit();

	/* Creating autostarting threads */

//	thread_create();
	terminal_initialize();

//	print_s("Getting memory regions!!!!\n");

//	print_s("Creating Threads!!!\n");
	init_threads();
	schedule();
	return ;


}
