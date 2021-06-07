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
void PreemtiveHPF();

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
    //printf("semval after UUUP %d \n",VAL);
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
    int AlgorithmNmber=p.processinfo[1];
    int quantum=p.processinfo[2];
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
     //   printf("check for recievness %d \n",prvtime);
     }
     
    while(true){
     rec_val =msgrcv(msgq_id,&p,sizeof(p.processinfo),0, IPC_NOWAIT);
     if(rec_val == -1)
      {     // perror("Errror in rec"); 
        break;
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
       
     }  
     if(AlgorithmNmber==1)
        {if(table.count!=0 && runPro==0){
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
        else if(AlgorithmNmber==4)
        {
             sortpriority(&table);
            PreemtiveHPF();
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
    //printf("semval after UUUP in function %d \n",VAL);

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

    
     
}
void PreemtiveHPF()
{
    char remain[100];
    // printf("HPF\n");
      
   if(runPro==0)
    { 
        //printf("i am here\n");
          // check if no process is running now
        struct ProcessPCB p=table.Procsess[0];
        // printf("state %s",p.state);
        // check if the current process in the first of queue  , its first time to run
        bool check=false;
        char stop[7]="stopped";
        for(int i=0;i<7;i++)
        {
            if(table.Procsess[0].state[i]!=stop[i])
            {
                check=true;
                break;
            }
            
        }
      if(check)
      { 
        // change state to be running
        char state[7]="running";
        strcpy(table.Procsess[0].state,state);
        // tell the schuelder that he runs process now 
        runPro=1;
        // send remainimg time to process through shared memory
       sendtoprocess(table.Procsess[0].remanningtime);
       // store copy of process " for get its  data  quickly if you will preeemtive it"
     
       // fork the process
       printf("i will for\n");
      int  pid=fork();
       printf("Process %d started\n",table.Procsess[0].id);

        if(pid==0)
        {
            // make the process start to run
          
          execl("./process.out","process.out",NULL);
        }
        else if(pid==-1)
        
        {
            // if error happen exits from the program
            printf("Error \n");
            exit(-1);
        }
        else
        {  // store the pid of running process in scheulder
           printf("pid %d\n",pid);
            table.Procsess[0].pid=pid;
              table.Run=table.Procsess[0];
            
        }
      }
      // if the process was sleeping /stopped , wake up it again 
      else
      { runPro=1;
            char state[7]="running";
              table.Run=table.Procsess[0];
          strcpy(table.Procsess[0].state,state);
          printf("Process %d Continue\n",table.Procsess[0].id);
          printf("getpid %d\n",table.Procsess[0].pid);
          kill(table.Procsess[0].pid,SIGCONT);
      }
    }
    else{
        // printf("checkks\n");
        // printf("Runpro %d \n",runPro);
      
            // get remaining time of current process
             // update it in scheulder
             // it upadted with every time but update it again for more correct
        //   int shmid = shmget(7893, 4096, IPC_CREAT | 0644);
        //   void *shmaddr = shmat(shmid, (void *)0, 0);
        //   strcpy(remain,(char*)shmaddr);

        // get the index of current running process
        printf("pid  here %d\n",table.Run.pid);
        int index_running=getProcess(table.Run.pid,&table);

        printf("indexof running  %d %d\n",table.Run.id,index_running);
        // check if index not 0 which means there is a process has less prioirty of current
         // get the remaining time from current process which is runnging now
        //  if(index_running!=-1)
        // table.Procsess[index_running].remanningtime=atoi(remain);
          // try to preeemtive it if you can
          bool checkequals=false;
        if(index_running!=0  && index_running!=-1 )
        {
            if(table.Procsess[0].priority==table.Procsess[index_running].priority)
            {
                if(table.Procsess[0].remanningtime<table.Procsess[index_running].remanningtime)
                {
                    checkequals=true;
                }
            }
            else
            {
                   checkequals=true;
            }
            if(checkequals)
            {printf("i will stop\n");
            // change state of current process
             char state[7]="stopped";
             strcpy(table.Procsess[index_running].state,state);

             table.Procsess[index_running].remanningtime=atoi(remain);
             printf("Process %d Stopped\n",table.Procsess[index_running].id);
             // send signal for process to stop it
             kill(table.Procsess[index_running].pid,SIGSTOP);
             // get the process which with less priority
             struct ProcessPCB p=table.Procsess[0];
             // chek if it running for fisrt time or not
             // all the same pervious steps
             bool check=false;
        char stop[7]="stopped";
        for(int i=0;i<7;i++)
        {
            if(table.Procsess[0].state[i]!=stop[i])
            {
                check=true;
                break;
            }
            
        }
            if(check)
            {  char state[7]="running";
                strcpy(table.Procsess[0].state,state);
            runPro=1;
            sendtoprocess(table.Procsess[0].remanningtime);
       
            int  pid=fork();

                if(pid==0)
                {
                    printf("Process %d started\n",table.Procsess[0].id);
                execl("./process.out","process.out",NULL);
                }
                else if(pid==-1)
                
                {
                    printf("Error \n");
                    exit(-1);
                }
                else
                { table.Procsess[0].pid=pid;
                     table.Run=table.Procsess[0];
                   
                }
            }
            else
            {
                runPro=1;
                table.Run=table.Procsess[0];
                char state[7]="running";
                 printf("Process %d Continue\n",table.Procsess[0].id);
                strcpy(table.Procsess[0].state,state);
                kill(table.Procsess[0].pid,SIGCONT);
            }
            }
        }

    }
      
       
}