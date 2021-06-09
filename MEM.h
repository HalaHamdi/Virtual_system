#include <stdio.h>
#define MAX_SIZE_MEM 1024
// struct PCB *Object,value;

struct Free
{

    int from;
    int to;
    int space;

};
struct Freeblocks
{
   struct Free Mem[MAX_SIZE_MEM];
   int count;
};

void printOneSpace(struct Free block){
    printf("from %d To %d a space of %d\n",block.from,block.to,block.space);
}

void push(struct Free F,struct Freeblocks *Object){
    Object->Mem[Object->count++]=F;
}

void printFreeSpace(struct Freeblocks *Object){
    for(int i=0;i<Object->count;i++){
        printOneSpace(Object->Mem[i]);        
    }
}



//Return the memory back to the free memory DS
//place it in the correct sorted address postion
void insertStart(struct Free F,struct Freeblocks *Object){
    int i=-1;
    for(i=Object->count-1;i>=0;i--){
        if(Object->Mem[i].from>F.from){
           Object->Mem[i+1]=Object->Mem[i];
        }else{
            Object->Mem[i+1]=F;
            break;
        }
    }
    if(i==-1){
      Object->Mem[i+1]=F;
    }
    //Object->Mem[i]=F;
    Object->count++;

}
struct Free GitFristFit(int space,struct Freeblocks *Object){
    struct Free fristblock;
    fristblock.space=0;
    for(int i=0;i<Object->count;i++){
        if(Object->Mem[i].space>=space){
            fristblock=Object->Mem[i];
            for(int j=i;j<Object->count-1;j++){
                Object->Mem[j]=Object->Mem[j+1];
            }
            Object->count--;
        }
    }
    return fristblock;
}
int GetNextfit(int space, struct Freeblocks *Object,int nextfit)
{
    int position=nextfit;
  for(int i=nextfit;i<Object->count;i++)
    {
           if(Object->Mem[i].space>=space)
           {
             
               return i;
           }  
    }
   
      for(int i=0;i<position;i++)
      {
          if(Object->Mem[i].space>=space)
          return i;
      }
    
    return -1;

}
//remove from the frist
void RemoveMEM(struct Freeblocks *Object)
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
//Works for a free list map that's sorted by addresses
void Marge(struct Freeblocks *Object){
    for(int i=0;i<Object->count-1;i++){
        if(Object->Mem[i].to==Object->Mem[i+1].from){
            Object->Mem[i].to=Object->Mem[i+1].to;
            Object->Mem[i].space+=Object->Mem[i+1].space;
            for(int j=i+1;j<Object->count-1;j++){
                Object->Mem[j]= Object->Mem[j+1];
            }
            Object->count--;
            //to check again for that free space after merging, whether there's another merge next or not
            i--;
        }
    }

}


//Return the memory back to the free memory DS
//place it in the correct sorted size postion
void insertSpace(struct Free F,struct Freeblocks *Object){
    int i=0;
    printf("Object->count-1 = %d\n",Object->count-1);
    for(i=Object->count-1;i>=0;i--){
        if(Object->Mem[i].space>F.space){
           Object->Mem[i+1]=Object->Mem[i];
        }else{
            Object->Mem[i+1]=F;
            break;
        }
    }
    if(i==-1){
      i++;
      Object->Mem[i]=F;
    }
    Object->count++;
}

struct Free GetBestFit(int space,struct Freeblocks *Object){
    struct Free bestblock;
    bestblock.space=0;
    int indexOfBestBlock = -1;
    for(int i=0;i<Object->count;i++){
        if(Object->Mem[i].space>=space && (bestblock.space == 0 || Object->Mem[i].space < bestblock.space )){
            bestblock=Object->Mem[i];
            indexOfBestBlock = i;
        }
    }
    
    //If there was a block available
    //then remove it from the free list and return it to the process
    if(indexOfBestBlock != -1){
        for(int j=indexOfBestBlock;j<Object->count-1;j++){
                Object->Mem[j]=Object->Mem[j+1];
            }
            Object->count--;
    }

    return bestblock;
}
