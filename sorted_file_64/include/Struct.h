#ifndef _STRUCT_H_
#define _STRUCT_H_
#include "bf.h"

typedef struct{
	BF_Block *block;
	int block_num;
	char *data;
	int num_of_blocks;
	int counter;
}Block_Data;

#endif