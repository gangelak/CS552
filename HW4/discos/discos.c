#include "threads.h"
#include "helper.h"
#include "multiboot.h"
#include "vga.h"
#include "schedule.h"
#include "types.h"
#include "pic.h"
#include "include/mem.h"
#include "include/file_ops.h"
/*
	Store the usable memory regions in an array for
	future use. Used the same methodology as memos-2
*/


#define PIT_FREQ 1193181 /* Frequency in hertz */

unsigned long base_addr[10];
unsigned long end_addr[10];
int mem_pointer[10];
int cnt = 0;   // Memory Region Counter

file_obj main_fdt[MAX_FILES + 1]; 		//FDT for the main thread - Max FDs = 1024 same as max files 


pcb fifos_threads[MAX_THREADS];
pcb dum_dum; 				//dummy thread doing nothing;

#ifdef PCR
int time= 0;
#endif



/*
 * 	We assume that the total memory is below 4GB
 */ 
unsigned long get_mem_regs(multiboot_info_t* mbt, mmap_entry_t* ent){
	
	while( ent < mbt->mmap_addr + mbt->mmap_length){
		// Case we hit the 4GB boundary
		if (ent->base_low + ent->len_low == 0)
			break;

		/*if (ent->type == 1)// Usable memory*/
		/*{*/
			base_addr[cnt] = ent->base_low;
			end_addr[cnt] = ent->base_low + ent->len_low;
			mem_pointer[cnt] = ent->base_low;
			cnt++;
		/*}*/

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



void init_pit(void)
{
  outb(0x34, 0x43); //00 11 010 0 to command port 0x43

  outb((PIT_FREQ / 100) & 0xFF, 0x40); //counter 0 low byte written to channel 0 data port 0x40
  outb((PIT_FREQ / 100) >> 8, 0x40); //counter 0 high byte
}


void kmain (multiboot_info_t* mbt, unsigned long magic) {
	/*int avail_regs, indx;          // # of available memory regions*/
	/*unsigned long avail_mem;*/

	/*if (mbt->flags & 0b1000000){*/
		/*mmap_entry_t* ent = (mmap_entry_t*) mbt->mmap_addr;*/
		/*avail_regs = get_mem_regs(mbt,ent);*/
	/*}*/
	 
	/*for (int i =0; i< avail_regs; i++){*/
		/*//Just a checkup of available memory regions*/
		/*[>itoa(buf,'x',base_addr[i]);<]*/
		/*[>print_s("Mem base:");<]*/
		/*[>print_s(buf);<]*/
		/*[>print_s(" Mem end:");<]*/
		/*[>itoa(buf,'x',end_addr[i]);<]*/
		/*[>print_s(buf);<]*/
		/*[>print_s("\n");<]*/
		/*avail_mem = end_addr[i] - base_addr[i];*/

		/*if (((avail_mem >> 20) & 0xfff) >= 2){*/
			/*print_s("Found avail mem at ");*/
			/*indx = i;*/
			/*itoa(buf,'x',base_addr[i]);*/
			/*print_s(buf);*/
			/*print_s("\n");*/
			/*break;*/
		/*}*/
	/*}*/
	
	terminal_initialize();

	char buf[10];


	init_mem();

	glob_fdt_ptr = main_fdt;

	init_fdt();

	show_inode_info(0);

	rd_creat("/test",RW);
	
	show_inode_info(0);
	show_inode_info(1);
	show_inode_info(2);
	rd_creat("/test",RW);
	
	show_inode_info(0);
	show_inode_info(1);
	show_inode_info(2);
	asm volatile("hlt");

	/* Initialize 8259 PIC */
	 /*init_pic();*/

	/* Initialize 8254 PIT */
	 /*init_pit();*/

	/* Initialize file system */
	//init_mem();

	/* Creating autostarting threads */
//	print_s("Creating Threads!!!\n");
	/*init_threads();*/
	


	/*schedule();*/
	return ;
}
