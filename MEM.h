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
Free GitFristFit(int space,struct Freeblocks *Object){
    Free fristblock;
    fristblock.space=0;
    for(int i=0;i<Object->count;i++){
        if(Object->Mem[i].space>=space){
            fristblock=Object->Mem[i];
            for(int j=i+1;i<Object->count-1;j++){
                Object->Mem[j]=Object->Mem[j+1];
            }
            Object->count--;
        }
    }
    return fristblock;
}
//remove from the frist
void Remove(struct Freeblocks *Object)
{
    for(int i=0;i<Object->count;i++)
    {
        Object->Mem[i]=Object->Mem[i+1];
    }
    Object->Mem[Object->count-1].from=-1;
    Object->Mem[Object->count-1].to=-1;
    Object->Mem[Object->count-1].space=-1;
    Object->count--;

}
//for inserted Start
void Marge(struct Freeblocks *Object){
    for(int i=0;i<Object->count-1;i++){
        if(Object->Mem[i].to==Object->Mem[i+1].from){
            Object->Mem[i].to=Object->Mem[i+1].from;
            Object->Mem[i].space+=Object->Mem[i+1].space;
            for(int j=i+1;j<Object->count-1;j++){
                Object->Mem[j]= Object->Mem[j+1];
            }
            Object->count--;
        }
    }

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