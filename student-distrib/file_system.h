#include "types.h"
#include "multiboot.h"

#define RESERVED_FS 13 // sets pf 4 bytes = 52 bytes
#define RESERVED_DENTRY 6 // sets of 4 bytes = 24 bytes
#define FILE_NAME_MAX_NUM_CHAR 32
#define SIZE_OF_DATA_BLOCK 4096
#define MAX_NUM_DIR_ENTRIES	63 // including '.'
#define MAX_NUM_DATABLOCKS_IN_INODE 1023 //First 4B of 4096B block is for the length of data
#define INODE_SIZE 4096

#define RTC_TYPE 0
#define DIR_TYPE 1
#define REG_FILE_TYPE 2

typedef struct file_system_statistic {
	uint32_t curr_num_dir_entries; 			
	uint32_t curr_num_inodes;
	uint32_t curr_num_data_blocks;
	uint32_t reserved[RESERVED_FS];  			
} file_system_statistic_t;

typedef struct dentry {
	uint8_t file_name[FILE_NAME_MAX_NUM_CHAR];  		
	uint32_t file_type;
	uint32_t inode_idx;
	uint32_t reserved[RESERVED_DENTRY];
} dentry_t;

typedef struct datablock{ 	
	uint8_t blocks[SIZE_OF_DATA_BLOCK];
}datablock_t;

typedef struct i_node {
	uint32_t length_in_B;
	uint32_t datablocks_indexes[MAX_NUM_DATABLOCKS_IN_INODE]; 		
} i_node_t;

typedef struct boot_block {
	file_system_statistic_t  file_system_stat;
	dentry_t dir_entries[MAX_NUM_DIR_ENTRIES]; // the first directory entry describes '.'
} boot_block_t;

// function declarations
void init_file_system(module_t* ptr);
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

uint32_t get_length(uint32_t inode);

int32_t filesystem_open();
int32_t filesystem_close();
int32_t filesystem_read(int32_t fd, void* buf, int32_t nbytes);
int32_t filesystem_write(int32_t fd, void* buf, int32_t nbytes);

boot_block_t* bootblock;
file_system_statistic_t* fs_stat;
i_node_t* inode_array;	// dependent on the module passed, in checkpt 2 it is 64
dentry_t* dentry_array;	// dependent on the module passed, in checkpt 2 it is 17
datablock_t* datablock_array;
