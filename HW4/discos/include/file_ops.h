#include "types.h"


/*File Modes*/
#define RO 0
#define WR 1
#define RW 2

#define DR 1
#define FL 2
#define USED 1
#define FREE 0
#define ERROR -1
#define JUNK 2000
#define CR 0 		//Create a file/dir
#define DL 1 		//Delete a file/dir


int rd_creat(char *pathname, mode_t mode);
int rd_mkdir(char *pathname);
int rd_open(char *pathname, int flags);
int rd_close(int fd);
int rd_read(int fd, char *address, int num_bytes);
int rd_write(int fd, char *address, int num_bytes);
int rd_lseek(int fd, int offset);
int rd_unlink(char *pathname);
int rd_cmmod(char *pathname, mode_t mode);

void show_inode_info(int inode);
void init_fdt(void);
void show_fd_object(int fd);
int allocate_block(int block_num, int inode);

file_obj *glob_fdt_ptr;
