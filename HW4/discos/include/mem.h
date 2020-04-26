#ifndef MEM_H
#define MEM_H

#include "types.h"


#define MAX_PART_SZ_MB 2
#define BLK_SZ 256
#define INDEX_NODE_ARRAY 256
#define INODE_SIZE 64
#define BITMAP_SIZE 1024
#define MAX_FILES 1023
#define MAX_FILE_SIZE 1067008 
#define DIRECT 8		/* Direct pointers in location attribute */
#define PTR_SZ 4		/* 32-bit [relative] addressing */
#define PTRS_PB  (BLK_SZ / PTR_SZ) /* Pointers per index block */


/*TODO*/
/* Create a partition superstruct that contains:
 * Define a superblock struct
 * Define an index node struct (type:4,size:4, location:40, access rights:4?)
 * Define the Bitmap
 *
 */

typedef struct block_t{
	uint8_t data_byte[256];
}block;


typedef struct directory{
	char filename[14]; 		//Directory name
	uint16_t inode_num; 		//Index into the inode array
}dir;


typedef struct superblock{
	int block_num;
	int free_indx;
	/*TODO Maybe add additional fields here!!!*/
	uint8_t pad[248];
}sblock;


typedef struct index_node{
	uint32_t type;   		//File type (dir or reg)
	uint32_t size; 			//File size in bytes
	uint32_t location[10]; 		//Block pointers (1st 8 are direct data block pts - 9nth single indirect - 10nth double indirect)
	uint32_t perm; 			//Permissions for the files (RO,WR,RW)
	uint8_t pad[12];
}indx_node;



typedef struct partition{
	sblock supblock; 		//256 bytes
	indx_node inode[1024]; 		//256 blocks * 256 bytes/block / 64 bytes/inode = 1024 inodes
	uint8_t bitmap[1024]; 		//4 blocks * 256 bytes/block = 1024 bytes
	block d_blks[7931]; 		//(2MB - (256 + 256*256 + 1024)) / 256
}pt;

pt *fs; 				//Our partition

dir *root; 				//Root directory

#endif
