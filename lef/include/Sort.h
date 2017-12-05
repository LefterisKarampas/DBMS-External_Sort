#ifndef _SORT_H_
#define _SORT_H_
#include "bf.h"
#include "sort_file.h"

void swap(char *,int,int,int);
int partition(char *,int ,int ,int ,int ,char ,int ,void *);
void QuickSort(char *,int ,int ,int ,int ,char ,int ,void *);
int SortBlock(int ,BF_Block *,char * ,int );
SR_ErrorCode MergeSort(int ,int ,int ,int ,int );

#endif