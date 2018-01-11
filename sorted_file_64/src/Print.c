#include "bf.h"
#include "sort_file.h"
#include <stdio.h>
#include <string.h>

int PrintBlock(int fileDesc,int block_num){
	if(block_num == -1){
		return -1;
	}
	BF_Block *block;
	BF_Block_Init(&block);
	char *data;
	int counter;
	SR_ErrorCode error;
	if((error=BF_GetBlock(fileDesc,block_num, block)) != BF_OK){                     //Get the first block
		BF_Block_Destroy(&block); 
	    BF_PrintError(error);                                                        //If fails
	    return -1;                                                              	 //Return error
	}
	data = BF_Block_GetData(block);
	memcpy(&counter,data,sizeof(int));
	int size = sizeof(Record);
	Record record;
	printf("Block: %d\n",block_num);
	for(int i=0;i<counter;i++){
		memcpy(&(record.id),&(data[sizeof(int)+size*i]),sizeof(int));
		memcpy(&(record.name),&(data[2*sizeof(int)+size*i]),sizeof(record.name));
		memcpy(&(record.surname),&(data[2*sizeof(int)+size*i+sizeof(record.name)]),sizeof(record.surname));
		memcpy(&(record.city),&(data[2*sizeof(int)+size*i+sizeof(record.name)+sizeof(record.surname)]),sizeof(record.city));
		printf("\t%d,%s,%s,%s\n",record.id,record.name,record.surname,record.city);
	}
	printf("-------------------------\n");
	int next_pointer;
	memcpy(&next_pointer,&(data[BF_BLOCK_SIZE-sizeof(int)]),sizeof(int));
	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);
	return next_pointer;
}