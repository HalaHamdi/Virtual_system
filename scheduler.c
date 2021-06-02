#include "headers.h"

struct processData
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
};
int main(int argc, char *argv[])
{
    initClk();
    key_t schedulerKey=1234;
    int msgq_id=msgget(schedulerKey,0666|IPC_CREAT);
      printf("between msgqueue   scheduler \n");

   if(msgq_id==-1){printf("error in creating msgQueue \n"); exit(-1);}
          printf("after msgqueue scheduler \n");
    struct processData p;
    msgrcv(msgq_id,&p,sizeof(&p),0,!IPC_NOWAIT);
    int TotalProcess=p.arrivaltime;
    struct processData  processArray[TotalProcess];
    
    for(int counter=0 ;counter<TotalProcess;counter++ ){
     msgrcv(msgq_id,&p,sizeof(&p),0,!IPC_NOWAIT);
     processArray[counter]=p;
     printf(" in recieving id %d , arrival; %d , runtime %d , priority; %d \n ",processArray[counter].id,processArray[counter].arrivaltime,processArray[counter].runningtime,processArray[counter].priority);

    }
   
    for(int i =0;i<TotalProcess;i++)
     printf(" id %d , arrival; %d , runtime %d , priority; %d \n ",processArray[i].id,processArray[i].arrivaltime,processArray[i].runningtime,processArray[i].priority);
    

    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}
