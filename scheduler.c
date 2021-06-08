#include "headers.h"
#include"DS.h"
#include"string.h"
#include "MEM.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

extern int errno;

union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    ushort *array;         /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};
void PreemtiveHPF();
void CallGetNextFit();
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
FILE * fp;
FILE * MEMf;
void Openfile(){
   fp = fopen ("scheduler.log","w");
   fprintf (fp, "#At time x process y state arr w total z remaing y wait k \n");
}
void Closefile(){
fclose (fp);
}
void WritetoFile(int id,char *state,int arr,int total,int remaining,int wait){
   
        fprintf(fp,"At time %d process %d %s arr %d total %d remain %d wait %d \n",getClk(),id,state,arr,total,remaining,wait);
    
}

void OpenMEMf(){
   MEMf = fopen ("memory.log","w");
   fprintf (MEMf, "#At time x allocated y bytes for process z from i to j \n");
}
void CloseMEMf(){
fclose (MEMf);
}
void WritetoMEMf(int id,int space,int from,int to){
   
        fprintf(fp,"At time %d allocated %d bytes for process %d from %d to %d \n",getClk(),id,space,from,to);
    
}


struct processData
{
    long mtype;
    int processinfo[5];
};

struct PCB table;
struct PCB Procsesswait;
struct Freeblocks F;
int runPro=0;
bool generatorHasFinished = false;
int pid=-1;
int nextfit=0;
int procCount=0;
int memAlg;
int AlgorithmNmber = -1;
int procCount;

void memAlg1(){
    int len=Procsesswait.count;
  for(int i=0;i<len;i++){

    printf("IIn memAlg 1 from functionAlg1 \n");
    printFreeSpace(&F);
    struct ProcessPCB waitP=Remove(&Procsesswait);
    struct Free getblock=GitFristFit(waitP.memsize,&F);
    if(getblock.space!=0){
    waitP.from=getblock.from;
    waitP.to=getblock.from+waitP.memsize;
    if(waitP.to-waitP.from!=getblock.space){   //we have external fragmentation need to pushed
    getblock.from=waitP.to;
    getblock.space=waitP.to-waitP.from;
    insertStart(getblock,&F);
     }
    waitP.inmemory=true; 
    Push(waitP,&table);
    procCount++;
    WritetoMEMf(waitP.id,waitP.memsize,waitP.from,waitP.to);
    printf("Has P with id= %d lockated in memory from %d to %d with space %d \n",waitP.id,waitP.from,waitP.to,waitP.memsize);
    }else{
     Push(waitP,&Procsesswait);
     printf("still not have place for it");
    }
  }
}

bool tryToAllocate_BestFit(struct ProcessPCB* p){
    struct Free getblock=GetBestFit(p->memsize,&F);
    if(getblock.space!=0){
        p->from = getblock.from;
        p->to = getblock.from + p->memsize;
        if(p->to - p->from != getblock.space){   //we have external fragmentation need to pushed
            getblock.from=p->to;
            getblock.space=getblock.to-getblock.from;
            insertStart(getblock, &F);
        }
        p->inmemory=true; 
        printf("Has P with id= %d lockated in memory from %d to %d with space %d \n",p->id,p->from,p->to,p->memsize);
    }
    else{
        p->inmemory=false;
    }

    //if was allocated will return true, if wasn't will return false
    return p->inmemory;
}
void memAlg3(struct Free Pblock){
    //Inserted the fred space after this process has finished
        insertStart(Pblock,&F);
        Marge(&F);
        
        printf("After deAllocating\n");
        printFreeSpace(&F);

        for(int i=0; i < Procsesswait.count; i++){
            bool isAllocated = tryToAllocate_BestFit(&Procsesswait.Procsess[i]);
            if(isAllocated){
                struct ProcessPCB p = Procsesswait.Procsess[i];
                //Remove that process from the Processwait PCB
                removeByIndex(i,&Procsesswait);
                if(AlgorithmNmber == 4){
                    InsertSortedByRemainTime(p, &table);
                }
                else{
                    Push(p,&table);
                }
                printf("inserted in table\n");
                procCount++;                    
            }
        }

}

void  dealwithFinished()
{
    //printf("My Pid is [%d], and i have got signal #%d\n",getpid(), signum);
    char finished[]="finished";  
    strcpy(table.Procsess[0].state,finished);
    printf("process Finished %d \n",table.Procsess[0].id);
    table.Procsess[0].remanningtime=0;
    WritetoFile(table.Procsess[0].id,table.Procsess[0].state,table.Procsess[0].arrivaltime,table.Procsess[0].runningtime,table.Procsess[0].remanningtime,table.Procsess[0].wait);
    
    runPro=0;
     //memAlg 1
    struct Free Pblock;
    Pblock.from=table.Procsess[0].from;
    Pblock.to=table.Procsess[0].to;
    Pblock.space=table.Procsess[0].memsize;
    insertStart(Pblock,&F);
    Marge(&F);
    if(Procsesswait.count>0){   //if process in wait List

    if(memAlg==1){
        memAlg1();
    }else if(memAlg==2){
        insertSpace(Pblock,&F);
        CallGetNextFit();

    }else if(memAlg==3){
        printf("Before deAllocating\n");
        printFreeSpace(&F);
        memAlg3(Pblock);
    }
   }
    Remove(&table);
    runPro=0;
  /* signal( SIGUSR1, handler); */
}

void doNotWaitForGenerator(int signum){
    generatorHasFinished = true;
}
void CallGetNextFit()
{
    if(Procsesswait.count>0)
     { int position=nextfit;
                        nextfit=GetNextfit(Procsesswait.Procsess[0].memsize,&F,nextfit);
                        if(nextfit==-1)
                        {
                            nextfit=position;
                        }
                        else
                        {Procsesswait.Procsess[0].from=F.Mem[nextfit].from;
                         {Procsesswait.Procsess[0].to=F.Mem[nextfit].from+Procsesswait.Procsess[0].memsize;
                          if(Procsesswait.Procsess[0].to-Procsesswait.Procsess[0].from!=F.Mem[nextfit].space)
                          {
                              F.Mem[nextfit].from=Procsesswait.Procsess[0].to;
                              F.Mem[nextfit].space=F.Mem[nextfit].to-F.Mem[nextfit].from;
                               insertStart(F.Mem[nextfit], &F);
                               Marge(&F);

                          }
                          Procsesswait.Procsess[0].inmemory=true;
                          WritetoMEMf(Procsesswait.Procsess[0].id,Procsesswait.Procsess[0].memsize,Procsesswait.Procsess[0].from,Procsesswait.Procsess[0].to);
                          printf("Has P with id= %d lockated in memory from %d to %d with space %d /n",Procsesswait.Procsess[0].id,Procsesswait.Procsess[0].from,Procsesswait.Procsess[0].to,Procsesswait.Procsess[0].memsize);

                          Push(Procsesswait.Procsess[0],&table);
                          Remove(&Procsesswait);


                        }
     }
}
}

 

int shmid;
int currentProcessRemTime;
void sendtoprocess(int remTime);
void handleNextProcess();
int getRemTimeFromProcess();

int main(int argc, char *argv[])
{
    initClk();
    //signal (SIGUSR1, handler);
    signal (SIGUSR2, doNotWaitForGenerator);
    Openfile();   
    OpenMEMf();
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
    AlgorithmNmber=p.processinfo[1];
    int quantum=p.processinfo[2];
    memAlg=p.processinfo[3];
    printf("num of process from Scheudler %d  and mem Algo %d \n",TotalProcess,memAlg);
    struct processData  processArray[TotalProcess];


    union Semun semun;
    key_t syncSendRecvSem = 7896;
    int semSync = semget(syncSendRecvSem, 1, 0666 | IPC_CREAT);

    if (semSync == -1)
    {
        perror("Error in create sem");
        exit(-1);
    }
    
    semun.val = 0; /* initial value of the semaphore, Binary semaphore */
    if (semctl(semSync, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }

    key_t FinsgedrocessSem = 7788;
    int semFinish = semget(FinsgedrocessSem, 1, 0666 | IPC_CREAT);

    if (semFinish == -1)
    {
        perror("Error in create sem FinsgedrocessSem");
        exit(-1);
    }
    if (semctl(semFinish, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }
    F.count = 0;
    struct Free Block;
    Block.from=0;
    Block.to=1024;
    Block.space=1024;
    insertStart(Block,&F);
    struct ProcessPCB Procsess;
    table.count=0;
    Procsesswait.count=0;

    procCount=0;
    char State[]="waiting";
    char stoppedState[] = "stopped";
    int FinshtimePro=-1; //Finishtrime for the runing process if it is nonPreemptive
    
    //flag determines something have been recieved in this time step
    bool hasRecivedNow = false;
     currentProcessRemTime = -1;
    while(true){

        if(prvtime!=getClk()){
            prvtime=getClk();
            
            printf("current time %d \n",prvtime);
            
            //reading the remianing time from the currently running process
            if(runPro != 0){
                //for each time step where a process is running
                //read its remaining time
                //This is done for each second not only at times when something new arrived
                //Since we want to ensure that every up in the process is consumed and corresponds to a down in the scheduler
                currentProcessRemTime = getRemTimeFromProcess();
                table.Procsess[getProcess(pid,&table)].remanningtime = currentProcessRemTime;
                    
            }

            //If generator hasn't finished yet
            //then wait go down to wait as if s.th might be sent to you
            //If generator has finished
            //no need to down() and wait before recievness because there is nothing to recieve 5las
            if(!generatorHasFinished){
                printf("Sch: down before the rcv..\n");
                down(semSync);
                printf("check for recievness %d \n",prvtime);
            }
             if(FinshtimePro==prvtime){  //there is processes should be terminate
                    down(semFinish);
                    dealwithFinished(&F);
             }
            
            //Recieving new arrived processes if found 
            while(true){
                //printf("check for recievness %d \n",prvtime);
                rec_val =rec_val=msgrcv(msgq_id,&p,sizeof(p.processinfo),0,IPC_NOWAIT);
                if(rec_val == -1)
                {     // perror("Errror in recccccccccccc"); 
                    break;
                }
                else{
                    //something have arrived
                    hasRecivedNow = true;
                        
                    printf("process ID %d \n",p.processinfo[0]);
                    Procsess.id=p.processinfo[0];
                    Procsess.arrivaltime=p.processinfo[1];
                    Procsess.runningtime=p.processinfo[2];
                    Procsess.priority=p.processinfo[3];
                    Procsess.remanningtime=p.processinfo[2];
                    Procsess.memsize=p.processinfo[4];
                    Procsess.wait=0;
                    Procsess.inmemory=false;
                    strcpy(Procsess.state,State);
                    //Memory
                    if(memAlg==1){
                    printf("IIn memAlg 1 \n");
                    printFreeSpace(&F);
                    struct Free getblock=GitFristFit(Procsess.memsize,&F);
                    if(getblock.space!=0){
                    Procsess.from=getblock.from;
                    Procsess.to=getblock.from+Procsess.memsize;
                    if(Procsess.to-Procsess.from!=getblock.space){   //we have external fragmentation need to pushed
                     getblock.from=Procsess.to;
                     getblock.space=getblock.to-getblock.from;
                     insertStart(getblock,&F);
                    }
                    Procsess.inmemory=true; 
                    WritetoMEMf(Procsess.id,Procsess.memsize,Procsess.from,Procsess.to);
                    printf("Has P with id= %d lockated in memory from %d to %d with space %d \n",Procsess.id,Procsess.from,Procsess.to,Procsess.memsize);
                    }
                    }

                    else if (memAlg == 3){
                        printf("In memAlg 3 \n");
                        printFreeSpace(&F);
                        tryToAllocate_BestFit(&Procsess); 
                        printFreeSpace(&F);                       

                    }  
                    else if(memAlg==2)
                    {
                        // next fit;
                        int position=nextfit;
                        nextfit=GetNextfit(Procsess.memsize,&F,nextfit);
                        if(nextfit==-1)
                        {
                            nextfit=position;
                        }
                        else
                        {Procsess.from=F.Mem[nextfit].from;
                          Procsess.to=F.Mem[nextfit].from+Procsess.memsize;
                          if(Procsess.to-Procsess.from!=F.Mem[nextfit].space)
                          {
                              F.Mem[nextfit].from=Procsess.to;
                              F.Mem[nextfit].space=F.Mem[nextfit].to-F.Mem[nextfit].from;
                               insertStart(F.Mem[nextfit], &F);
                              

                          }
                          Procsess.inmemory=true;
                        WritetoMEMf(Procsess.id,Procsess.memsize,Procsess.from,Procsess.to);  
                        printf("Has P with id= %d lockated in memory from %d to %d with space %d /n",Procsess.id,Procsess.from,Procsess.to,Procsess.memsize);


                        }
                         printFreeSpace(&F);       
                    }  

                    if(Procsess.inmemory){
                    if(AlgorithmNmber == 4){
                        InsertSortedByRemainTime(Procsess, &table);
                    }
                    else{
                        Push(Procsess,&table);
                    }
                    procCount++;
                    }
                    else{
                      Push(Procsess,&Procsesswait);
                    }
                
                    /////////////////////////////////here we will try to alocate process in memoy if not alcated put it in waitList and pop from table
                }    
            }
            printf("table count %d\n",table.count);

            //Redirecting
            if(AlgorithmNmber==2 || AlgorithmNmber==1){

                if(table.count!=0 && runPro==0){
                    if(AlgorithmNmber==2){    
                    sortrunnigtime(&table);
                    }
                    //FinshtimePro=prvtime+table.Procsess[0].runningtime;
                    sendtoprocess(table.Procsess[0].runningtime);   
                    
                    pid=fork();
                    if(pid==-1){perror("error in forking clock");}
                    else if (pid==0){                                 //child
                        execl("./process.out","process.out",NULL);
                        }
                    
                    char runing[]="started";
                    printf("process started %d \n",table.Procsess[0].id);  
                    strcpy(table.Procsess[0].state,runing);
                    table.Procsess[0].pid=pid;
                    WritetoFile(table.Procsess[0].id,table.Procsess[0].state,table.Procsess[0].arrivaltime,table.Procsess[0].runningtime,table.Procsess[0].remanningtime,table.Procsess[0].wait);
                    runPro=1;
                    FinshtimePro=prvtime+table.Procsess[0].runningtime;
                }
            }
            else if(AlgorithmNmber==3)
            {
                sortpriority(&table);
                PreemtiveHPF();
            }
            else if (AlgorithmNmber == 4){
                if( runPro != 0 && currentProcessRemTime == 0){
                    dealwithFinished(&F);
                }
                
                //if a new process has arrived go compare it with the currently running
                if(table.count!=0 && runPro!=0 && hasRecivedNow){
                    
                    int currentRunningIndex = getProcess(pid,&table);
                    
                    printf("currentRunningIndex: %d\n",currentRunningIndex);
                    PrintProcess(table.Procsess[0]);
                    //if the remaining time of the processes currently running is larger than that of the first process in the queue (the process that mostly deserves to run in SRTN) 
                    if(currentProcessRemTime > table.Procsess[0].remanningtime){
                        //send stop signal to the currently running process
                        
                        printf("sending a stop signal \n");
                        kill(pid,SIGSTOP);
                        strcpy(table.Procsess[currentRunningIndex].state,stoppedState);
                        runPro = 0;

                        WritetoFile(table.Procsess[currentRunningIndex].id,table.Procsess[currentRunningIndex].state,table.Procsess[currentRunningIndex].arrivaltime,table.Procsess[currentRunningIndex].runningtime,table.Procsess[currentRunningIndex].remanningtime,table.Procsess[currentRunningIndex].wait);

                    }
                    //update the Remaining time of the currently running process with the value it has read from the shared mem
                    PrintProcess(table.Procsess[currentRunningIndex]);
                    
                }
                //if no process is currently running, go fetch the next one
                if(table.count!=0 && runPro==0){
                    handleNextProcess();
                }
            }
            hasRecivedNow = false;
        }

        if(procCount==TotalProcess&& table.count==0){break;}
    
    }

    Closefile();
    CloseMEMf();
    printf("scheduler is exiting..\n");
    destroyClk(true);
}
void handleNextProcess(){

        printf("In handling next process\n");
        if(strcmp(table.Procsess[0].state,"waiting") == 0){  
            sendtoprocess(table.Procsess[0].remanningtime); 
            printf("Forking nxt process\n");
            pid=fork();
            if(pid==-1){perror("error in forking clock");}
            else if (pid==0){                                 //child
                execl("./process.out","process.out",NULL);
            }    
            char runing[]="started";
            printf("process started %d \n",table.Procsess[0].id);  
            strcpy(table.Procsess[0].state,runing);
            table.Procsess[0].pid = pid;
            
            WritetoFile(table.Procsess[0].id,table.Procsess[0].state,table.Procsess[0].arrivaltime,table.Procsess[0].runningtime,table.Procsess[0].remanningtime,table.Procsess[0].wait);
                    
        }
        else if (strcmp(table.Procsess[0].state,"stopped") == 0){
            //if the current proccess to be runned had a stopped state
            //then no need to fork it, it was already forked before
            //send to it a continue signal with its latest remaining time
            pid = table.Procsess[0].pid;
            kill(pid,SIGCONT);
            char resumed[]="resumed";
            printf("process resumed! %d \n",table.Procsess[0].id);  
            strcpy(table.Procsess[0].state,resumed);

            WritetoFile(table.Procsess[0].id,table.Procsess[0].state,table.Procsess[0].arrivaltime,table.Procsess[0].runningtime,table.Procsess[0].remanningtime,table.Procsess[0].wait);

        }
        //whatever was the state, now we have a running process
        runPro=1;
}


int getRemTimeFromProcess(){

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

    char remTimeString[10];
    down(sem1);
    
    
    strcpy(remTimeString, (char *) shmaddr);
    int remTime = atoi(remTimeString);
    printf("Remaining Time recieved from the process %d \n", remTime);
    //table.Procsess[0].remanningtime = remTime;
    return remTime;
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
     //printf("HPF\n");
     if(runPro==1 && table.count>0)
     {
         printf("can hereeeeeeeeeeeeeee\n");
         int time=currentProcessRemTime;
         
         table.Procsess[getProcess(pid,&table)].remanningtime=time;
         if(time==0)
         {
            printf("al mfroood index0 = %d",getProcess(pid,&table));
            dealwithFinished();
            runPro=0;
         }
     }
      
   if(runPro==0 && table.count>0)
    { 
        //printf("i am here\n");
          // check if no process is running now
        struct ProcessPCB p=table.Procsess[0];
        // printf("state %s",p.state);
        // check if the current process in the first of queue  , its first time to run
        bool check=false;
        char stop[]="stopped";
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
        char state[]="started";
        strcpy(table.Procsess[0].state,state);
        // tell the schuelder that he runs process now 
        runPro=1;
        // send remainimg time to process through shared memory
       sendtoprocess(table.Procsess[0].remanningtime);
       // store copy of process " for get its  data  quickly if you will preeemtive it"
     
       // fork the process
       //printf("i will for\n");
        pid=fork();
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
            WritetoFile(table.Procsess[0].id,table.Procsess[0].state,table.Procsess[0].arrivaltime,table.Procsess[0].runningtime,table.Procsess[0].remanningtime,table.Procsess[0].wait);

        }
      }
      // if the process was sleeping /stopped , wake up it again 
      else
      { runPro=1;
          char state[]="resumed";
          table.Run=table.Procsess[0];
          strcpy(table.Procsess[0].state,state);
          printf("Process %d Continue\n",table.Procsess[0].remanningtime);
          pid = table.Procsess[0].pid;
          printf("getpid %d\n",table.Procsess[0].pid);
          kill(table.Procsess[0].pid,SIGCONT);
          WritetoFile(table.Procsess[0].id,table.Procsess[0].state,table.Procsess[0].arrivaltime,table.Procsess[0].runningtime,table.Procsess[0].remanningtime,table.Procsess[0].wait);

      }
    }
    else if(table.count>0){
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
        
        //printf("ok done Raghadddddddddddddddddddddddddd\n");
        // check if index not 0 which means there is a process has less prioirty of current
         // get the remaining time from current process which is runnging now
        //  if(index_running!=-1)
        // table.Procsess[index_running].remanningtime=atoi(remain);
          // try to preeemtive it if you can
          bool checkequals=false;
        if(index_running!=0  )
        { //printf("tmam okkkkkkkkkkkkk");
            
              if(table.Procsess[0].priority==table.Procsess[index_running].priority)
                {
                    if(table.Procsess[0].remanningtime<table.Procsess[index_running].remanningtime)
                    {
                        checkequals=true;
                    }
                
                }
                else
                 checkequals=true;
           
            
            if(checkequals)
            {printf("i will stop\n");
            // change state of current process
            
             char state[]="stopped";
             strcpy(table.Procsess[index_running].state,state);
             WritetoFile(table.Procsess[index_running].id,table.Procsess[index_running].state,table.Procsess[index_running].arrivaltime,table.Procsess[index_running].runningtime,table.Procsess[index_running].remanningtime,table.Procsess[index_running].wait);

             
             printf("Process %d Stopped\n",table.Procsess[index_running].id);
             // send signal for process to stop it
             kill(table.Procsess[index_running].pid,SIGSTOP);
             // get the process which with less priority
            
             struct ProcessPCB p=table.Procsess[0];

            
             // chek if it running for fisrt time or not
             // all the same pervious steps
             bool check=false;
        char stop[]="stopped";
        for(int i=0;i<7;i++)
        {
            if(table.Procsess[0].state[i]!=stop[i])
            {
                check=true;
                break;
            }
            
        }
            if(check)
            {  char state[]="started";
                strcpy(table.Procsess[0].state,state);
            runPro=1;
            // printf("before sending");
            sendtoprocess(table.Procsess[0].remanningtime);
            // printf("sending\n");
       
              pid=fork();

                if(pid==0)
                {
                    printf("Process %d started with time %d\n",table.Procsess[0].id,table.Procsess[0].remanningtime);
                execl("./process.out","process.out",NULL);
                }
                else if(pid==-1)
                
                {
                    printf("Error \n");
                    exit(-1);
                }
                else
                { 
                    table.Procsess[0].pid=pid;
                    table.Run=table.Procsess[0];
                    WritetoFile(table.Procsess[0].id,table.Procsess[0].state,table.Procsess[0].arrivaltime,table.Procsess[0].runningtime,table.Procsess[0].remanningtime,table.Procsess[0].wait);
                }
            }
            else
            {
                runPro=1;
                table.Run=table.Procsess[0];
                char state[]="resumed";
                pid = table.Procsess[0].pid;
                printf("Process %d Continue\n",table.Procsess[0].remanningtime);
                strcpy(table.Procsess[0].state,state);
                kill(table.Procsess[0].pid,SIGCONT);
                WritetoFile(table.Procsess[0].id,table.Procsess[0].state,table.Procsess[0].arrivaltime,table.Procsess[0].runningtime,table.Procsess[0].remanningtime,table.Procsess[0].wait);

            }
            }
        }
        

    }
    // printf("i am outtttttttttttttttttttttttttttttttttttttttttttt hhhhhhhhhhhh\n");
       
}
