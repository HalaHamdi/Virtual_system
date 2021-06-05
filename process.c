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
    printf("Hiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii you are in process %d \n",getpid());
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
    //printf("Hiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii you are in process2 \n");
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
            printf("current time from process: %d \n",getClk());
            remainingtime--; 
            prvtime=getClk();
        }
    }

    destroyClk(false);
    printf("TRMINAT process with run time %d \n",atoi(runTime));
    kill(getppid(), SIGUSR1);
    return 0;
}
