#include "Sort.h"
#include "bf.h"
#include "sort_file.h"
#include "QuickSort.h"
#include <string.h>
#include <stdlib.h>
#include "Compare.h"
#include "Struct.h"
#include <stdio.h>
#include <math.h>
#include <unistd.h>


int intlog(double base, double x) {
    return ceil((log(x) /(double)log(base)));
}


int SortBlock(int num_block,BF_Block *block,char *data,int m,int length,void *pivot_value,char type){
	if(num_block == -1){
		return -1;
	}
	int counter;
	memcpy(&counter,data,sizeof(int));
	int pivot = counter -1;
	int wall = 0;
	int size = sizeof(Record);
	int next_pointer = num_block;

	//Sort internal block with QuickSort
	QuickSort(data,size,wall,pivot,m,type,length,pivot_value);

	memcpy(&next_pointer,&(data[BF_BLOCK_SIZE-sizeof(int)]),sizeof(int));
	BF_Block_SetDirty(block);
	return next_pointer;
}



int Initialize_BlockData(Block_Data *block_data,int fd,int M,int num_block,int max_blocks,int N,
		int loop,int m,int length,void *pivot_value,char type){
	int i;
	for(i=0;i<M-1;i++){
		if(num_block == -1){
			break;
		}
		if(BF_GetBlock(fd,num_block,block_data[i].block) != BF_OK){  //Get the first block
		    BF_Block_Destroy(&block_data[i].block);
		    return SR_ERROR;         //Return error
		}

		//Initialize Block_Data structure
		block_data[i].data = BF_Block_GetData(block_data[i].block);
		block_data[i].counter = 0;
		block_data[i].num_of_blocks = 1;
		block_data[i].block_num = num_block;

		//If stage 0 sort internal blocks
		if(loop == 0){
			num_block = SortBlock(num_block,block_data[i].block,block_data[i].data,
				m,length,pivot_value,type);
		}
		else{
			num_block = num_block + max_blocks;
			if(num_block > N){
				num_block = -1;
			}
		}
	}
	return i;
}



int Find_min(Block_Data *block_data,int M,int m,int length,char type,int size,void *value){
	int i;
	int min = -1;
	for(i=0;i<M;i++){
		int pos = block_data[i].counter;
		//If block has finished
		if(pos == -1){
			continue;
		}
		//Otherwise check the min with the current block
		else{
			if((min == -1) || (!compare(&(block_data[i].data[sizeof(int)+size*pos+m]),
				value,type,length))){
				min = i;
				memcpy(value,&(block_data[i].data[sizeof(int)+size*pos+m]),length);
			}
		}
	}
	return min;
}


SR_ErrorCode MergeSort(int num_block,int in_fileDesc,int out_fileDesc,int fieldNo,int bufferSize){
	BF_Block *out_block;
	BF_Block *temp_block;
	BF_Block_Init(&temp_block);
	BF_Block_Init(&out_block);

	int temp_fileDesc;
	char *out_data;
	char *temp_data;

	//Create output file and update the first block with metadata
	int start;
	BF_GetBlockCounter(out_fileDesc,&start);
	BF_AllocateBlock(out_fileDesc,out_block);
	BF_UnpinBlock(out_block);

	//Create temp file for sorting
	SR_CreateFile("temp");
	SR_OpenFile("temp",&temp_fileDesc);
	BF_AllocateBlock(temp_fileDesc,temp_block);
	BF_UnpinBlock(out_block);
	BF_UnpinBlock(temp_block);
	


	int counter;
	int N;
	int M = bufferSize;
	//Get the number of blocks of input_file
	if(BF_GetBlockCounter(in_fileDesc,&N) != BF_OK){
		BF_Block_Destroy(&temp_block); 
		BF_Block_Destroy(&out_block); 
	    return SR_ERROR;
	}
	N--; 	//Except the first metadata block

	//Create the Block_Data structure
	Block_Data * block_data;
	block_data = (Block_Data *)malloc(sizeof(Block_Data)*(M-1));
	int i;
	for(i=0;i<M-1;i++){
		BF_Block_Init(&(block_data[i].block));
	}


	int j;
	int size = sizeof(Record);
	int m;
	int length;
	char type;
	void *value;

	//Read the field that we are going to sort the file and initialize the vars
	switch(fieldNo){
		case 0:{
			m = 0;
			length = sizeof(int);
			value = (int *)malloc(sizeof(int));
			type = 'i';
			break;
		}
		case 1:{
			m = sizeof(int);
			length = sizeof(char)*15;
			value = (char *)malloc(sizeof(char)*length);
			type = 'c';
			break;
		}
		case 2:{
			m = sizeof(int)+sizeof(char)*15;
			length = sizeof(char)*20;
			value = (char *)malloc(sizeof(char)*length);
			type = 'c';
			break;
		}
		case 3:{
			m = sizeof(int)+sizeof(char)*(15+20);
			length = sizeof(char)*20;
			value = (char *)malloc(sizeof(char)*length);
			type = 'c';
			break;
		}
	}


	int out_file_flag = 0;
	int fd;
	char * write_data;
	BF_Block * write_block;
	int write_counter;
	int write_fd;
	int max_blocks;
	int loop = 0;


	int max_loop = intlog(M-1,N);	//Get the number of loops where we have to do
									//for sorting the entire file

	BF_Block_Init(&write_block);
	int k = (int)ceil(N/(double)(M-1)); 	//Number of sub-joints
	int current_block;
	

	if(max_loop % 2 == 0){
		out_file_flag = 0;			//Start with temp_file to write
		write_fd = temp_fileDesc;
	}
	else{
		out_file_flag = 1;			//Start with the out_file to write
		write_fd = out_fileDesc;
	}

	while(loop < max_loop){
		if(loop == 0){
			fd = in_fileDesc;						//IN
			max_blocks = 1;
		}
		else{
			max_blocks *= (M-1);
			if(out_file_flag){
				fd = temp_fileDesc;					//IN
				write_fd = out_fileDesc;			//OUT
			}
			else{
				fd = out_fileDesc; 					//IN
				write_fd = temp_fileDesc;			//OUT
			}
			int next_pointer = -1;
    		memcpy(&(write_data[BF_BLOCK_SIZE-sizeof(int)]),&next_pointer,sizeof(int));
    		BF_Block_SetDirty(write_block);
			BF_UnpinBlock(write_block);
		}

		out_file_flag = (out_file_flag +1) % 2;
		BF_GetBlock(write_fd,1,write_block);
		write_data = BF_Block_GetData(write_block);
		write_counter = 0;
		current_block = 1;
		num_block = 1;
		//For each group of blocks
		for(j=0;j<k;j++){
			//Initialize the Block Data and Sort the block if stage 0
			i = Initialize_BlockData(block_data,fd,M,num_block,max_blocks,N,loop,m,length,value,type);
			
			num_block += max_blocks*(M-1);
			int min = -1;
			//Until merge_sort all the groups of block
			while(1){
				//Get the min value from blocks
				min = Find_min(block_data,i, m, length, type,size,value);
				if(min == -1){
					break;
				}

				if (write_counter >= (BF_BLOCK_SIZE - 2*sizeof(int))/ sizeof(Record)){
					if(loop < 2){
						int next_pointer;
						BF_GetBlockCounter(write_fd,&next_pointer);
						memcpy(&(write_data[BF_BLOCK_SIZE-sizeof(int)]),&next_pointer,sizeof(int));
						BF_Block_SetDirty(write_block);
						BF_UnpinBlock(write_block);
						BF_AllocateBlock(write_fd,write_block);
						current_block = next_pointer;
					}
					else{
						current_block++;
						BF_Block_SetDirty(write_block);
						BF_UnpinBlock(write_block);
						BF_GetBlock(write_fd,current_block,write_block);
					}
					
    				write_data = BF_Block_GetData(write_block);
    				write_counter = 0;
				}

				//WRITE DATA
				int pos = block_data[min].counter;
				Record record;
				memcpy(&(write_data[sizeof(int)+size*write_counter]),
					&(block_data[min].data[sizeof(int)+size*pos]),sizeof(int));

			    memcpy(&(write_data[2*sizeof(int)+size*write_counter]),
			    	&(block_data[min].data[2*sizeof(int)+size*pos]),sizeof(record.name));

			    memcpy(&(write_data[2*sizeof(int)+sizeof(record.name)+size*write_counter]),
			    	&(block_data[min].data[2*sizeof(int)+sizeof(record.name)+size*pos]),sizeof(record.surname));
			    
			    memcpy(&(write_data[2*sizeof(int)+sizeof(record.name)+sizeof(record.surname)+size*write_counter]),
			    	&(block_data[min].data[2*sizeof(int)+sizeof(record.name)+sizeof(record.surname)+size*pos]),sizeof(record.city));
			
				write_counter++;
				memcpy(write_data,&write_counter,sizeof(int));

				//UPDATE READ BLOCK INFO
				block_data[min].counter++;
				memcpy(&counter,block_data[min].data,sizeof(int));
				if(counter <= block_data[min].counter){
					int next = -1;
					memcpy(&next,&(block_data[min].data[BF_BLOCK_SIZE-sizeof(int)]),sizeof(int));
					if(block_data[min].num_of_blocks < max_blocks && next != -1){
						block_data[min].counter = 0;
						BF_UnpinBlock(block_data[min].block);
						BF_GetBlock(fd,next,block_data[min].block);
						block_data[min].data = BF_Block_GetData(block_data[min].block);
						block_data[min].num_of_blocks++;
					}
					else{
						block_data[min].counter = -1;
					}
				}
			}
			int p;
			for(p=0;p<i;p++){
				BF_UnpinBlock(block_data[p].block);
			}
		}
		k = (int)ceil(k/(double)(M-1));
		loop++;
	}


	free(value);
	for(i=0;i<M-1;i++){
		BF_Block_Destroy(&(block_data[i].block));
	}
	int null_pointer = -1;
	memcpy(&(write_data[BF_BLOCK_SIZE-sizeof(int)]),&null_pointer,sizeof(int));
	BF_Block_SetDirty(write_block);
	BF_UnpinBlock(write_block);
	BF_CloseFile(temp_fileDesc);
	unlink("temp");
	free(block_data);
	BF_Block_Destroy(&out_block);
	BF_Block_Destroy(&temp_block);
	BF_Block_Destroy(&write_block);
}
