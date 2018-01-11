#include "sort_file.h"
#include "Print.h"
#include "bf.h"
#include <string.h>
#include <stdio.h>
#include "Sort.h"

SR_ErrorCode SR_Init() {
  BF_Init(LRU); 
  return SR_OK;
}


SR_ErrorCode SR_CreateFile(const char *fileName) {
  SR_ErrorCode error;
  int fd;
  BF_Block *block;
  BF_Block_Init(&block);                                                          //Initialize Block
  if((error = BF_CreateFile(fileName)) != BF_OK)
  {                                 
    BF_Block_Destroy(&block);
    //BF_PrintError(error);                                                         //If creation fails
    return SR_ERROR;                                                              //Return error
  }
  if ((error = BF_OpenFile(fileName, &fd)) != BF_OK)
  {
    BF_Block_Destroy(&block);
    //BF_PrintError(error);                                                         
    return SR_ERROR;
  }
  if((error = BF_AllocateBlock(fd, block)) != BF_OK)                              //Allocate a new block
  {                             
    BF_CloseFile(fd);
    BF_Block_Destroy(&block);
    //BF_PrintError(error);                                                         //If fails 
    return SR_ERROR;                                                              //Return error
  } 
  int m=0;
  char *data;
  data = BF_Block_GetData(block);
  memcpy(data, "SR", strlen("SR")+1);                                             //Initialize the new file
  BF_Block_SetDirty(block);                                                       //as SR
  BF_UnpinBlock(block);                                                           
  BF_CloseFile(fd);                                                               //Close file
  BF_Block_Destroy(&block);  
  return SR_OK;
}



SR_ErrorCode SR_OpenFile(const char *fileName, int *fileDesc) {
  BF_Block *block;
  SR_ErrorCode error;
  BF_Block_Init(&block);                                                  //Initialize Block
  if((error = BF_OpenFile(fileName, fileDesc)) != BF_OK){                 //Open the file
    BF_Block_Destroy(&block);
    BF_PrintError(error);                                                 //If fails 
    return SR_ERROR;                                                      //Return error
  }
  int i = 0;

  int block_num;
  BF_GetBlockCounter(*fileDesc, &block_num);                               //Check if exists the first block
  if(block_num <= 0){
    BF_Block_Destroy(&block); 
    BF_PrintError(error);                                                 //If fails
    return SR_ERROR;
  }

  if((error =BF_GetBlock(*fileDesc, i, block)) != BF_OK){                 //Get the first block
    BF_Block_Destroy(&block); 
    BF_PrintError(error);                                                 //If fails
    return SR_ERROR;                                                      //Return error
  }

  char *data;
  data = BF_Block_GetData(block);                                         //Get the block's data
  if(strcmp(data,"SR")){                                                  //Check if the file is sort_file 
    BF_UnpinBlock(block);                                                 //If it isn't sort_file
    BF_Block_Destroy(&block);                                                      
    return SR_ERROR;                                                      //Return error
  }

  BF_UnpinBlock(block);                                                   //Unpin the block
  BF_Block_Destroy(&block);                                               //Deallocate the block's structure
  return SR_OK;                                                           //else return OK
}



SR_ErrorCode SR_CloseFile(int fileDesc) {
  SR_ErrorCode error;
  if((error = BF_CloseFile(fileDesc)) != BF_OK){                                    //Close file
    BF_PrintError(error);
    return SR_ERROR;
  }
  return SR_OK;
}




SR_ErrorCode SR_InsertEntry(int fileDesc, Record record) {
  BF_Block *block;
  BF_Block_Init(&block);
  char *data;
  int block_num;
  int next_block = -1;
  BF_GetBlockCounter(fileDesc, &block_num);
  BF_GetBlock(fileDesc, block_num-1, block);
  data = BF_Block_GetData(block);
  int k = 0;
  int size = sizeof(Record);
  if ((block_num -1) == 0)
  {
    BF_AllocateBlock(fileDesc, block);
    data = BF_Block_GetData(block);
    int m=sizeof(int);
    int counter = 1;
    memcpy(data,&counter,sizeof(int));                        //counter
    memcpy(&(data[BF_BLOCK_SIZE-sizeof(int)]), &next_block, sizeof(int)); //set next pointer to default
  }

  else
  {
    int counter;
    int m = sizeof(int);
    memcpy(&counter, data, sizeof(int));
    if (counter < (BF_BLOCK_SIZE - 2*sizeof(int))/ sizeof(Record))        //free space in current block
    {
      k = counter;
      counter++;
      memcpy(data, &counter, sizeof(int));
    }

    else                                    //full block, allocate new
    {
      memcpy(&(data[BF_BLOCK_SIZE-sizeof(int)]), &block_num, sizeof(int));  //update next pointer 
      BF_Block_SetDirty(block);
      BF_UnpinBlock(block);
      BF_AllocateBlock(fileDesc, block);
      data = BF_Block_GetData(block);
      counter = 1;
      memcpy(data,&counter,sizeof(int));
      memcpy(&(data[BF_BLOCK_SIZE-sizeof(int)]), &next_block, sizeof(int)); //set next pointer to default
    }
  }

  memcpy(&(data[sizeof(int)+size*k]),&(record.id),sizeof(int));
  memcpy(&(data[2*sizeof(int)+size*k]),&(record.name),sizeof(record.name));
  memcpy(&(data[2*sizeof(int)+size*k+sizeof(record.name)]),&(record.surname),sizeof(record.surname));
  memcpy(&(data[2*sizeof(int)+size*k+sizeof(record.name)+sizeof(record.surname)]),&(record.city),sizeof(record.city));
  
  BF_Block_SetDirty(block);
  BF_UnpinBlock(block);
  BF_Block_Destroy(&block);
  return SR_OK;
}




SR_ErrorCode SR_SortedFile(
  const char* input_filename,
  const char* output_filename,
  int fieldNo,
  int bufferSize
) {
    //If buffersize is greater than max blocks in memory or less than 3 blocks, exit!  
    if(bufferSize > BF_BUFFER_SIZE || bufferSize < 3){ 
      return SR_ERROR;
    }

    int fileDesc;
    int out_fileDesc;
    SR_OpenFile(input_filename, &fileDesc);                             //Open input_file
    SR_CreateFile(output_filename);                                     //Create output_file
    SR_OpenFile(output_filename,&out_fileDesc);                         //Open output_file

    BF_Block *block;
    SR_ErrorCode error;
    BF_Block_Init(&block);

    int block_num;
    BF_GetBlockCounter(fileDesc, &block_num);   //Check if data exists in input_file                 
    if(block_num <= 0){
      BF_Block_Destroy(&block);
      BF_CloseFile(fileDesc);
      BF_CloseFile(out_fileDesc);
      return SR_ERROR;
    }
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);

    //Start Sorting 
    MergeSort(1,fileDesc,out_fileDesc,fieldNo,bufferSize);

    BF_CloseFile(fileDesc);
    BF_CloseFile(out_fileDesc);
    return SR_OK;
}



SR_ErrorCode SR_PrintAllEntries(int fileDesc) {
  BF_Block *block;
  SR_ErrorCode error;
  BF_Block_Init(&block);

  int block_num;
  BF_GetBlockCounter(fileDesc, &block_num);   //Check if file is not empty                 
  if(block_num <= 0){
    BF_Block_Destroy(&block);
    BF_CloseFile(fileDesc);
    return SR_ERROR;
  }

  int temp;
  block_num = 1;
  //Start Printing
  while((temp = PrintBlock(fileDesc,block_num)) != -1){
    block_num = temp;
  }

  BF_Block_Destroy(&block);
  return SR_OK;
}
