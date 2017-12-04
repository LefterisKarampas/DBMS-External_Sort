#include <string.h>

int compare(void * value1,void *value2,char attrType,int attrLength){
  if(attrType == 'c'){                                                    //If type is char *
    char x1[attrLength];
    char x2[attrLength];
    memcpy(x1,(char *)value1,attrLength);                                //Copy the string in temp_var
    x1[attrLength-1] = '\0';
    memcpy(x2,(char *)value2,attrLength);                                //Copy the string in temp_var
    x2[attrLength-1] = '\0';
    return (strcmp(x1,x2) <= 0? 0:1);    //Return 0 if x1 < x2 else return 1
  }
  else{
    if(attrType == 'f'){                                                 //If type is float
      if((*(float *)value1) <= *((float *)value2)){
        return 0;                                                         //Return 0 if x1 < x2
      }
    }
    else if(*((int *)value1) <= *((int *)value2)){                         //If type is int
      return 0;                                                           //Return 0 if x1 <X2
    }
  }
  return 1;                                                               //Return 1 if x1>=x2
}