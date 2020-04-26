#include "types.h"


/*File Modes*/
#define RO 0
#define WR 1
#define RW 2

#define DR 1
#define FL 2

int rde_creat(char *pathname, mode_t mode);
int rd_mkdir(char *pathname);
int rd_open(char *pathname, int flags);
int rd_close(int fd);
int rd_read(int fd, char *address, int num_bytes);
int rd_write(int fd, char *address, int num_bytes);
int rd_lseek(int fd, int offset);
int rd_unlink(char *pathname);
int rd_cmmod(char *pathname, mode_t mode);
