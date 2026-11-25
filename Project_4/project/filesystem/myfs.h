#ifndef MYFS
#define MYFS

// #define MAX_FILE_BLOCKS 16
#define HASH_SIZE 40
// #define MAX_FILES 10
#define BLOCK_SIZE 4096

typedef struct hash_info{
    char hash[HASH_SIZE+1]; //name of block file
    int touches;
}hash_info_t;

#endif