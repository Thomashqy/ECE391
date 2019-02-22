#include "lib.h"
#include "pcb.h"

#define SIZE_BOOTBLOCK_B 4096
#define SIZE_FILESYS_STAT_B 64
#define SIZE_DENTRY_BLOCK 64
#define SIZE_INODE_BLOCK_B 4096
#define SIZE_DATA_BLOCK_B 4096
#define MAX_DIR_ENTRIES 63
#define FILE_NAME_LENGTH_B 32


int32_t filesystem_open(){
	return 0;
}
int32_t filesystem_close(){
	return 0;
}
int32_t filesystem_read(int32_t fd, void* buf, int32_t nbytes)
{	
	pcb_t* pcb = get_pcb();
	uint32_t position = pcb->descriptors[fd].f_position;
	i_node_t* inode = pcb->descriptors[fd].f_inode;

	uint8_t* buf_temp = buf;
	if(inode == NULL) {
		int32_t i = 0;
		if(position < fs_stat->curr_num_dir_entries) {
			dentry_t den = dentry_array[position];
			for(i = 0; i < nbytes && i < FILE_NAME_MAX_NUM_CHAR; i++) {
				buf_temp[i] = den.file_name[i];
			}
			pcb->descriptors[fd].f_position++;
		}
		return i;
	}
		uint32_t inode_num = ((uint32_t)inode - (uint32_t)inode_array)/INODE_SIZE;
		int32_t bytes_read = read_data(inode_num, position, buf_temp, (uint32_t)nbytes);

		if(bytes_read != -1)
			pcb->descriptors[fd].f_position += bytes_read;

		return bytes_read;
}
int32_t filesystem_write(int32_t fd, void* buf, int32_t nbytes)
{
	return -1;
}

/*
* void init_file_system(module_t* ptr)
*   Inputs: 		module_t* ptr: the module containing information of the filesystem, such as the starting address
*					dentry_t* dentry: dentry struct, where we copy the values from the searched dentry to
*   Return Value: 	none
*	Function: 		Initializes the file system with parameters passed from a loaded module. It places 'placemarkers' along
* 					the memory that was allocated to us, denoting where the following arrays start: bootblock, filestatistic,
* 					dentry array, inode array, datablock array
*/
void init_file_system(module_t* ptr){
	
	bootblock 		= (boot_block_t*)				ptr->mod_start;
	fs_stat 		= (file_system_statistic_t*)	ptr->mod_start;
	dentry_array 	= (dentry_t*)					(ptr->mod_start + SIZE_FILESYS_STAT_B);
	inode_array 	= (i_node_t*)					(ptr->mod_start + SIZE_BOOTBLOCK_B);
	uint32_t num_of_inodes = fs_stat->curr_num_inodes;
	datablock_array = (datablock_t*)				(ptr->mod_start + SIZE_BOOTBLOCK_B + num_of_inodes * SIZE_INODE_BLOCK_B);
	return;
}

/*
* int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
*   Inputs: 		const uint8_t* fname: name of file to read
*					dentry_t* dentry: dentry struct, where we copy the values from the searched dentry to
*   Return Value: 	0: read success, -1: read failure
*	Function: 		search for the directory entries in bootblock by name, and if the search is successful,
* 					copy the searched dentry's values over to the dentry that was passed as an argument
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){

	// INITIALIZE SEARCH VARIABLES
	int idx;
	int entry_found = 0;

	// LOOP THROUGH DENTRIES LOCATED IN BOOTBLOCK
	for (idx = 0; idx < fs_stat->curr_num_dir_entries; idx++){ // including '.'

		// Use the strncmp function, it returns 0 if both strings are identical
		int search_success = strncmp((int8_t*)dentry_array[idx].file_name, (int8_t*)fname, FILE_NAME_LENGTH_B);
		// Stop search if directory entry is found
		if(search_success == 0){
			entry_found = 1;// if found, set flag entry_found to 1
			break;
		}
	}

	// CHECKS FOR SUCCESSUL READ -> fills up dentry struct if so, else return -1
	if (entry_found){ 	// Successful read
		strncpy((int8_t*)(dentry->file_name), (int8_t*)dentry_array[idx].file_name, FILE_NAME_LENGTH_B);// 'Append' to the dentry filename
		dentry->file_type = dentry_array[idx].file_type; 				// Can just assign directly
		dentry->inode_idx = dentry_array[idx].inode_idx;				// Can just assign directly 
		return 0;
	}
	else return -1; // Failure (non-existent file)
}

/*
* int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
*   Inputs: 		uint32_t index: index of the array of directory entries
*					dentry_t* dentry: dentry struct, where we copy the values from the searched dentry to
*   Return Value: 	0: read success, -1: read failure
*	Function: 		search for the directory entries in the dentry array by index, and if the search is successful,
* 					copy the searched dentry's values over to the dentry passed as an argument
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){

	int idx = index;
	// DEFINE CONSTRAINTS
	int max_index = fs_stat->curr_num_dir_entries -1;

	// CHECKS IF INDEX IS NOT WITHIN RANGE -> exits if so
	if (!(index >= 0 && index <= max_index)) return -1;

	// INDEX IS VALID -> fill up the dentry struct
	strncpy((int8_t*)(dentry->file_name), (int8_t*)dentry_array[idx].file_name, FILE_NAME_LENGTH_B); // 'Append' to the dentry filename
	dentry->file_type = dentry_array[idx].file_type; 				// Can just assign directly
	dentry->inode_idx = dentry_array[idx].inode_idx;				// Can just assign directly 
	return 0;

	// Note: Syntax for strncpy: int8_t* strncpy(int8_t* dest, const int8_t*src, uint32_t n);
}


/*
* int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
*   Inputs: 		uint32_t inode: the inode index in the i_node_array
*					uint32_t offset: the starting position of the data where we start to read data( like starting idx)
* 					uint8_t* buf: the buffer where we copy the data read from the datablocks to
* 					uint32_t length: number of bytes of data that was requested to be copied
*   Return Value: 	length-bytes_left_to_read: the number of bytes of data that was successfully copied to buf
					-1: read failure
*	Function: 		Based on the inode index and the offset, copy data from the corresponding datablocks of the inode for a certain
* 					length(bytes) of data
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){

	// CHECKS IF INDEX IS NOT WITHIN RANGE -> exits if so
	int max_index = fs_stat->curr_num_inodes - 1;
	if (!(inode >= 0 && inode <= max_index)) return -1;

	// CHECKS FOR BAD DATA BLOCK NUMBER WITHIN THE FILE BOUNDS OF GIVEN INODE
	int max_valid_idx_of_data_blocks = fs_stat-> curr_num_data_blocks - 1; 	// max valid idx of data blocks array in the file system
	int remaining_bytes_to_check = inode_array[inode].length_in_B;			// Check that within the length of the file, there are no invalid datablocks
	int idx = 0; 															// Initialize index to iterate through the datablocks array in the inode
	while (remaining_bytes_to_check > 0){
		int data_block_idx = inode_array[inode].datablocks_indexes[idx]; 				// Obtain the index of the datablock
		if (data_block_idx > max_valid_idx_of_data_blocks) return -1; 					// Check that the index is invalid -> return -1 if so
		idx += 1; 																		// increment the index, and go through the loop again
		remaining_bytes_to_check -= SIZE_DATA_BLOCK_B;									// decrement the number of bytes left to check for the file
	}

	// DEFINE ITERATION VARIABLES
	int bytes_left_to_read = length; 														// The number of bytes we have to read from the file
	int cur_data_block_in_inode	= offset / SIZE_DATA_BLOCK_B;								// the index in which we use to refer to the array of datablocks IN THE INODE
	int num_of_bytes_into_file = offset; 													// aka the file data pointer, max value should be filesize - 1
	int idx_in_buf = 0; 																	// current index in the buffer we are copying the data to
	int bytes_read = 0;

	while (num_of_bytes_into_file < inode_array[inode].length_in_B && bytes_left_to_read > 0 ){ 				// Check for the end of file
		int data_block_idx = inode_array[inode].datablocks_indexes[cur_data_block_in_inode];  					// Obtain the datablock index
		buf[idx_in_buf] = datablock_array[data_block_idx].blocks[num_of_bytes_into_file % SIZE_DATA_BLOCK_B]; 	// copy data over to the buffer
		bytes_left_to_read -= 1; 																				// decrement the number of bytes left to read
		num_of_bytes_into_file += 1; 																			// increment the file index pointer
		cur_data_block_in_inode = num_of_bytes_into_file / SIZE_DATA_BLOCK_B; 									// Update the datablock index in the inode
		idx_in_buf += 1; 																						// increment the index of the buffer
		bytes_read += 1; 																						// increment the count of the number of bytes read
	}
	return bytes_read;  	//number of bytes read
}

uint32_t get_length(uint32_t inode)
{
	// CHECKS IF INDEX IS NOT WITHIN RANGE -> exits if so
	int max_index = fs_stat->curr_num_inodes - 1;
	if (!(inode >= 0 && inode <= max_index)) return 0;
	
	return inode_array[inode].length_in_B;
}

