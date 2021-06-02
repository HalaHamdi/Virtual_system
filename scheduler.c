#include "headers.h"
 #include"DS.h"

struct processData
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
};
int shmid;
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



// #include "headers.h"
// #include"DS.h"
// struct processData
// {
//     int arrivaltime;
//     int priority; // qutaum is float
//     int runningtime;
//     int id;
// };

// int shmid,TotalProcess,AlgorithmNumber,msgq_id,counter=0;
// float quantum;

// void GetData(struct processData  processArray[]);
// int main(int argc, char *argv[])
// {
// //    initClk();
//     key_t schedulerKey=1234;
//      msgq_id=msgget(schedulerKey,0666|IPC_CREAT);
  
//     if(msgq_id==-1)
//     {
//         printf("error in creating msgQueue \n"); 
//         exit(-1);
//     }
//           printf("after msgqueue scheduler \n");
   
//      CreatePCB();
//     printf("howwwwwwwwwwwww \n");
//      attach_PCB();
//      printf("Cretead \n");
//       struct processData p;
//     printf(" i will recevie\n");
// //     msgrcv(msgq_id,&p,sizeof(&p),0,!IPC_NOWAIT);
// //     printf("%d %d  %d %d\n",p.arrivaltime,p.id,p.priority,p.runningtime);
// //     quantum=-1;
// //     TotalProcess=p.arrivaltime;
// //     AlgorithmNumber=p.id;
// //     quantum=p.priority;
// //     struct processData  processArray[TotalProcess];
    
   
// //     while(counter<TotalProcess)
// //   {  
// //      GetData(processArray);
    
// //   } 
    
   
//     // for(int i =0;i<TotalProcess;i++)
//     //  printf(" id %d , arrival; %d , runtime %d , priority; %d \n ",processArray[i].id,processArray[i].arrivaltime,processArray[i].runningtime,processArray[i].priority);
    

//     //TODO: implement the scheduler.
//     //TODO: upon termination release the clock resources.

//     destroyClk(true);
// }
void CreatePCB()
{   key_t key; 
    key=ftok("PCB",'P');
    shmid =  shmget(key, 4096, IPC_CREAT | 0644);
   if(shmid==-1)
   {
       printf("Error \n");
       exit(-1);
   
   }
   else
   printf("Cretead \n");
   

 }
void attach_PCB()
{
    printf("%d \n",shmid);
     void *shmaddr = shmat(shmid, (void *)0, 0);
     printf("non problem \n");
    

    struct PCB value;
    struct PCB  *Object=&value;
      printf("non problem1 \n");
              Object = (struct PCB*) shmaddr;
                printf("non problem2 \n");
                value.count=0;
                Object=&value;
             
              printf("non problem3 \n");
             shmaddr=Object;
               printf("non problem4\n");
               
              printf("ahhhhhhhhhhhhhhhh %d \n",Object->count);
         printf("ahhhhhh  %d \n",( (struct PCB*) shmaddr)->count);
       
         struct ProcessPCB mp;
         mp.arrivaltime=1000;
         mp.id=5;
         mp.priority=10;
         Insert(mp,((struct PCB*) shmaddr));
         printf("jjjjj %d %d %d", ( (struct PCB*) shmaddr)->Procsess[0].arrivaltime, ( (struct PCB*) shmaddr)->Procsess[0].id, ( (struct PCB*) shmaddr)->Procsess[0].priority);
    
     
}
// void GetData(struct processData  processArray[])
// {
//     struct processData p;
    
//      p.arrivaltime=-1;
//      p.id=-1;
//      p.priority=-1;
//      p.runningtime=-1;
  
//      if(counter<TotalProcess )
//      {  msgrcv(msgq_id,&p,sizeof(&p),0,IPC_NOWAIT);
//         if(p.arrivaltime!=-1 && p.id!=-1 && p.runningtime!=-1)
//         {  processArray[counter]=p;
//             printf(" in recieving id %d , arrival; %d , runtime %d , priority; %d \n ",processArray[counter].id,processArray[counter].arrivaltime,processArray[counter].runningtime,processArray[counter].priority);
//             counter++;
    
//         }

//     }
// }
  

