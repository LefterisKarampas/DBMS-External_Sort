#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "QuickSort.h"
#include "Compare.h"
#include "sort_file.h"

void QuickSort(char *data,int size,int wall,int pivot,int m,char type,int length,void *pivot_value){
	if(wall >= pivot){
		return;
	}
	int r = partition(data,size,wall,pivot,m,type,length,pivot_value);
	QuickSort(data,size,wall,r-1,m,type,length,pivot_value);
	QuickSort(data,size,r+1,pivot,m,type,length,pivot_value);
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