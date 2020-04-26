#include "mem.h"
#include "helper.h"
#include "file_ops.h"


int init_mem(unsigned long *base_addr){

	unsigned long mem_size = (MAX_PART_SZ_MB<<20);
	

	/* Initialize filesystem memory to 0 */
	memset(base_addr,0,mem_size);

	/* Set the partition to point to the start of our free memory*/
	fs = (pt *)base_addr;
	
	/* Start initializing the different fields of the partition*/

	/*Setup root*/
	root->filename[0] = '/';
	root->filename[1] = '\0';
	root->inode_num = 0; 			//First inode in the inode array
	root = (dir*) &fs->d_blks[0]; 		//Start from block 0 for root

	fs->inode[0].type = DR; 
	fs->inode[0].size = 16; 		//Only one entry for root -> itself
	fs->inode[0].location[0] = 0; 		//Starting from data block 0
	fs->inode[0].perm = RW; 		//Root is read write
}
