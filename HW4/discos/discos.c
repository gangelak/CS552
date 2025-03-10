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


 int threads = 0;

pcb fifos_threads[MAX_THREADS];
pcb dum_dum; 				//dummy thread doing nothing;

#ifdef PCR
int time= 0;
#endif


#define TEST1
#define TEST2
#define TEST3
#define TEST4
#define TEST5
#define TEST6



#define CREAT   rd_creat
#define OPEN    rd_open
#define WRITE   rd_write
#define READ    rd_read
#define UNLINK  rd_unlink
#define MKDIR   rd_mkdir
#define CLOSE   rd_close
#define LSEEK   rd_lseek
#define CHMOD   rd_chmod

#define TEST_SINGLE_INDIRECT
#define TEST_DOUBLE_INDIRECT

#define BLK_SZ 256/* Block size 		*/
#define DIRECT 8/* Direct pointers in location attribute */
#define PTR_SZ 4/* 32-bit [relative] addressing */
#define PTRS_PB  (BLK_SZ / PTR_SZ) /* Pointers per index block */

static char pathname[80];

static char data1[BLK_SZ*DIRECT]; /* Largest data directly accessible */
static char data2[PTRS_PB*BLK_SZ];     /* Single indirect data size */
static char data3[PTRS_PB*PTRS_PB*BLK_SZ]; /* Double indirect data size */
static char addr[PTRS_PB*PTRS_PB*BLK_SZ+1]; /* Scratchpad memory */

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
	init_mem();
	glob_fdt_ptr = main_fdt;
	init_fdt();
	
	char buf[16];
	
	
	/*[> Some arbitrary data for our files <]*/
	memset (data1, '1', sizeof (data1));
	memset (data2, '2', sizeof (data2));
	memset (data3, '3', sizeof (data3));
	int retval, i;
	int fd;
	

#ifdef TEST1
	  /* ****TEST 1: MAXIMUM file creation**** */
	  /* Generate MAXIMUM regular files */
	 pathname[0] = '/';
	  for (i = 0; i < MAX_FILES; i++)
	  { 
		  itoa((pathname+1), 'd', i);
		  retval = CREAT (pathname, RO);
		  if (retval < 0) 
		  {
			  print_s("creat: File creation error!\n");
			  if (i != MAX_FILES)
				  asm volatile("hlt");
	      }
	      memset (pathname + 1, '\0', 79);
	  }
	  /* Delete all the files created */
	  for (i = 0; i < MAX_FILES; i++) 
	  { 
		 itoa((pathname+1), 'd', i);
		 retval = UNLINK (pathname);	
		  if (retval < 0) 
		  {
			  print_s("unlink: File delection error!\n");
			  asm volatile("hlt");
		  }
		  memset (pathname + 1, '\0', 79);
	  }


	  print_s("Passed TEST1\n");
#endif // TEST1


#ifdef TEST2
	    /* ****TEST 2: LARGEST file size**** */
	    /* Generate one LARGEST file */
	    retval = CREAT ("/bigfile", RW);
	    if (retval < 0) 
	    {
		    print_s("creat: File creation error!\n");
		    asm volatile("hlt");
	    }
	    retval =  OPEN ("/bigfile", RW); /* Open file to write to it */
	    if (retval < 0) {
		    print_s("open: File open error!\n");
		    asm volatile("hlt");
	    }
	    fd = retval;/* Assign valid fd */
	    /* Try writing to all direct data blocks */
	    retval = WRITE (fd, data1, sizeof(data1));
	    if (retval < 0) 
	    {
		    print_s("write: File write STAGE1 error\n");
		    asm volatile("hlt");
	    }
#ifdef TEST_SINGLE_INDIRECT
	/* Try writing to all single-indirect data blocks */
	    retval = WRITE (fd, data2, sizeof(data2));
	    if (retval < 0) 
	    {
		    print_s("write: File write STAGE2 error\n");
		    asm volatile("hlt");
	    }
#ifdef TEST_DOUBLE_INDIRECT
	    /* Try writing to all double-indirect data blocks */
	    retval = WRITE (fd, data3, sizeof(data3));
	    if (retval < 0) 
	    {
		    print_s("write: File write STAGE3 error\n");
		    asm volatile("hlt");
	    }
#endif // TEST_DOUBLE_INDIRECT
#endif // TEST_SINGLE_INDIRECT
#endif // TEST2

	  print_s("Passed TEST2\n");

#ifdef TEST3
	int counter = 0;
	memset(addr, '\0', sizeof(addr));
	/* ****TEST 3: Seek and Read file test**** */
	retval = LSEEK (fd, 0);/* Go back to the beginning of your file */
	if (retval < 0) {
		print_s("lseek: File seek error\n");
		asm volatile("hlt");
	}

	/* Try retvalading from all direct data blocks */
	retval = READ (fd, addr, sizeof(data1));
	if (retval < 0) {
		print_s("read: File read STAGE1 error\n");
		asm volatile("hlt");
	}
	
	for (int i =0; i< sizeof(data1); i++){
		if (addr[i] == '1')
			counter++;
		else 
			break;
	}
	
	itoa(buf,'d',counter);
	print_s("STAGE1 1s read ");
	print_s(buf);
	print_s("\n");
	
	itoa(buf,'d',sizeof(data1));
	print_s("STAGE1 data1 size ");
	print_s(buf);
	print_s("\n");

	/* Should be all 1s here... */
#ifdef TEST_SINGLE_INDIRECT
	/* Try reading from all single-indirect data blocks */
	retval = READ (fd, addr, sizeof(data2));
	if (retval < 0) 
	{
		print_s("read: File read STAGE2 error\n");
		asm volatile("hlt");
	}
	
	counter =0 ;
	for (int i =0; i< sizeof(data2); i++){
		if (addr[i] == '2')
			counter++;
		else 
			break;
	}
	
	itoa(buf,'d',counter);
	print_s("STAGE2 2s read ");
	print_s(buf);
	print_s("\n");
	
	itoa(buf,'d',sizeof(data2));
	print_s("STAGE2 data2 size ");
	print_s(buf);
	print_s("\n");
	
	/* Should be all 2s here... */
#ifdef TEST_DOUBLE_INDIRECT
	/* Try reading from all double-indirect data blocks */
	retval = READ (fd, addr, sizeof(data3));
	if (retval < 0) 
	{
		print_s("read: File read STAGE3 error\n");
		asm volatile("hlt"); 
	}
	/* Should be all 3s here... */
#endif // TEST_DOUBLE_INDIRECT
#endif // TEST_SINGLE_INDIRECT
	/* Close the bigfile */
	retval = CLOSE(fd);
	if (retval < 0) {
		print_s("close: File close error\n");
		asm volatile("hlt");
	}
	
	counter =0 ;
	for (int i =0; i< sizeof(data3); i++){
		if (addr[i] == '3')
			counter++;
		else 
			break;
	}
	
	itoa(buf,'d',counter);
	print_s("STAGE3 3s read ");
	print_s(buf);
	print_s("\n");
	
	itoa(buf,'d',sizeof(data3));
	print_s("STAGE3 data3 size ");
	print_s(buf);
	print_s("\n");


#endif // TEST3
	print_s("Passed TEST3\n");

#ifdef TEST4
	    retval =  OPEN ("/bigfile", RW); /* Open file to write to it */
	    if (retval < 0) {
		    print_s("open: File open error!\n");
		    asm volatile("hlt");
	    }
	    fd = retval;/* Assign valid fd */
	  
	    /* ****TEST 4: Check permissions**** */
	  retval = CHMOD("/bigfile", RO); // Change bigfile to read-only
	  if (retval < 0) {
		  print_s("chmod: Failed to change mode\n");
	  }
	 /* Now try to write to bigfile again, but with read-only permissions! */
	retval = WRITE (fd, data1, sizeof(data1));
	if (retval < 0) {
		print_s("chmod: Tried to write to read-only file\n");	    
	}
	
	retval = CLOSE(fd);
	if (retval < 0) {
		print_s("close: File close error\n");
		asm volatile("hlt");
	}
	
	retval = UNLINK ("/bigfile");
	if (retval < 0) 
	{
		print_s("unlink: /bigfile file deletion error\n");
	}
#endif // TEST4
	print_s("Passed TEST4\n");

#ifdef TEST5
	/* ****TEST 5: Make directory including entries **** */
	retval = MKDIR ("/dir1");
	if (retval < 0) 
	{
		print_s("mkdir: Directory 1 creation error\n");
	}
	retval = MKDIR ("/dir1/dir2");
	if (retval < 0) 
	{
		print_s("mkdir: Directory 2 creation error\n");
	}
	retval = MKDIR ("/dir1/dir3");
	if (retval < 0) 
	{
		print_s("mkdir:Directory 3 creation error\n");
	}
#endif //TEST5
	print_s("Passed TEST5\n");

#ifdef TEST6

	/* ****TEST 6: 2 process test**** */

	init_pic();
	init_pit();
	threads = 1;
	init_threads();
	schedule();

#endif // TEST6
	print_s("Passed TEST6\n");
	
	asm volatile("hlt");
	return ;
}
