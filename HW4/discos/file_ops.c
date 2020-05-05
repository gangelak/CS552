#include "include/mem.h"
#include "include/file_ops.h"
#include "helper.h"
#include "vga.h"
#include "types.h"


int check_pathname(char *pathname, int* par_inode, char *filename);
int exist_inodes(void);
int update_parent(int parent_inode,char *filename, int action, uint32_t type, uint32_t mode);
int allocate_inode(uint32_t type, uint32_t permissions);
int check_if_exists(char *name,int par_inode);
void show_bitmap(void);
int find_block(int block_num, block_t** cur_block, int inode);
int exist_blocks(void);
void deallocate_block(int indx);
void init_block(block_t* cur_block, int type);


extern int threads;


/*TODO*/
// Need one function to recurse through the pathnames
// Need one function to recurse through the inodes
// Need one strcmp function to compare string names


/*Initialize the current global fdt pointer*/
//Don't run anywhere else except when the table is
//first created

void init_fdt(void){
	for (int i =0; i<= MAX_FILES; i++){
		glob_fdt_ptr[i].in_use = FREE;
	}
}




// Create a file or a directory
// TODO: Check if the path name exists
// 	 Check if there are available inodes
// 	 Check if there is at least one available block
int rd_creat(char *pathname, mode_t mode){


	asm volatile("cli");

	char buf[16];

	int res = 0;
	char filename[14];
	int parent_inode;
	//Check if we are trying to create the root directory
	if (strcmp(pathname,root->filename) == 0){
		print_s("creat: Root directory already exists!!!");
		return ERROR;
	}
	
	//Check if there are available inodes for our file
	
	res = exist_inodes();
	
	if (res < 0){
		print_s("creat: There are no available inodes...Try later, sorry\n");
		return ERROR;
	}
	
	res = check_pathname(pathname,&parent_inode,filename);
	
	if (res >= 0){
		print_s("creat: The file already exists\n");
		return ERROR;
	}
	else if (res < 0 && parent_inode < 0){
		print_s("creat: Invalid path\n");
		return ERROR;
		
	}


	if (parent_inode < 0){
		print_s("creat: Invalid path...Aborting\n");
		return ERROR;
	}

	//Ok now we have the correct parent...Allocate resources for the file
	//Also update the parent entry
	
//	print_s("Before updating the parent with the new entry\n");
	res = update_parent(parent_inode,filename, CR, FL, mode);
	if (res < 0){
		print_s("creat: Cannot create a new entry for this file...Aborting\n");
		return ERROR;
	}


	if ( threads == 1 ){
		asm volatile("sti");
	}

//	print_s("creat: Parent updated\n");

}


int rd_mkdir(char *pathname){
	asm volatile("cli");
	int res = 0;

	char filename[14];
	int parent_inode;
	char buf[16];
	//Check if we are trying to create the root directory
	if (strcmp(pathname,root->filename) == 0){
		print_s("mkdir: Root directory already exists!!!");
		return ERROR;
	}

	//Check if there are available inodes for our file
	
	res = exist_inodes();
	
	if (res < 0){
		print_s("mkdir: There are no available inodes...Try later, sorry\n");
		return ERROR;
	}
	
	res = check_pathname(pathname,&parent_inode,filename);
	
	if (res >= 0){
		print_s("mkdir: The file already exists\n");
		return ERROR;
	}
	else if (res < 0 && parent_inode < 0){
		print_s("mkdir: Invalid path\n");
		return ERROR;
		
	}
	
	/*itoa(buf,'d',parent_inode);*/
	/*print_s("Parent inode is ");*/
	/*print_s(buf);*/
	/*print_s("\n");*/

	if (parent_inode < 0){
		print_s("mkdir: Invalid path...Aborting\n");
		return ERROR;
	}

	//Ok now we have the correct parent...Allocate resources for the file
	//Also update the parent entry
	
	print_s("Before updating the parent with the new entry\n");
	res = update_parent(parent_inode,filename, CR, DR, RW);
	if (res < 0){
		print_s("mkdir: Cannot create a new entry for this directory...Aborting\n");
		return ERROR;
	}

	print_s("mkdir: Parent updated\n");

	if ( threads == 1 )
	{	
		asm volatile("sti");
	}
	
	return 0;
}

int rd_open(char *pathname, int flags){
	asm volatile("cli");	
	int res = 0;

	char filename[14];
	int parent_inode, file_inode;
	char buf[16];

	if (strcmp(pathname,root->filename) == 0){
		print_s("open: Root directory is always open\n");
		return ERROR;
	}
	
	file_inode = check_pathname(pathname,&parent_inode,filename);
	
	if (file_inode < 0 && parent_inode < 0){
		print_s("open: Invalid path\n");
		return ERROR;
		
	}
	
	/*itoa(buf,'d',parent_inode);*/
	/*print_s("\nParent inode is ");*/
	/*print_s(buf);*/
	/*print_s("\n");*/

	if (parent_inode < 0){
		print_s("open: Invalid path...Aborting\n");
		return ERROR;
	}

	//OK ata this point we have both the parent and the file inodes
	
	
	/* Check the permissions*/

	// The permissions to open the file are not correct
	if (flags > fs->inode[file_inode].perm){
		print_s("open: Incorrect permissions\n");
		return ERROR;
	}
	if(flags == RO && fs->inode[file_inode].perm == WR){
		print_s("open: The file write only\n");
		return ERROR;
	}
	if (flags == RW && fs->inode[file_inode].perm != RW){
		print_s("open: The file cannot be opened as read write\n");
		return ERROR;
	}
	
	/*Assign a valid descriptor to the file*/
	for (int i = 0; i <= MAX_FILES; i++){
		if (glob_fdt_ptr[i].in_use == FREE){
			glob_fdt_ptr[i].in_use = USED;
			glob_fdt_ptr[i].flags = flags;
			glob_fdt_ptr[i].pos_ptr = 0;
			glob_fdt_ptr[i].inode = file_inode;
			fs->inode[file_inode].opened++;
			
			// Return the index to the fdt
			return i;
		}
	}
	
	print_s("Non available file descriptors for the file\n");
	
	if ( threads == 1 )
		asm volatile("sti");

	return ERROR;
}

int rd_close(int fd){
	asm volatile("cli");
	int inode_num;

	if (glob_fdt_ptr[fd].in_use == FREE){
		print_s("close: The file your are trying to close does not exist\n");
		return ERROR;
	}
	
	inode_num = glob_fdt_ptr[fd].inode;
	fs->inode[inode_num].opened--;

	glob_fdt_ptr[fd].in_use = FREE;
	glob_fdt_ptr[fd].flags = JUNK;
	glob_fdt_ptr[fd].pos_ptr = 0;
	glob_fdt_ptr[fd].inode = JUNK;

	if ( threads == 1 )
		asm volatile("sti");

	return 0;
}

int rd_read(int fd, char *address, int num_bytes){

	asm volatile("cli");
	int res;
	int num_of_blocks; 				//num of blocks to write
	int extra_bytes; 				//Bytes of the last block
	int inode_num; 					//Inode number for the file
	uint32_t pos_ptr; 				//Position to start in the file
	int flags; 					//File permissions
	int cur_block; 					//Current block to start writing from
	int last_blk_ofst; 				//Offset in the cur block
	block_t *block_ptr;
	char temp[16];
	int fl_size; 					//Size of file
	int cur_pos_ptr; 				//Cur pos in the file
	int bytes_read = 0; 				

	/*print_s("File descriptor \n");*/
	/*itoa(temp,'d',fd);*/
	/*print_s(temp);*/
	/*print_s("\n");*/
	
	/*FD does not exist*/
	if (glob_fdt_ptr[fd].in_use == FREE){
		print_s("read: The file you are trying to write to does not exist\n");
		return ERROR;
	}
	
	/*Read only file*/
	flags = glob_fdt_ptr[fd].flags;
	inode_num = glob_fdt_ptr[fd].inode;
	
	/*print_s("Inode \n");*/
	/*itoa(temp,'d',inode_num);*/
	/*print_s(temp);*/
	/*print_s("\n");*/

	if (flags == WR || fs->inode[inode_num].perm == WR){
		print_s("read: The file is opened as writeonly\n");
		return ERROR;
	}
	
	/*Case of a directory*/
	if (fs->inode[inode_num].type == DR){
		print_s("read: You cannot read a directory file\n");
		return ERROR;
	}

	pos_ptr = glob_fdt_ptr[fd].pos_ptr;
	cur_pos_ptr = pos_ptr;
	fl_size = fs->inode[inode_num].size;
	
	cur_block = pos_ptr / 256; 		//Number of blocks already written
	last_blk_ofst = pos_ptr % 256; 		//Offset in the last block
	
	
	if (pos_ptr == fl_size){
		print_s("read: We are at the end of the file...use lseek first\n");
		return 0;
	}

	res = find_block(cur_block,&block_ptr,inode_num); //Find the current block;
	
	if (res < 0){
		print_s("read: Cannot find a location pointer for this block\n");
		return ERROR;
	}
	
	char *cur_byte = ((char*) (block_ptr)) + last_blk_ofst;
	
	while(bytes_read < num_bytes){
		
		address[bytes_read] = *cur_byte;
		bytes_read++;
		cur_byte++;
		cur_pos_ptr++;
		
		if (bytes_read + pos_ptr >= fl_size){
			print_s("HEREEEEEADFASFASDF\n");
			break;
		}
		
		// We need to go to the next block
		if (cur_pos_ptr % 256 == 0){
			cur_block++;
			res = find_block(cur_block,&block_ptr,inode_num); //Find the current block;
			cur_byte = (char*) (block_ptr);
		}

	}
	glob_fdt_ptr[fd].pos_ptr = cur_pos_ptr;

	if ( threads == 1 )
		asm volatile("sti");

	return bytes_read;
}


int rd_write(int fd, char *address, int num_bytes){

	asm volatile("cli");
	int res;
	int num_of_blocks; 				//num of blocks to write
	int extra_bytes; 				//Bytes of the last block
	int inode_num; 					//Inode number for the file
	uint32_t pos_ptr; 				//Position to start in the file
	int flags; 					//File permissions
	int cur_block; 					//Current block to start writing from
	int cur_ofst; 				//Offset in the cur block
	block_t *block_ptr;
	char temp[16];
	int fl_size; 					//Size of the file
	int blocks_alloc; 				//Blocks allocated by the file
	int bytes_written = 0;
	
	
	print_s("Num of bytes to write \n");
	itoa(temp,'d',num_bytes);
	print_s(temp);
	print_s("\n");

	/*FD does not exist*/
	if (glob_fdt_ptr[fd].in_use == FREE){
		print_s("write: The file you are trying to write to does not exist\n");
		return ERROR;
	}
	
	/*Read only file*/
	flags = glob_fdt_ptr[fd].flags;
	inode_num = glob_fdt_ptr[fd].inode;

	if (flags == RO || fs->inode[inode_num].perm == RO){
		print_s("write: The file is opened as readonly\n");
		return ERROR;
	}
	
	/*Case of a directory*/
	if (fs->inode[inode_num].type == DR){
		print_s("write: You cannot write to a directory file\n");
		return ERROR;
	}
	
	/*Get the size of the file*/
	fl_size = fs->inode[inode_num].size;

	if (fl_size == MAX_FILE_SIZE){
		print_s("write: File has max size\n");
		return ERROR;
	}

	blocks_alloc = fl_size / 256 + 1; 	//Will be needed to see if we have to allocate a new block or not

	// Find the block where the write is going to begin based on the pos pointer
	pos_ptr = glob_fdt_ptr[fd].pos_ptr;
	
	cur_block = pos_ptr / 256; 		//Block index to start
	cur_ofst = pos_ptr % 256; 		//Offset in that block
	
	itoa(temp,'d',cur_block);
	print_s("Current block index ");
	print_s(temp);
	print_s("\n");
	
	// We are at the end of the file so we have to allocate new blocks
	if (pos_ptr >= fl_size && fl_size < MAX_FILE_SIZE && pos_ptr != 0){
		//cur_block++;
		res = allocate_block(cur_block,inode_num);

		if(res < 0){
			print_s("write: No more available blocks to allocate\n");
			return ERROR;
		}
		cur_ofst =0;
		blocks_alloc++;
		itoa(temp,'d',blocks_alloc);
		print_s("Blocks allocated till now ");
		print_s(temp);
		print_s("\n");
	}

	res = find_block(cur_block,&block_ptr,inode_num); //Find the current block;
	
	if (res < 0){
		print_s("write: Cannot find a location pointer for this block\n");
		return ERROR;
	}
	
	/*print_s("In write Current block is \n");*/
	/*itoa(temp,'d',block_ptr);*/
	/*print_s(temp);*/
	/*print_s("\n");*/

	init_block(block_ptr, fs->inode[inode_num].type); // Init the block



	/*Current byte in the block that we are able to write to*/
	char *cur_byte = ((char*) (block_ptr)) + cur_ofst;

	while(bytes_written < num_bytes && fs->inode[inode_num].size < MAX_FILE_SIZE){
		*cur_byte = address[bytes_written];
		bytes_written++;
		cur_byte++;
		pos_ptr++;
		fs->inode[inode_num].size++;
		if (bytes_written == num_bytes)
			break;
		//We are at the end of the last block so we need to allocate a new one
		if (pos_ptr % 256 ==0){
			if (pos_ptr / 256 + 1 > blocks_alloc){
				itoa(temp,'d',bytes_written);
				/*print_s("Bytes written up to this point ");*/
				/*print_s(temp);*/
				/*print_s("\n");*/
				cur_block++;
				/*print_s("Allocating new block\n");*/
				/*itoa(temp,'d',cur_block);*/
				/*print_s("New block index \n");*/
				/*print_s(temp);*/
				/*print_s("\n");*/
				res = allocate_block(cur_block,inode_num);
				

				if(res < 0){
					/*itoa(temp,'d',blocks_alloc);*/
					/*print_s("Blocks allocated till now ");*/
					/*print_s(temp);*/
					/*print_s("\n");*/
					
					/*itoa(temp,'d',bytes_written);*/
					/*print_s("Bytes written till now ");*/
					/*print_s(temp);*/
					/*print_s("\n");*/
					
					print_s("write: No more available blocks to allocate\n");
					return ERROR;
				}
				res = find_block(cur_block,&block_ptr,inode_num); //Find the current block;
				
				/*itoa(temp,'d',block_ptr);*/
				/*print_s("Current block pointer ");*/
				/*print_s(temp);*/
				/*print_s("\n");*/
				
				init_block(block_ptr, fs->inode[inode_num].type);

				blocks_alloc++;
				cur_byte = (char*) block_ptr;
			}
			// Else just go to the next block
			else{
				/*print_s("Going to the next block\n");*/
				cur_block++;
				res = find_block(cur_block,&block_ptr,inode_num); //Find the current block;
				cur_byte = (char*) block_ptr;
			}
		}
	}

	glob_fdt_ptr[fd].pos_ptr = pos_ptr;
	
	/*itoa(Vtemp,'d',blocks_alloc);*/
	/*print_s("Blocks allocated till now ");*/
	/*print_s(temp);*/
	/*print_s("\n");*/


	if ( threads == 1 )
		asm volatile("sti");

	return bytes_written;

}


int rd_lseek(int fd, int offset){

	asm volatile("cli");
	int inode_num = glob_fdt_ptr[fd].inode; 					//Inode number for the file
	int file_size;

	inode_num = glob_fdt_ptr[fd].inode;
	
	/*FD does not exist*/
	if (glob_fdt_ptr[fd].in_use == FREE){
		print_s("lseek: The file you are trying to write to does not exist\n");
		return ERROR;
	}
	
	/*Case of a directory*/
	if (fs->inode[inode_num].type == DR){
		print_s("lseek: You cannot seek to a directory file\n");
		return ERROR;
	}
	
	
	file_size = fs->inode[inode_num].size;
	
	if (offset > file_size){
		glob_fdt_ptr[fd].pos_ptr = file_size;
	}
	else{
		glob_fdt_ptr[fd].pos_ptr = offset;
	}

	if ( threads == 1 )
		asm volatile("sti");

	return 0;

}

int rd_unlink(char *pathname){
	asm volatile("cli");	
	char buf[16];

	int res = 0;
	char filename[14];
	int parent_inode;
	int type, opened,size;
	int file_inode;

	//Check if we are trying to create the root directory
	if (strcmp(pathname,root->filename) == 0){
		print_s("unlink: You cannot unlink root directory!!!");
		return ERROR;
	}
	
	//Get the parent's and the file's inode number

	file_inode = check_pathname(pathname,&parent_inode,filename);
	
	if (file_inode < 0 && parent_inode < 0){
		print_s("unlink: Invalid path\n");
		return ERROR;
		
	}
	
	//We got the info...Make some checks: flags, opened

	type = fs->inode[file_inode].type;
	opened = fs->inode[file_inode].opened;
	size = fs->inode[file_inode].size;

	if (opened > 0 || (type == DR && size != 0)){
		print_s("unlink: Cannot delete file or directory\n");
		return ERROR;
	}
	
	/*First update the parent directory*/
//	print_s("Before updating the parent\n");

	res = update_parent(parent_inode,filename, DL, DR, JUNK);
	
	if (res < 0){
		print_s(filename);
		print_s("unlink: Cannot delete the entry for this file...Aborting\n");
		return ERROR;
	}
	
	int block_nums, blk_indx;
	
	//Number of blocks currently allocated for the file
	block_nums = fs->inode[file_inode].size / 256 ;
	

	/*Now delete the inode and all its data*/
	block_t *blk_ptr;
	
	/*print_s("Deleting inode ");*/
	/*itoa(buf,'d',file_inode);*/
	/*print_s(buf);*/
	/*print_s("\n");*/
	// Set the pointer to the first block of the file
	for (int i=0; i< block_nums; i++){
		res = find_block(i,&blk_ptr,file_inode);
		if (res < 0){
			print_s("\nunlink: Something went terribly wrong when unlinking the file\n");
			return ERROR;
		}
		

		blk_indx = (blk_ptr - &fs->d_blks[0]);
		/*print_s("Block index to be deleted ");*/
		/*itoa(buf,'d',blk_indx);*/
		/*print_s(buf);*/
		/*print_s("\n");*/
		deallocate_block(blk_indx);

	}
	

	/*Now delete the inode*/
	fs->inode[file_inode].in_use = FREE;
	fs->inode[file_inode].type = 0;
	fs->inode[file_inode].size = 0;
	fs->inode[file_inode].perm = JUNK;
	fs->inode[file_inode].opened = 0;
	
	for (int i =0; i< 10; i++){
		fs->inode[file_inode].location[i] = 0;
	}

	fs->superblock.free_inodes++;

	if ( threads == 1 )
		asm volatile("sti");

	//print_s("unlink: File deleted\n");
}

int rd_chmod(char *pathname, mode_t mode){

	asm volatile("cli");
	int res = 0;

	char filename[14];
	int parent_inode, file_inode;
	char buf[16];

	if (strcmp(pathname,root->filename) == 0){
		print_s("chmod: You cannot change the permissions of the root directory\n");
		return ERROR;
	}
	
	// Get the file's and parent's inode numbers
	file_inode = check_pathname(pathname,&parent_inode,filename);
	
	if (file_inode < 0 && parent_inode < 0){
		print_s("chmod: Invalid path\n");
		return ERROR;
		
	}
	
	/*itoa(buf,'d',parent_inode);*/
	/*print_s("Parent inode is ");*/
	/*print_s(buf);*/
	/*print_s("\n");*/

	/*Updating the mode*/
	fs->inode[file_inode].perm = mode;

	print_s("chmod: File mode updated\n");

	if ( threads == 1 )
		asm volatile("sti");


}

int find_block(int block_num, block_t** cur_block, int inode){
	//We reached the blocks limit for a single inode 
	if (block_num == MAX_INODE_BLOCKS){
//		print_s("We reached the max number of blocks for this inode...\n");
		return ERROR;
	}
	
	char temp[16] ;

	if(block_num <= 7){
		*cur_block = (block_t*) fs->inode[inode].location[block_num];
		/*itoa(temp, 'd',inode);*/
		/*print_s("Inode ");*/
		/*print_s(temp);*/
		/*itoa(temp, 'd',block_num);*/
		/*print_s("\nBlock num ");*/
		/*print_s(temp);*/
		/*itoa(temp, 'd',*cur_block);*/
		/*print_s("\nIn find_block address is ");*/
		/*print_s(temp);*/
		/*print_s("\n");*/

		return 0;
	}
	//First indirection
	else if (block_num <= 71){
		block_t *indirect;
		indirect = fs->inode[inode].location[8];
		*cur_block =  (block_t*) (indirect + block_num - 8);
		/*itoa(temp, 'd',inode);*/
		/*print_s("Indirect 1 Inode ");*/
		/*print_s(temp);*/
		/*itoa(temp, 'd',block_num);*/
		/*print_s("\nBlock num ");*/
		/*print_s(temp);*/
		/*itoa(temp, 'd',*cur_block);*/
		/*print_s("\nIn find_block address is ");*/
		/*print_s(temp);*/
		/*print_s("\n");*/
		return 0;
	}
	//Second indirection
	else if (block_num <= MAX_INODE_BLOCKS - 1){
		int ind_1 = (block_num - 72) / 64;
		int ind_2 = (block_num - 72) % 64;
		block_t *indirect_1;
		indirect_1 = fs->inode[inode].location[9];
		block_t *indirect_2;
		indirect_2 = indirect_1 + ind_1;
		*cur_block = (block_t*) (indirect_2 + ind_2);
		/*print_s("Inode ");*/
		/*print_s("Indirect 2 Inode ");*/
		/*print_s(temp);*/
		/*itoa(temp, 'd',block_num);*/
		/*print_s("\nBlock num ");*/
		/*print_s(temp);*/
		/*itoa(temp, 'd',*cur_block);*/
		/*print_s("\nIn find_block address is ");*/
		/*print_s(temp);*/
		/*print_s("\n");*/
		return 0;
	}
	

}


// Check if the File/Dir with name exist under parent with inode par_inode
int check_if_exists(char name[],int par_inode){
	
	/*TODO*/
	// Maybe add a check if the parent is full or not
	// Because maybe we cannot add the new entry to parent even if there 
	// is space for it in general (inodes, blocks)

	//Case where the parent inode is a file
	if(fs->inode[par_inode].type == FL){
		print_s("The parent is a regular file...Invalid path!\n");
		return ERROR;
	}
	// The parent is empty so the file/dir definitely does not exist
	if(fs->inode[par_inode].size == 0){
		return ERROR;
	}

	//We have to search in the parent if the file exists or not
	block_t *cur_block;
	cur_block = (block_t*) fs->inode[par_inode].location[0]; 	//Start from parent's first block
	int block_num = 0;      					//First block (will help with indirections)
	int status = 0;

//	char tmp[10] = "";
//	itoa(tmp,'d', par_inode);
//	print_s("the parent inode is ");
//	print_s(tmp);
//	print_s("\n");
	while (status != -1){
		for (int i = 0; i < 16; i++){
			dir_t *entry;  					//Temporary entry struct to extract the inode num
			//&(fs->inode[parent_inode].location[block_num])
			//cur_block = (block_t*) fs->inode[par_inode].location[block_num];        //Start from parent's first block
			entry = (dir_t *) (cur_block);
			entry += i;
			//The file/dir we are looking for exists in this block...Yay!
			char tmp[14] = "";
			/*print_s("ENTRY INODE NUM ");*/
			/*itoa(tmp,'d',entry->inode_num);*/
			/*print_s(tmp);*/
			/*print_s("\n");*/
			/*print_s("ENTRY_FILENAME ");*/
			/*print_s(entry->filename);*/
			/*print_s("\n");*/
			/*print_s("ENTRY_ADDRESS ");*/
			/*itoa(tmp,'16',entry);*/
			/*print_s(tmp);*/
			/*print_s("\n");*/
			if (strncmp(entry->filename, name, strlen(name)) == 0){
			//	print_s("File found!\n");
				return (int) entry->inode_num; 			//Return the inode number
			}
		}
		block_num++;
		status = find_block(block_num,&cur_block,par_inode);
//		char tmp[15];
//		itoa(tmp,'d',block_num);
//		print_s("the block num is");
//		print_s(tmp);
//		print_s("\n");
	}

	return ERROR;
}


int check_pathname(char *pathname, int *parent_inode, char filename[]){
	char buf[16];
	
	*parent_inode = ERROR; 		//Set the parent's inode to be return to ERROR

	int par_inode = 0; 		// Start from the root inode
	int status = 1;
	int path_len;
	path_len = strlen(pathname);
	itoa(buf,'d',path_len);

	if (pathname[0] != '/'){
		print_s("Pathnames must start with /...Abort!\n");
		return ERROR;
	}
	
	//Only '/' in pathname
	if (path_len == 1){
		print_s("Cannot create nor delete root...Abort!\n");
		return ERROR;
	}

//	print_s("After checks 1\n");
	//Get the length of the path to know if we are at the end...

	int name_finish = 0;
	int final = 0 ; 		//Flag to check if we are at the last name or not
	while (final != 1 && status != ERROR){
		/* First get every directory in the path and check if they exist */
		char temp_name[14];
		int i = 0;
		name_finish++;
		while(pathname[name_finish] != '/' && pathname[name_finish] != '\0'){
			temp_name[i] = pathname[name_finish];
			name_finish++;
			i++;
		}
		temp_name[i] = '\0';
//		name_finish++;
		/*print_s("name_finish is ");*/
		/*char tmp[10];*/
		/*itoa(tmp,'d', name_finish);*/
		/*print_s(tmp);*/
		/*print_s("\n");*/
		/*print_s("parthname is ");*/
		/*print_s(pathname);*/
		/*print_s("\n");*/
		/*print_s("Temp name is ");*/
		/*print_s(temp_name);*/
		/*print_s("\n");*/
		if (name_finish == path_len){
//			print_s("Here1\n");
			final = 1;
			strncpy(filename,temp_name,strlen(temp_name));
		}

		//This is a directory name
		if (pathname[name_finish] == '/'){
//			print_s("Here2\n");
			temp_name[i] = '\0'; 		//Null terminator for name
		}

		/*print_s("Before check if exists\n");*/
		//Check if the file exists and return its inode if it exists else ERROR
		status = check_if_exists(temp_name,par_inode);
		
		//Case that the name requested does not exist yay
		if (status == ERROR && final == 1){
	//		print_s("The path is valid and the file does not exist\n");
			*parent_inode = par_inode;
			return ERROR;
			
		}
		// Case where the parent is a regural file
		if (status != ERROR && final == 0){
			//print_s("Traversing the path\n");
			par_inode = status;
			/*continuing*/
		}
		// This is a problem...A directory in the path does not exist
		else if (status == ERROR && final == 0){
			//print_s("The path does not exist\n");
			*parent_inode = ERROR;
			return ERROR;
		}
		// The file/dir that we requested already exists...Return ERROR
		else if (status != ERROR && final == 1) {
			//print_s("The file/directory that you requested already exists!\n");
			*parent_inode = par_inode;
			return status; 				//This is actually the inode of the file
		}


	}
	return ERROR;
	
	
}


void init_block(block_t* cur_block, int type){
	if (type == DR){
		dir_t *entry;
		entry = (dir_t*) cur_block;

		for (int i =0; i<16; i++){
			entry->inode_num = JUNK;
			for (int j=0; j< 14; j++){
				entry->filename[j] = '\0';
			}
			entry++;
		}
	}
	else{
		char* byte;
		byte = (char*) cur_block;

		for (int i=0 ; i< 256; i++){
			*byte = '\0';
		}
		byte++;
	}

}


int allocate_inode(uint32_t type, uint32_t permissions){
	/*Loop through the inode array to see if there are free inodes*/
	int ret = ERROR;
	int status;
	for (int i = 1; i <= MAX_FILES; i++){
		if (fs->inode[i].in_use == 0){
			ret = i;
			status = allocate_block(0,i);

			if (status < 0){
				print_s("There is no block available...Aborting\n");
				return ERROR;
			}

			fs->inode[i].type = type;
			

			fs->inode[i].size = 0;
			fs->inode[i].perm = permissions;
			fs->superblock.free_inodes--; 				//Decrease available inodes
			fs->inode[i].in_use = 1;
			fs->inode[i].opened = 0;
//			char tmp[10];
//			itoa(tmp,'d', ret);
//			print_s("the allocated inode is ");
//			print_s(tmp);
//			print_s("\n");
			return ret;
		}
	}
	return ret;
}

void deallocate_block(int indx){
	int actual_blk;
	int btmp_indx;
	int bit_indx;
	uint8_t *bmap_ptr;
	
	char buf[16];
	actual_blk = indx + 261;
	/*itoa(buf,'d',actual_blk);*/
	/*print_s("Block ");*/
	/*print_s(buf);*/
	/*print_s("\n");*/
	
	btmp_indx = actual_blk / 8;
	/*itoa(buf,'d',btmp_indx);*/
	/*print_s("Block index ");*/
	/*print_s(buf);*/
	/*print_s("\n");*/
	bit_indx = actual_blk % 8;
	/*itoa(buf,'d',bit_indx);*/
	/*print_s("Bit index ");*/
	/*print_s(buf);*/
	/*print_s("\n");*/
	
	bmap_ptr = &fs->bitmap[btmp_indx];
	*bmap_ptr ^= (0x80 >> bit_indx);

	fs->superblock.block_num++;
	
	return;

}


int get_available_block(){
	
	int block_num ;
	uint8_t *bmap_ptr;
	int bit;
	//The first 32 * 8 = 256 blocks are allocated for the superblock inodes
	//It is actually the first 261 blocks, so we need to start from index 32
	//in the bitmap cause there are some unmapped blocks in that byte
	//
	//show_bitmap();
	if (!exist_blocks)
		return ERROR;

	for (int i = 32; i< BITMAP_SIZE; i++){
		for (int j =0; j< 8; j++){
			bmap_ptr = &fs->bitmap[i];
			bit = (*bmap_ptr << j) & 0x80;
			// Case of a free block in bitmap
			if (bit == 0){
				
				block_num = i * 8 + j - 261;
				*bmap_ptr |= (0x80 >> j);
				fs->superblock.block_num--; 			//Decrease available blocks
				
				return block_num;
			}
		}
	}
	return ERROR;

}


int allocate_block(int block_num, int inode){
	// No need to do anything if there are not any blocks
	if(!exist_blocks())
		return ERROR;
	
	char buf[16];

	int blk_indx;
	// No indirection
	if (block_num < 8){
		blk_indx = get_available_block();
		fs->inode[inode].location[block_num] = &fs->d_blks[blk_indx];
	}
	// First indirection
	else if (block_num < 72){
		block_t *blk_pointers; 			//256 bytes
		// First indirection init
		if (block_num == 8){
			int ind_1 = get_available_block(); 		//One block for indirection
			if (ind_1 < 0){
				print_s("Not available blocks for IND1\n");
				return ERROR;
			}

			blk_indx = get_available_block(); 		//One block for data
			if(blk_indx < 0){
				print_s("No more blocks for IND1 data\n");
				deallocate_block(ind_1);
				return ERROR;
			}

			
			blk_pointers =  &fs->d_blks[ind_1];
			/*for (int i = 0; i< 64; i++){*/
				/**(blk_pointers + i) = 0;*/
				/*itoa(buf,'d',&blk_pointers[i]);*/
				/*print_s("The blk_pointer indx addr is ");*/
				/*print_s(buf);*/
				/*print_s("\n");*/
			/*}*/

			*(blk_pointers+0) = fs->d_blks[blk_indx];
			fs->inode[inode].location[8] = &fs->d_blks[ind_1];
			
		}
		else{
			blk_indx = get_available_block(); 		//One block for data
			if(blk_indx < 0){
				print_s("No more blocks for IND1 data\n");
				return ERROR;
			}
			
			blk_pointers = fs->inode[inode].location[8];
			/*itoa(buf,'d',blk_pointers);*/
			/*print_s("The start address of blk_pointers is ");*/
			/*print_s(buf);*/
			/*print_s("\n");*/
			/**(blk_pointers + block_num - 8) = fs->d_blks[blk_indx];*/
			/*itoa(buf,'d',(blk_pointers + block_num -8));*/
			/*print_s("The new blocks address is ");*/
			/*print_s(buf);*/
			/*print_s("\n");*/
		}
		//Check if the actual number of blocks is > 63 // We already allocated the first block

	}
	// Second indirection
	else if(block_num < MAX_INODE_BLOCKS){
		int ind1 = (block_num - 72) / 64;
		int ind2 = (block_num - 72) % 64;
		block_t *blk_pointers; 			//256 bytes
		block_t *blk_pointers2; 			//256 bytes
		
		// Nothing has been done for indirection 2, bith ind_1 and ind_2 = 0 block 72
		if(ind1 == 0 && ind2 == 0){
			

			int ind_1 = get_available_block(); 		//One block for indirection
			if (ind_1 < 0){
				print_s("Not available blocks for IND1\n");
				return ERROR;
			}
			
			blk_pointers = &fs->d_blks[ind_1];
			/*for (int i = 0; i< 64; i++){*/
				/*blk_pointers[i] = JUNK;*/
			/*}*/
			

			int ind_2 = get_available_block();
			if (ind_2 < 0){
				print_s("Not available blocks for IND2\n");
				deallocate_block(ind_1);
				return ERROR;
			}
			
			blk_pointers2 = &fs->d_blks[ind_2];
			/*for (int i = 0; i< 64; i++){*/
				/*blk_pointers2[i] = JUNK;*/
			/*}*/
			
			// Finally get a data block
			blk_indx = get_available_block();
			if (blk_indx < 0){
				print_s("Not available blocks for IND2\n");
				deallocate_block(ind_1);
				deallocate_block(ind_2);
				return ERROR;
			}
		
			
			// Update the first of the second indirect pointers to point to the data block
			*(blk_pointers2 + 0) = fs->d_blks[blk_indx];
			
			/*itoa(buf,'d',(blk_pointers2 + 0));*/
			
			/*print_s("The new ind2 blocks address is ");*/
			/*print_s(buf);*/
			/*print_s("\n");*/

			//Finally set the last location pointer
			
			fs->inode[inode].location[9] = &fs->d_blks[ind_1];
		}
		// A new double indirection 
		else if(ind2 == 0){
			
			

			blk_pointers = fs->inode[inode].location[9];
			
			int ind_2 = get_available_block();
			if (ind_2 < 0){
				print_s("Not available blocks for IND2\n");
				return ERROR;
			}
			
			blk_pointers2 = &fs->d_blks[ind_2];
			/*for (int i = 0; i< 64; i++){*/
				/*blk_pointers2[i] = JUNK;*/
			/*}*/
			
			// Finally get a data block
			blk_indx = get_available_block();
			if (blk_indx < 0){
				print_s("Not available blocks for IND2\n");
				deallocate_block(ind_2);
				return ERROR;
			}
		
			// Set the double indirection pointer
			*(blk_pointers + ind1 ) = fs->d_blks[ind_2];

			// Update the first of the second indirect pointers to point to the data block
			*(blk_pointers2 + 0) = fs->d_blks[blk_indx];
			
			/*itoa(buf,'d',(blk_pointers2 + 0));*/
			
			/*print_s("The new ind2 blocks address is ");*/
			/*print_s(buf);*/
			/*print_s("\n");*/
		}
		// Just update a second indirection
		else{
			blk_pointers = fs->inode[inode].location[9];
			blk_pointers2 = blk_pointers + ind1;
			
			blk_indx = get_available_block();
			if (blk_indx < 0){
				print_s("Not available blocks for IND2\n");
				return ERROR;
			}

			// Update the first of the second indirect pointers to point to the data block
			*(blk_pointers2 + ind2) = fs->d_blks[blk_indx];
			
			/*itoa(buf,'d',(blk_pointers2 + ind2));*/
			
			/*print_s("The new ind2 blocks address is ");*/
			/*print_s(buf);*/
			/*print_s("\n");*/
			
		}


	}
	// We are way outside the boundaries
	else{
		return ERROR;
	}

	return 0;
}

//We got the parent inode and the filename
int update_parent(int parent_inode, char* filename, int action, uint32_t type, uint32_t perm){
	//Depending on the parent's size we can find the correct location pointer
	//No space left in the parent
	if (fs->inode[parent_inode].size == MAX_FILE_SIZE){
		print_s("Parent has no space for a new entry\n");
		return ERROR;
	}
	
	dir_t *entry; 					//Temp pointer to update smth
	
	//If we want to create something, the search is smoother
	if (action == CR){
		//First we find how many blocks the parent has, then based on the block
		//number we find which location pointer to use
		int res;
		int block_num = fs->inode[parent_inode].size / 256; 	//Block number used
		int offset = fs->inode[parent_inode].size % 256;  	//Offset in the last block number
		block_t *cur_block;


		/*TODO*/
		//Update parent with the new block if needed

		// We have to allocate a new block for the parent 
		// Block 0 is always allocated
		if (offset == 0 && block_num != 0 ){
			/*print_s("HEREEEEEEE!!\n");*/
			
			//block_num++;
			offset = 0;
			char temp[16] ;
			/*itoa(temp, 'd',block_num);*/
			/*print_s("Block number is ");*/
			/*print_s(temp);*/
			/*print_s("\n");*/
			
			res = allocate_block(block_num, parent_inode);
			
			if (res < 0){
				print_s("Not available blocks to store the new entry\n");
				return ERROR;
			}

			res = find_block(block_num,&cur_block,parent_inode);
			
			if (res < 0){
				print_s("Did not find the block requested\n");
				return ERROR;
			}
			
			entry = (dir_t *) (cur_block);
			//Initialize the block
			for (int i =0; i< 16; i++){
				for (int j =0; j< 14; j++)
					entry->filename[j] = '\0';
				entry->inode_num = JUNK;
				entry++;
			}
					
			//Initialize the new block for the parent
			//init_block(cur_block,parent_inode);
//			char temp[16] ;
//			itoa(temp, 'd',cur_block);
//			print_s("In update parent address is ");
//			print_s(temp);
//			print_s("\n");


			
			// Set the entry to the beginning of the new block again
			entry = (dir_t *) (cur_block);
			

			// Allocate a new inode for the new file
			int inode_num;
			/*print_s("Allocating inode\n");*/
			inode_num = allocate_inode(type, perm);
			
			if (inode_num < 0){
				print_s("Not any available inodes for the new entry\n");
				deallocate_block(block_num);
				return ERROR;
			}
			
			// Its a mess but we need to init the new block
			block_t* temp_ptr;
			res = find_block(0,&temp_ptr,inode_num);
			if (res < 0){
				print_s("The new inode did not properly allocate a block\n");
				return ERROR;
			}
			//Init the block
			init_block(temp_ptr,type);
			
			strncpy(entry->filename,filename,strlen(filename)); 		// Security is everything :)
			entry->inode_num = inode_num;
			fs->inode[parent_inode].size +=16;
			
			return 0;
			
		}
		// We have space at one block for the parent
		// We have to find which
		else{
			//print_s("HEREEEEEEE222222!!\n");
			
			for (int i = 0; i <= block_num; i++){
				res = find_block(i,&cur_block,parent_inode);
				
				if (res < 0){
					print_s("Something went wrong when finding the current block pointer for the parent\n");
					return ERROR;
				}
				//print_s("HEREEEEEEE33333333!!\n");
			
				for (int i = 0; i < 16; i++){
					
					entry = (dir_t *) (cur_block);
					entry += i;
					
					// If a space is found the update the entry
					if (entry->inode_num == JUNK){
						char buf[16];
						/*print_s("Entry space found!\n");*/
						
						// Allocate a new inode for the new file
						int inode_num;
						/*print_s("Allocating inode\n");*/
						inode_num = allocate_inode(type, perm);
			//			print_s("HEREEEEEEE44444!!\n");
						
						if (inode_num < 0){
							print_s("Not any available inodes for the new entry\n");
							return ERROR;
						}
						
						// Its a mess but we need to init the new block
						block_t* temp_ptr;
						res = find_block(0,&temp_ptr,inode_num);
						if (res < 0){
							print_s("The new inode did not properly allocate a block\n");
							return ERROR;
			//			print_s("HEREEEEEEE55555!!\n");
						}
						//Init the block
						init_block(temp_ptr,type);

						/*print_s("INODE NUMVER ALLOCATED ");*/
						/*itoa(buf,'d',inode_num);*/
						/*print_s(buf);*/
						/*print_s("\n");*/

						strncpy(entry->filename,filename,strlen(filename)); 	
						entry->inode_num = inode_num;
						fs->inode[parent_inode].size +=16;
						return 0;
					}
				}

			}
		
		}
	}
	// We want to delete the entry
	else if (action == DL){
		//The search is similar to the check_if_exists
		int par_blks = fs->inode[parent_inode].size / 256 + 1;

		block_t *cur_block;
		cur_block = (block_t*) fs->inode[parent_inode].location[0]; 	//Start from parent's first block
		int block_indx = 0;      					//First block (will help with indirections)
		int status = 0;

		while (status != -1){
			for (int i = 0; i < 16; i++){
				
				entry = (dir_t *) (cur_block);
				entry += i;
				if (strncmp(entry->filename, filename, strlen(filename)) == 0){
					/*print_s("File to be deleted found!\n");*/
					for (int j =0; j<14; j++){
						entry->filename[j] = '\0';
					}
					entry->inode_num = JUNK;
					fs->inode[parent_inode].size -=16;
					return 0;
				}
			}
			block_indx++;
			// Parent has no more blocks to search so exiting
			if (cur_block == 0){
				return ERROR;
			}
			status = find_block(block_indx,&cur_block,parent_inode);
		}
	}
	return ERROR;
}



int exist_blocks(void){
	
	if (fs->superblock.block_num > 0)
		return TRUE;
	else{
		return FALSE;
	}
}

int exist_inodes(void){
	
	if (fs->superblock.free_inodes > 0)
		return TRUE;
	else{
		return FALSE;
	}
	
}


void show_dir_info(int inode)
{
	int block_num = 0;
//	while (block_num != -1)
//	{
		block_t *cur_block;
		cur_block = (block_t*) fs->inode[inode].location[block_num]; 	//Start from parent's first block
		for (int i = 0; i < 16; i++)
		{
			dir_t *entry;  					//Temporary entry struct to extract the inode num
			//&(fs->inode[parent_inode].location[block_num])
			cur_block = (block_t*) fs->inode[inode].location[block_num];        //Start from parent's first block
			entry = (dir_t *) (&cur_block + i * 16);
			//The file/dir we are looking for exists in this block...Yay!
			char tmp[14] = "";
			itoa(tmp,'d',entry->inode_num);
			print_s(tmp);
			print_s("\n");
			print_s(entry->filename);
			print_s("\n");
			}
//		block_num = next_block(block_num,cur_block,inode);
//	}
}
void show_inode_info(int inode){
	char buf[16];
	inode_t *inode_ptr = &fs->inode[inode];
	print_s("\nShowing information for inode\n");
	itoa(buf,'d',inode);
	print_s(buf);
	print_s("\nInode type: ");
	itoa(buf,'d',inode_ptr->type);
	print_s(buf);
	print_s("\nInode size: ");
	itoa(buf,'d',inode_ptr->size);
	print_s(buf);
	print_s("\nInode location pointers: ");
	for (int i =0; i< 10; i++){
		itoa(buf,'16',inode_ptr->location[i]);
		print_s(buf);
		print_s(" ");
//		if ( i == 0 )
//			show_dir_info(inode);
	}
	print_s("\nInode permissions: ");
	itoa(buf,'d',inode_ptr->perm);
	print_s(buf);
	print_s("\nInode is used: ");
	itoa(buf,'d',inode_ptr->in_use);
	print_s(buf);
	print_s("\n");

}

void show_fd_object(int fd){
	char buf[16];
	
	itoa(buf,'d',fd);
	print_s("File descriptor ");
	print_s(buf);
	itoa(buf,'d',glob_fdt_ptr[fd].in_use);
	print_s("\nIn use ");
	print_s(buf);
	itoa(buf,'d',glob_fdt_ptr[fd].flags);
	print_s("\nFlags ");
	print_s(buf);
	itoa(buf,'d',glob_fdt_ptr[fd].pos_ptr);
	print_s("\nPosition pointer ");
	print_s(buf);
	print_s("\n");
}


void show_bitmap(void){
	char buf[16];

	for (int i =0; i<1024; i++){
		itoa(buf,'x',fs->bitmap[i]);
		print_s(buf);
		print_s(" ");
	}
}

