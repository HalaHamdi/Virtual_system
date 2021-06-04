#include "headers.h"
#include"DS.h"
#include"string.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>

union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    ushort *array;         /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};


void up(int sem)
{
    struct sembuf v_op;

    v_op.sem_num = 0;
    v_op.sem_op = 1;
    v_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &v_op, 1) == -1)
    {
        perror("Error in up()");
        exit(-1);
    }

    union Semun semun;
    int VAL=semctl(sem, 0, GETVAL, semun);
    printf("semval after UUUP %d \n",VAL);
}


struct processData
{
    long mtype;
    int processinfo[4];
    //int arrivaltime;
    //int priority;
    //int runningtime;
    //int id;
};
struct PCB table;
int runPro=0;
void handler(int signum)
{
 
    //printf("My Pid is [%d], and i have got signal #%d\n",getpid(), signum);
    char finished[8]="finished";  
    strcpy(table.Procsess[0].state,finished);
    printf("process Finished %d \n",table.Procsess[0].id);
    Remove(&table);
    runPro=0;
  
  /* signal( SIGUSR1, handler); */
}

int shmid;
void sendtoprocess(int remTime);
int main(int argc, char *argv[])
{
    initClk();
    signal (SIGUSR1, handler);
    int prvtime=getClk();
    printf("Schadular id= %d  \n",getpid());
    key_t schedulerKey=1234;
    int msgq_id=msgget(schedulerKey,0666|IPC_CREAT);
     printf("msgq_id from Scudular %d \n",msgq_id);

   if(msgq_id==-1){printf("error in creating msgQueue \n"); exit(-1);}

    struct processData p;
    int rec_val=msgrcv(msgq_id,&p,sizeof(p.processinfo),0,!IPC_NOWAIT);
    if(rec_val == -1)
            perror("Errror in rec");

    int TotalProcess=p.processinfo[0];
      printf("num of process from Schudler %d \n",TotalProcess);
    struct processData  processArray[TotalProcess];

  /*  for(int counter=0 ;counter<TotalProcess;counter++ ){
     msgrcv(msgq_id,&p,sizeof(p.processinfo),0,!IPC_NOWAIT);
     processArray[counter]=p;

    }*/
    
    
    struct ProcessPCB Procsess;
    table.count=0;
    int procCount=0;
    char State[7]="waiting";
    //int FinshtimePro=-1;
    int pid=-1;
    while(true){

    if(prvtime!=getClk()){
     prvtime=getClk();
     printf("current time %d \n",prvtime);
     rec_val =msgrcv(msgq_id,&p,sizeof(p.processinfo),0, IPC_NOWAIT);
     if(rec_val == -1)
      {     // perror("Errror in rec"); 
      }
      else{
        printf("process ID %d \n",p.processinfo[0]);
        Procsess.id=p.processinfo[0];
        Procsess.arrivaltime=p.processinfo[1];
        Procsess.runningtime=p.processinfo[2];
        Procsess.priority=p.processinfo[3];
        Procsess.remanningtime=p.processinfo[2];
        strcpy(Procsess.state,State);
        Push(Procsess,&table);
        procCount++;
      }    
        /*if(prvtime==FinshtimePro){
        char finished[8]="finished";  
        strcpy(table.Procsess[0].state,finished);
        printf("process Finished %d \n",table.Procsess[0].id);
        Remove(&table);
        runPro=0;
        }*/
        //printf("Raghad \n");
        if(table.count!=0 && runPro==0){
        sortrunnigtime(&table);
        //FinshtimePro=prvtime+table.Procsess[0].runningtime;
        sendtoprocess(table.Procsess[0].runningtime);   
        
        pid=fork();
        if(pid==-1){perror("error in forking clock");}
        else if (pid==0){                                 //child
            execl("./process.out","process.out",NULL);
             }
          
        char runing[7]="started";
        printf("process started %d \n",table.Procsess[0].id);  
        strcpy(table.Procsess[0].state,runing);
        runPro=1;
        }
        

    }

    if(procCount==TotalProcess&& table.count==0){break;}
    }

    // while (true)
    // {
         /* code */
    // }
     
  //  if(table.count>0){

  //  }
  //  for(int i =0;i<TotalProcess;i++)
  //     printf(" id %d , arrival; %d , runtime %d , priority; %d \n ",processArray[i].processinfo[0],processArray[i].processinfo[1],processArray[i].processinfo[2],processArray[i].processinfo[3]);

    //PrintPCB(&table);
    destroyClk(true);
}

void sendtoprocess(int remTime){
    int shmid, pid,sempho1;
    //memo = ftok("Up", 'r');
     sempho1 = ftok("sem1", 'a');
     key_t memo=7893;
     //key_t sempho1=7894;

    shmid = shmget(memo, 4096, IPC_CREAT | 0644);

    if (shmid == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    void *shmaddr = shmat(shmid, (void *)0, 0);
   /* if (shmaddr == -1)
    {
        perror("Error in attach in Clint");
        exit(-1);
    }*/
    union Semun semun;

    int sem1 = semget(sempho1, 1, 0666 | IPC_CREAT);

    if (sem1 == -1)
    {
        perror("Error in create sem");
        exit(-1);
    }

    semun.val = 0; /* initial value of the semaphore, Binary semaphore */
    if (semctl(sem1, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }
    char text[10];
   sprintf(text, "%d", remTime);  
   strcpy((char *) shmaddr,text);
   //memset(shmaddr,remTime,1);
    up(sem1);
    int VAL= semctl(sem1, 0, GETVAL, semun);

    //sem_getvalue(sem1, &VAL);
    printf("semval after UUUP in function %d \n",VAL);

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
    

    // struct PCB value;
    // struct PCB  *Object=&value;
    //   printf("non problem1 \n");
    //           Object = (struct PCB*) shmaddr;
    //             printf("non problem2 \n");
    //             value.count=0;
    //             Object=&value;
             
    //           printf("non problem3 \n");
    //          shmaddr=Object;
    //            printf("non problem4\n");
               
    //           printf("ahhhhhhhhhhhhhhhh %d \n",Object->count);
    //      printf("ahhhhhh  %d \n",( (struct PCB*) shmaddr)->count);
       
    //      struct ProcessPCB mp;
    //      mp.arrivaltime=1000;
    //      mp.id=5;
    //      mp.priority=10;
    //      Insert(mp,((struct PCB*) shmaddr));
    //      printf("jjjjj %d %d %d", ( (struct PCB*) shmaddr)->Procsess[0].arrivaltime, ( (struct PCB*) shmaddr)->Procsess[0].id, ( (struct PCB*) shmaddr)->Procsess[0].priority);
    
     
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
  

