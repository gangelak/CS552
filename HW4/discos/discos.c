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


//#define TEST1
//#define TEST2
//#define TEST3
//#define TEST4
//#define TEST5
//#define TEST6



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

static char data1[BLK_SZ*DIRECT + 257]; /* Largest data directly accessible */
static char data2[8*256 + 64*256 + 256*64*64];     /* Single indirect data size */
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
	
	itoa(buf,'d',&fs->d_blks[0]);
	print_s("Address of block 0 ");
	print_s(buf);
	print_s("\n");
	
	/* Some arbitrary data for our files */
	memset (data1, '1', sizeof (data1));
	memset (data2, '2', sizeof (data2));
	memset (data3, '3', sizeof (data3));
	int retval, i;
	int fd;
	int index_node_number;


	
        rd_mkdir("/test");
	/*show_inode_info(0);*/
	/*show_inode_info(1);*/
        rd_mkdir("/test/tmp");
	/*show_inode_info(1);*/
	/*show_inode_info(2);*/
        rd_creat("/test/tmp/giannis", RW);
	/*show_inode_info(2);*/
	/*show_inode_info(3);*/

	char tmp[]="123456789";
	memset (data1, '1', sizeof (data1));
        fd = rd_open("/test/tmp/giannis", RW);
	
	itoa(buf,'d',glob_fdt_ptr[0].inode);
	
	print_s("Write time\n");
        rd_write(fd, data3, sizeof(data3));
        
        memset(addr, '\0', sizeof(addr));
	
	rd_lseek(fd, 0);
	print_s("Read time\n");
	int size = rd_read(fd, addr, sizeof(data3));
        
	print_s("THE CONTENT IS: ");
	int count =0;
	for (int i =0; i< size; i++){
		if (addr[i] == '3'){
			//terminal_putchar(addr[i]);
			count++;
		}
		else 
			break;
	}
	itoa(buf,'d',count);
	print_s("Count is ");
	print_s(buf);
	print_s("\n");
	/*print_s(addr);*/
	/*print_s("\n");*/
	char strsize[10];
	itoa(strsize,'d',size);
	print_s("\n");
	print_s(strsize);

	



#ifdef TEST1
	  /* ****TEST 1: MAXIMUM file creation**** */
	  /* Generate MAXIMUM regular files */
	  for (i = 0; i < MAX_FILES; i++)
	  { 
		  sprintf(pathname, "/file", i);
		  retval = CREAT (pathname, RO);
		  if (retval < 0) 
		  {
			  print_s("creat: File creation error!\n");
			  if (i != MAX_FILES)
				  asm volatile("hlt");
	      }
	      memset (pathname, 0, 80);
	  }
	  /* Delete all the files created */
	  for (i = 0; i < MAX_FILES; i++) 
	  { 
		  sprintf (pathname, "/file", i);
		 retval = UNLINK (pathname);	
		  if (retval < 0) 
		  {
			  print_s("unlink: File delection error!\n");
			  asm volatile("hlt");
		  }
		  memset (pathname, 0, 80);
	  }
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
#ifdef TEST_DOUBLE_INDIRECTT
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






#ifdef TEST3
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
	/* Should be all 1s here... */
#ifdef TEST_SINGLE_INDIRECT
	/* Try reading from all single-indirect data blocks */
	retval = READ (fd, addr, sizeof(data2));
	if (retval < 0) 
	{
		print_s("read: File read STAGE2 error\n");
		asm volatile("hlt");
	}
	
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
#endif // TEST3

#ifdef TEST4
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
	retval = UNLINK ("/bigfile");
	if (retval < 0) 
	{
		print_s("unlink: /bigfile file deletion error\n");
	}
#endif // TEST4

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

#ifdef TEST6
	/* ****TEST 6: 2 process test**** */

	init_pic();
	init_pit();
	init_threads();
	schedule();

#endif // TEST6
	print_s("Congrats\n");
	asm volatile("hlt");











	asm volatile("hlt");

	


	/*schedule();*/
	return ;
}
