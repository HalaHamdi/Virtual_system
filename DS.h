#include <stdio.h>
#define MAX_SIZE 5000
// struct PCB *Object,value;

struct ProcessPCB
{

    int arrivaltime;
    int priority;
    int runningtime;
    int remanningtime;
    int wait;
    int id;
    int pid;
    char state[10];
    int memsize;
    bool inmemory

};
struct PCB
{
   struct ProcessPCB Procsess[MAX_SIZE];
   int count;
   struct ProcessPCB Run;
   

};

// where should i decaler it???

 // Object.count=0;
void Push(struct ProcessPCB p,struct PCB *Object)
{
   //printf("ooh");
   
   Object->Procsess[Object->count++]=p;
    //printf("done");
}
void POP(struct PCB *Object)
{
    Object->Procsess[Object->count].arrivaltime=-1;
    Object->Procsess[Object->count].priority=-1;
    Object->Procsess[Object->count].runningtime=-1;
    Object->Procsess[Object->count].remanningtime=-1;
    Object->Procsess[Object->count].id=-1;
    Object->Procsess[Object->count].pid=-1;
    // Object.Procsess[Object.count].state[0]='      ';
    Object->count--;

}
void Remove(struct PCB *Object)
{
    for(int i=0;i<Object->count;i++)
    {
        Object->Procsess[i]=Object->Procsess[i+1];
    }
    Object->Procsess[Object->count-1].arrivaltime=-1;
    Object->Procsess[Object->count-1].priority=-1;
    Object->Procsess[Object->count-1].runningtime=-1;
    Object->Procsess[Object->count-1].remanningtime=-1;
    Object->Procsess[Object->count-1].id=-1;
    Object->Procsess[Object->count-1].pid=-1;
    Object->count--;

}
void Removeone(int pid,struct PCB *Object)
{
    int index=-1;
    for(int i=0;i<Object->count;i++)
    {
        if(Object->Procsess[i].pid==pid)
        {
            index=i;
              break;
        }
    }
    if(index!=-1)
    { 
        for(int i=index;i<Object->count;i++)
        {
            Object->Procsess[i]=Object->Procsess[i+1];
        }
        POP(Object);
    }

}

void shiftStartingFrom(int start, struct PCB *theTable){
    for(int j = theTable->count-1; j >= start; j--){
        theTable->Procsess[j+1] = theTable->Procsess[j]; 
    }
    theTable->count++;
}

//Assuming the PCBTable is already sorted
//Insert in it this new Process entry in its correct sorted relative position
void InsertSortedByRemainTime(struct ProcessPCB entry,struct PCB *theTable){
    
    for(int i=0; i < (theTable->count); i++){
        if(entry.remanningtime < theTable->Procsess[i].remanningtime){
            shiftStartingFrom(i, theTable);
            theTable->Procsess[i] = entry;
            return;
        }
    }
    //If all the processes has remainingTime less than the current inserted entry
    //Then place it at the last position
    theTable->Procsess[theTable->count] = entry;
    theTable->count=theTable->count+1;
}

void Insert(struct ProcessPCB p,struct PCB *Object)
{
    Object->count ++;
    struct PCB *newObject=Object;
     for(int i=0;i<Object->count;i++ )
     {
         Object->Procsess[i+1]=newObject->Procsess[i];
     }
     Object->Procsess[0]=p;
}
void Clear(struct PCB *Object)
{

for(int i=0;i<Object->count;i++)
    {
        Object->Procsess[i].arrivaltime=-1;
    Object->Procsess[i].priority=-1;
    Object->Procsess[i].runningtime=-1;
    Object->Procsess[i].remanningtime=-1;
    Object->Procsess[i].id=-1;
    Object->Procsess[i].pid=-1;

    }
     Object->count=0;

}
int getProcess(int pid,struct PCB *Object)
{
     for(int i=0;i<Object->count;i++)
     {
         if(Object->Procsess[i].pid==pid)
         {
             return i;

         }
     }
     return -1;
}
void Swap(int i,int j,struct PCB *Object)
{
   struct ProcessPCB P=Object->Procsess[i];
   Object->Procsess[i]=Object->Procsess[j];
   Object->Procsess[j]=P;

}
void sortrunnigtime(struct PCB *Object)
{
    
   
    for(int i=0;i<Object->count;i++)
    {
        int index=i;
          for(int j=i+1;j<Object->count;j++)
          {
                   if(Object->Procsess[index].runningtime>Object->Procsess[j].runningtime)
                   {
                       index=j;
                   }
          }
          Swap(index,i,Object);
    }
    
    
}
void sortpriority(struct PCB *Object)
{
    
   
    for(int i=0;i<Object->count;i++)
    {
        int index=i;
          for(int j=i+1;j<Object->count;j++)
          {
                   if(Object->Procsess[index].priority>Object->Procsess[j].priority)
                   {
                       index=j;
                   }
          }
          Swap(index,i,Object);
    }
    
    
}
void PrintProcess(struct ProcessPCB p)
{
    printf("pid: %d \n",p.pid);
    printf("id: %d \n",p.id);
    printf("arrival time: %d \n",p.arrivaltime);
    printf("running time: %d \n",p.runningtime);
    printf("remaining time: %d \n",p.remanningtime);
    printf("state: %s \n\n",p.state);
}
void PrintPCB(struct PCB *Object)
{
    for(int i=0;i<Object->count;i++)
    {
        printf("ahhhh %d %d",Object->Procsess[i].arrivaltime,Object->Procsess[i].runningtime);
    }
}

