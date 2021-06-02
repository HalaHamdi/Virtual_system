#include "headers.h"
#include"DS.h"
struct processData
{
    int arrivaltime;
    int priority; // qutaum is float
    int runningtime;
    int id;
};

int shmid,TotalProcess,AlgorithmNumber,msgq_id,counter=0;
float quantum;


int main(int argc, char *argv[])
{
   initClk();
    key_t schedulerKey=1234;
     msgq_id=msgget(schedulerKey,0666|IPC_CREAT);
  
    if(msgq_id==-1)
    {
        printf("error in creating msgQueue \n"); 
        exit(-1);
    }
          printf("after msgqueue scheduler \n");
    struct processData p;
    msgrcv(msgq_id,&p,sizeof(&p),0,!IPC_NOWAIT);
    quantum=-1;
    TotalProcess=p.arrivaltime;
    AlgorithmNumber=p.id;
    quantum=p.priority;
    struct processData  processArray[TotalProcess];

    
    
    
   
    for(int i =0;i<TotalProcess;i++)
     printf(" id %d , arrival; %d , runtime %d , priority; %d \n ",processArray[i].id,processArray[i].arrivaltime,processArray[i].runningtime,processArray[i].priority);
    // CreatePCB();
    // attach_PCB();


    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}
void CreatePCB()
{   key_t key; 
    key=ftok("PCB",'P');
   int shmid =  shmget(key, 4096, IPC_CREAT | 0644);
   if(shmid==-1)
   {
       printf("Error ");
       exit(-1);
   
   }
   

}
void attach_PCB()
{
     void *shmaddr = shmat(shmid, (void *)0, 0);
    

    // not sure
             Object = (struct PCB*) shmaddr;
             Object->count=0;
             shmaddr=Object;
   
    
     
}
void GetData(struct processData  processArray[])
{
     struct processData p;
     p.arrivaltime=-1;
     p.id=-1;
     p.priority=-1;
     p.runningtime=-1;
  
     if(TotalProcess>0 )
     {  msgrcv(msgq_id,&p,sizeof(&p),0,IPC_NOWAIT);
     if(p.arrivaltime!=-1 && p.id!=-1 && p.priority!=-1 && p.runningtime!=-1)
     {processArray[counter]=p;
        counter++;
        TotalProcess--;
     printf(" in recieving id %d , arrival; %d , runtime %d , priority; %d \n ",processArray[counter].id,processArray[counter].arrivaltime,processArray[counter].runningtime,processArray[counter].priority);
     }

    }
}
  

