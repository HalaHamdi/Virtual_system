#include "headers.h"
#include<string.h>
#include <sys/sem.h>

int msgq_id;

struct processData
{
    long mtype;
    int processinfo[4];
};


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


void clearResources(int);
void getChildren(int arr[]);
int getTotalLines(char fileName[], FILE *fp);
void storeProcessData(struct processData processArray[],FILE *fp,int Totallines);

int main(int argc, char *argv[])
{

    signal(SIGINT, clearResources);
    printf("Process generator id= %d  \n",getpid());
   
    char fileName[50];  
    int AlgorithmNmber;
    int quantum=-1;
    key_t schedulerKey=1234;
    int pid[2],stat_loc;
    struct processData Initiator;
   
    // * taking terminal parameters 
    //  Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    strcpy(fileName,argv[1]);
    AlgorithmNmber=atoi(argv[3]);
    if(argc>4){
        quantum=atoi(argv[5]);
    }

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

   
    //  Read the input files.
    FILE *fp=fopen(fileName,"r");
    int Totallines=getTotalLines(fileName,fp);
    struct processData processArray[Totallines-1];
    storeProcessData(processArray,fp,Totallines);


    // 3. Initiate and create the scheduler and clock processes.
    //destroyClk(true);
    getChildren(pid); //sometimes after clck terminates still running
    printf("clk id= %d ,scheduler id=%d \n",pid[0],pid[1]);

    //  initialize clock.
    initClk();


    // Create a data structure for processes and provide it with its parameters.
    
    msgq_id=msgget(schedulerKey,0666|IPC_CREAT);
    printf("msgq_id from generator %d \n",msgq_id);

    if(msgq_id==-1){
        printf("error in creating msgQueue \n");
         exit(-1);}


    //Send the Initiator which contains the number of processes , Algorithm number and any extra parameter (e.g :quantum).
    printf("num of process from generator %d \n",Totallines-1);

    Initiator.processinfo[0]=Totallines-1;
    Initiator.processinfo[1]=AlgorithmNmber;
    Initiator.processinfo[2]=quantum;
    Initiator.processinfo[3]=-1;
    Initiator.mtype=1;
    int send_val=msgsnd(msgq_id,&Initiator,sizeof(Initiator.processinfo),!IPC_NOWAIT);

     if(send_val == -1)
     perror("Errror in send");
    //send each process when its time comes
    int counter=0;
    int prvTime = -1;
    while(true){
        
        if(prvTime !=  getClk()){
            printf("current time from generator %d \n",getClk());
                
            if(getClk()>=processArray[counter].processinfo[1]){
                printf("Gen: new process arrived %d \n",getClk());
                int send_val=msgsnd(msgq_id,&processArray[counter],sizeof(processArray[counter].processinfo),!IPC_NOWAIT);
                printf("Gen: process sent to the Sch \n");
                
                if(send_val == -1)
                    perror("Errror in send");
                counter++;
            }
            prvTime = getClk();
            

            union Semun semun;
            int VAL=semctl(semSync, 0, GETVAL, semun);
            printf("Gen: VAL = %d\n",VAL);
            if(VAL == 0 ){
                //I've placed this if condition so that
                //If Val was 1 then no need to up again
                //Upping a semaphore with value one, would result in value = 2
                //which introduces an undesired behavour at the down side
                printf("Gen: up now after the send..\n");
                up(semSync);
            }
            if(counter==Totallines-1){break;}
            
        }
    }


    
    //Sending a signal to the scheduler to inform him
    //that the generator has finished 
    //and no need to wait on the down semaphore
    //because there is nothing left to be sent before the recievness line 
    kill(pid[1], SIGUSR2); //pid[1] refers to the pid of the forked scheduler

    //will wait till the scheduler finishes   
    printf("generator is waiting..\n");
    waitpid(pid[1],&stat_loc,0);
    
    
    
    // 7. Clear clock resourcess
    printf("generator is exiting..\n");
    destroyClk(true);
}

void clearResources(int signum)
{
    msgctl(msgq_id,IPC_RMID,(struct msqid_ds*)0);
    killpg(getpgrp(),SIGKILL);
    signal(SIGINT, clearResources);

}
void getChildren(int pid[]){
   pid[0]=fork();

    if(pid[0]==-1){perror("error in forking clock");}
    else if (pid[0]==0){
        execl("./clk.out","clk.out",NULL); 
    }

    pid[1]=fork();
   
    if(pid[1]==-1){perror("error in forking scheduler");}
    else if (pid[1]==0){

        execl("./scheduler.out","scheduler.out",NULL); 

    }
}

int getTotalLines(char fileName[],FILE *fp){
    
    if(fp==NULL){ printf("File \"%s\" does not exist!!!\n",fileName); exit(-1);}
    char ch;
    int Totallines=0;
    //getting number of processes made in text file.
    //by counting the number of lines
    while((ch=fgetc(fp))!=EOF){
        if(ch=='\n'){Totallines++;}
    }
    return Totallines;
}
void storeProcessData(struct processData processArray[],FILE *fp,int Totallines){
    fseek(fp, 0, SEEK_SET);
    char word[20];
    //skip the first line 
    for(int i=0;i<4;i++){fscanf(fp,"%s",word);}
    //store the processes 
    for(int i=0;i<Totallines-1;i++){
        struct processData p;
        fscanf(fp,"%d",&p.processinfo[0]);
        fscanf(fp,"%d",&p.processinfo[1]);
        fscanf(fp,"%d",&p.processinfo[2]);
        fscanf(fp,"%d",&p.processinfo[3]);
        processArray[i]=p;
    }
    fclose(fp);
}