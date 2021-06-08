#include "headers.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include"string.h"
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
        perror("Error in down()");
        exit(-1);
    }
}

/* Modify this file as needed*/
int remainingtime;


int main(int agrc, char *argv[])
{
    printf("you are in process %d \n",getpid());
    initClk();
    int shmid, pid,sempho1;
    key_t memo=7893;
    sempho1 = ftok("sem1", 'a');
    // key_t sempho1=7894;

    shmid = shmget(memo, 4096, IPC_CREAT | 0644);

    if (shmid == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    void *shmaddr = shmat(shmid, (void *)0, 0);
    union Semun semun;

    int sem1 = semget(sempho1, 1, 0666 | IPC_CREAT);

    if (sem1 == -1)
    {
        perror("Error in create sem");
        exit(-1);
    }
    
    /*
    semun.val = 0; 
    if (semctl(sem1, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }
   */
    int VAL=semctl(sem1, 0, GETVAL, semun);
    printf("semval %d \n",VAL);
    down(sem1);

    char runTime[10];
    strcpy(runTime,(char *) shmaddr);
    //TODO The process needs to get the remaining time from somewhere
    remainingtime = atoi(runTime);
    printf("runTime From process %d \n",remainingtime);
    int prvtime=getClk();
    while (remainingtime > 0)
    {
        if(getClk()!=prvtime)
        {   
            if(getClk()!= prvtime +1){
                prvtime=getClk();
            }
            else{
                printf("current time from process: %d \n",getClk());
                remainingtime--; 
                prvtime=getClk();
                
                printf("process: remTime %d \n",remainingtime);
                
                //Send the latest remaining time to the scheduler
                char text[10];
                sprintf(text, "%d", remainingtime);  
                strcpy((char *) shmaddr,text);
                
                int VAL= semctl(sem1, 0, GETVAL, semun);
                printf("semval after UUUP in function %d \n",VAL);
                //if wasn't a zero, then it's already upped , no need to up it twice
                // if(VAL == 0){
                //     up(sem1);
                // }
            }
        }
        
    }

    key_t FinsgedrocessSem = 7788;
    int semFinish = semget(FinsgedrocessSem, 1, 0666 | IPC_CREAT);

    if (semFinish == -1)
    {
        perror("Error in create sem FinsgedrocessSem");
        exit(-1);
    }
     semun.val = 0; 
    if (semctl(semFinish, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }
    
    destroyClk(false);
    printf("TRMINAT process with run time %d \n",atoi(runTime));
    //kill(getppid(), SIGUSR1);
    up(semFinish);
    return 0;
}
