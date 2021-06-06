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

void down(int sem)
{
    /*union Semun semun;
    int VAL=semctl(sem, 0, GETVAL, semun);
    printf("semval from down %d \n",VAL);*/
    
    printf("Sch: down before the rcv..\n");

    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = -1;
    p_op.sem_flg = !IPC_NOWAIT;
    if (semop(sem, &p_op, 1) == -1)
    {
        perror("Sch: Error in down()");
        exit(-1);
    }

    
}


struct processData
{
    long mtype;
    int processinfo[4];
};
struct PCB table;
int runPro=0;
bool generatorHasFinished = false;

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

void doNotWaitForGenerator(int signum){
    generatorHasFinished = true;
}

int shmid;
void sendtoprocess(int remTime);
int main(int argc, char *argv[])
{
    initClk();
    signal (SIGUSR1, handler);
    signal (SIGUSR2, doNotWaitForGenerator);
    
    int prvtime=getClk();
    
    printf("Scheduler id= %d \n created at time: %d \n",getpid(),prvtime);
    key_t schedulerKey=1234;
    int msgq_id=msgget(schedulerKey,0666|IPC_CREAT);
    printf("msgq_id from Scudular %d \n",msgq_id);

   if(msgq_id==-1){printf("error in creating msgQueue \n"); exit(-1);}

    struct processData p;
    int rec_val=msgrcv(msgq_id,&p,sizeof(p.processinfo),0,!IPC_NOWAIT);
    if(rec_val == -1)
            perror("Errror in rec");

    int TotalProcess=p.processinfo[0];
      printf("num of process from Scheudler %d \n",TotalProcess);
    struct processData  processArray[TotalProcess];

  

    union Semun semun;
    key_t syncSendRecvSem = 7896;
    int semSync = semget(syncSendRecvSem, 1, 0666 | IPC_CREAT);

    if (semSync == -1)
    {
        perror("Error in create sem");
        exit(-1);
    }

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
     
     //If generator hasn't finished yet
     //then wait go down to wait as if s.th might be sent to you
     //If generator has finished
     //no need to down() and wait before recievness because there is nothing to recieve 5las
     if(!generatorHasFinished){
        down(semSync);
        printf("check for recievness %d \n",prvtime);
     }
     
     
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

   
    printf("scheduler is exiting..\n");
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
