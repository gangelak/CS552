#include "mem.h"
#include "helper.h"
#include "file_ops.h"

void init_mem(unsigned long *base_addr){

	unsigned long mem_size = (MAX_PART_SZ_MB<<20);
	

	/* Initialize filesystem memory to 0 */
	memset(base_addr,0,mem_size);

	/* Set the partition to point to the start of our free memory*/
	fs = (pt *)base_addr;
	
	/* Start initializing the different fields of the partition*/
	
	/*Superblock Setup */
	fs->superblock.block_num = 7930; 	//Account for root directory
	fs->superblock.free_inodes = 1023; 	//Same here

	/*Setup root*/
	root->filename[0] = '/';
	root->filename[1] = '\0';
	root->inode_num = 0; 			//Uses the first inode


	fs->inode[0].type = DR; 
	fs->inode[0].size = 0; 				//Root is empty
	memset(fs->inode[0].location[0],JUNK,40); 	//Initialize the location pointers to JUNK = 2000 (Way above 1024)
	fs->inode[0].perm = RW; 			//Root is read write
	fs->inode[0].in_use = USED; 			//Mark the first inode as used

}
