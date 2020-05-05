#include "include/mem.h"
#include "helper.h"
#include "include/file_ops.h"
#include "vga.h"
#define my_sizeof(x) ((&x + 1) - &x)
#define my_sizeof(x) ((char *)(&x + 1) - (char *)&x)

extern pt FS;


void init_bitmap(void){
	//first 256 blocks
	uint8_t *bit_ptr;
	for (int i =0 ; i <= 31; i++){
		fs->bitmap[i] = 0xff;
	}
	bit_ptr = &fs->bitmap[32];
	*bit_ptr = 0x0; 
	*bit_ptr |= 0xf8; 		//1111 1000

	for (int i =33 ; i < 1024; i++){
		fs->bitmap[i] = 0x0;
	}
	
}

void init_inodes(void){
	char buf[16];
	for (int i = 0; i <= MAX_FILES ; i++){
		fs->inode[i].in_use = FREE;
		fs->inode[i].type = JUNK; 
		fs->inode[i].size = JUNK; 				//Root is empty
		//memset(fs->inode[0].location[0],JUNK,40); 	//Initialize the location pointers to JUNK = 2000 (Way above 1024)
		for (int j= 0; j< 10; j++){
			fs->inode[i].location[j] = 0;
		}
		fs->inode[i].perm = JUNK; 			//Root is read write
		fs->inode[i].in_use = FREE; 			//Mark the first inode as used
	}
}


void init_mem(){
	
	unsigned long mem_size = (MAX_PART_SZ_MB<<20);
	char buf[16];

	/* Initialize filesystem memory to 0 */
	//memset(&base_addr,0,mem_size);

	/* Set the partition to point to the start of our free memory*/
	fs = &FS;

	/* Start initializing the different fields of the partition*/
	
	/*Superblock Setup */
	fs->superblock.block_num = 7930; 	//Account for root directory
	fs->superblock.free_inodes = 1023; 	//Same here
	
	/*Initialize bitmap*/
	//The first 261 blocks are allocated for superblock, inode array, root dir
	init_bitmap();
	
	/*Initialize the inode array*/
	init_inodes();
	
	/*Setup root*/
	root->filename[0] = '/';
	root->filename[1] = '\0';
	root->inode_num = 0; 			//Uses the first inode


	fs->inode[0].type = DR; 
	fs->inode[0].size = 0; 				//Root is empty
	//memset(fs->inode[0].location[0],JUNK,40); 	//Initialize the location pointers to JUNK = 2000 (Way above 1024)
	for (int i= 0; i< 10; i++){
		fs->inode[0].location[i] = 0;
	}
	
	int res;
	//Allocate block for root
	res = allocate_block(0,0);
	if (res < 0){
		print_s("Could not allocate a block for root...Major problem\n");
		return -1;
	}

	fs->inode[0].location[0] = &fs->d_blks[0]; 	//Initialize the first location pointer
	
	//Initialize the block
	dir_t * entry;

	entry = (dir_t *) fs->inode[0].location[0];
	//Initialize the block
	for (int i =0; i< 16; i++){
	   for (int j =0; j< 14; j++)
		   entry->filename[j] = '\0';
	   entry->inode_num = JUNK;
	   entry++;
	 }
	
	fs->inode[0].perm = RW; 			//Root is read write
	fs->inode[0].in_use = USED; 			//Mark the first inode as used
	fs->inode[0].opened = 0; 			//Mark the first inode as used

}
