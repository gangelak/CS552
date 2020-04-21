#ifndef MEM_H
#define MEM_H

#include "types.h"


#define MAX_PARTITION_SIZE_MB 2
#define BLOCK_SIZE 256
#define INDEX_NODE_ARRAY 256
#define INODE_SIZE 64
#define BITMAP_SIZE 1024
#define MAX_FILE_SIZE 1067008 

/*TODO*/
/* Create a partition superstruct that contains:
 * Define a superblock struct
 * Define an index node struct (type:4,size:4, location:40, access rights:4?)
 * Define the Bitmap
 *
 */

typedef struct superblock{
	int block_num;
	int free_indx;
	/*TODO Maybe add additional fields here!!!*/
	uint8_t pad[248];
}sblock;




typedef struct partition{
	sblock supblock;

}



int rd_creat(char *pathname, mode_t mode);
int rd_mkdir(char *pathname);
int rd_open(char *pathname, int flags);
int rd_close(int fd);
int rd_read(int fd, char *address, int num_bytes);
int rd_write(int fd, char *address, int num_bytes);
int rd_lseek(int fd, int offset);
int rd_unlink(char *pathname);
int rd_chmod(char *pathname, mode_t mode);



#endif
