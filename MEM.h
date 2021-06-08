#include <stdio.h>
#define MAX_SIZE 1024
// struct PCB *Object,value;

struct Free
{

    int from;
    int to;
    int space;

};
struct Freeblocks
{
   struct Free Mem[MAX_SIZE];
   int count;
};

void push(struct Free F,struct Freeblocks *Object){
    Object->Mem[Object->count++]=F;
}

void insertStart(struct Free F,struct Freeblocks *Object){
    int i=0;
    for(i=Object->count-1;i>=0;i--){
        if(Object->Mem[i].from>F.from){
           Object->Mem[i+1]=Object->Mem[i];
        }else{
            Object->Mem[i+1]=F;
            break;
        }
    }
    if(i=0){
      Object->Mem[i]=F;
    }
    //Object->Mem[i]=F;
    Object->count++;

}

void insertSpace(struct Free F,struct Freeblocks *Object){
    int i=0;
    for(i=Object->count-1;i>=0;i--){
        if(Object->Mem[i].space>F.space){
           Object->Mem[i+1]=Object->Mem[i];
        }else{
            Object->Mem[i+1]=F;
            break;
        }
    }
    if(i=0){
      Object->Mem[i]=F;
    }
}