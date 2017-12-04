#include "Sort.h"
#include "bf.h"
#include "sort_file.h"
#include <string.h>
#include <stdlib.h>
#include "Compare.h"
#include <stdio.h>


void swap(char *data,int size,int i,int j){
	Record record;
	memcpy(&(record.id),&(data[sizeof(int)+size*i]),sizeof(int));
	memcpy(&(record.name),&(data[2*sizeof(int)+size*i]),sizeof(record.name));
	memcpy(&(record.surname),&(data[2*sizeof(int)+size*i+sizeof(record.name)]),sizeof(record.surname));
	memcpy(&(record.city),&(data[2*sizeof(int)+size*i+sizeof(record.name)+sizeof(record.surname)]),sizeof(record.city));

	memcpy(&(data[sizeof(int)+size*i]),&(data[sizeof(int)+size*j]),sizeof(int));
	memcpy(&(data[2*sizeof(int)+size*i]),&(data[2*sizeof(int)+size*j]),sizeof(record.name));
	memcpy(&(data[2*sizeof(int)+size*i+sizeof(record.name)]),&(data[2*sizeof(int)+size*j+sizeof(record.name)]),sizeof(record.surname));
	memcpy(&(data[2*sizeof(int)+size*i+sizeof(record.name)+sizeof(record.surname)]),&(data[2*sizeof(int)+size*j+sizeof(record.name)+sizeof(record.surname)]),sizeof(record.city));

	memcpy(&(data[sizeof(int)+size*j]),&(record.id),sizeof(int));
	memcpy(&(data[2*sizeof(int)+size*j]),&(record.name),sizeof(record.name));
	memcpy(&(data[2*sizeof(int)+size*j+sizeof(record.name)]),&(record.surname),sizeof(record.surname));
	memcpy(&(data[2*sizeof(int)+size*j+sizeof(record.name)+sizeof(record.surname)]),&(record.city),sizeof(record.city));
}

int partition(char *data,int size,int wall,int pivot,int m,char type,int length,void *pivot_value){
	memcpy(pivot_value,&data[sizeof(int)+size*pivot+m],length);
	for(int i=wall;i<pivot;i++){
		if(!compare(&(data[sizeof(int)+size*i+m]),&(data[sizeof(int)+size*pivot+m]),type,length)){
			if(i>wall){
				swap(data,size,i,wall);
			}
			wall++;
		}
	}
	swap(data,size,wall,pivot);
	return wall;
}


void QuickSort(char *data,int size,int wall,int pivot,int m,char type,int length,void *pivot_value){
	if(wall >= pivot){
		return;
	}
	int r = partition(data,size,wall,pivot,m,type,length,pivot_value);
	QuickSort(data,size,wall,r-1,m,type,length,pivot_value);
	QuickSort(data,size,r+1,pivot,m,type,length,pivot_value);
}


int SortBlock(int num_block,int fileDesc,int fieldNo){
	if(num_block == -1){
		return -1;
	}
	BF_Block *block;
	BF_Block_Init(&block);
	char *data;
	int counter;
	SR_ErrorCode error;
	if((error=BF_GetBlock(fileDesc,num_block, block)) != BF_OK){                     //Get the first block
		BF_Block_Destroy(&block); 
	    BF_PrintError(error);                                                         //If fails
	    return SR_ERROR;                                                              //Return error
	}
	data = BF_Block_GetData(block);
	memcpy(&counter,data,sizeof(int));
	int pivot = counter -1;
	int wall = 0;
	int size = sizeof(Record);
	int m;
	int length;
	void * pivot_value;
	char type;
	switch(fieldNo){
		case 0:{
			m = 0;
			length = sizeof(int);
			pivot_value = (int *)malloc(sizeof(int));
			type = 'i';
			break;
		}
		case 1:{
			m = sizeof(int);
			length = sizeof(char)*15;
			pivot_value = (char *)malloc(sizeof(char)*length);
			type = 'c';
			break;
		}
		case 2:{
			m = sizeof(int)+sizeof(char)*15;
			length = sizeof(char)*20;
			pivot_value = (char *)malloc(sizeof(char)*length);
			type = 'c';
			break;
		}
		case 3:{
			m = sizeof(int)+sizeof(char)*(15+20);
			length = sizeof(char)*20;
			pivot_value = (char *)malloc(sizeof(char)*length);
			type = 'c';
			break;
		}
	}
	QuickSort(data,size,wall,pivot,m,type,length,pivot_value);
	free(pivot_value);
	int next_pointer;
	memcpy(&next_pointer,&(data[BF_BLOCK_SIZE-sizeof(int)]),sizeof(int));
	BF_Block_SetDirty(block);
	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);
	return next_pointer;
}