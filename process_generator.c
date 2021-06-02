#include "headers.h"
#include<string.h>

struct processData
{
    int arrivaltime; //total processes number for first item send to scheduler
    int priority;    //quantum is send if found
    int runningtime;
    int id;      //id of the algorithm to execute
};



void clearResources(int);
void getChildren(int arr[]);
int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);

    // * taking terminal parameters 
    char fileName[50];  
    strcpy(fileName,argv[1]);
    int AlgorithmNmber=atoi(argv[3]);
    int quantum=-1;
    if(argc>4){
        quantum=atoi(argv[5]);
    }

    // TODO Initialization
    key_t schedulerKey=1234;
    // 1. Read the input files.
    FILE *fp=fopen(fileName,"r");
    if(fp==NULL){ printf("File \"%s\" does not exist!!!\n",fileName); exit(-1);}
    char ch;
    int Totallines=0;
    //getting number of processes made in text file.
    //by counting the number of lines
    while((ch=fgetc(fp))!=EOF){
        if(ch=='\n'){Totallines++;}
    }
    struct processData processArray[Totallines-1];
    //reset the pointer to the begining of file
    fseek(fp, 0, SEEK_SET);
    char word[20];
    //skip the first line 
    for(int i=0;i<4;i++){fscanf(fp,"%s",word);}
    //store the 
    for(int i=0;i<Totallines-1;i++){
        struct processData p;
        fscanf(fp,"%d",&p.id);
        fscanf(fp,"%d",&p.arrivaltime);
        fscanf(fp,"%d",&p.runningtime);
        fscanf(fp,"%d",&p.priority);
        processArray[i]=p;
    }
    fclose(fp);
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.

    // 3. Initiate and create the scheduler and clock processes.
    int pid[2],stat_loc;
    getChildren(pid); //sometimes after clck terminates still running
    printf("clk id= %d ,scheduler id=%d \n",pid[0],pid[1]);
    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.

   int msgq_id=msgget(schedulerKey,0666|IPC_CREAT);
      printf("between msgqueue  \n");

   if(msgq_id==-1){printf("error in creating msgQueue \n"); exit(-1);}
          printf("after msgqueue  \n");

    // 6. Send the information to the scheduler at the appropriate time.
   struct processData Initiator;
    Initiator.arrivaltime=Totallines-1; 
    Initiator.id=AlgorithmNmber;
    Initiator.priority=quantum;
    msgsnd(msgq_id,&Initiator,sizeof(&Initiator),!IPC_NOWAIT);
    int counter=0;
    while(true){
        
        if(getClk()>=processArray[counter].arrivaltime){
        printf("current time %d \n",getClk());
        msgsnd(msgq_id,&processArray[counter],sizeof(&processArray[counter]),!IPC_NOWAIT);
        counter++;
        if(counter==Totallines){break;}
        }
    }
    

    
    // 7. Clear clock resources
   destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    killpg(getpgrp(),SIGKILL);
    signal(SIGINT, clearResources);

}
void getChildren(int pid[]){
   pid[0]=fork();
    if(pid[0]==-1){perror("error in forking clock");}
    else if (pid[0]==0){
        printf("clock pid %d \n",getpid());
        execl("./clk.o","clk.o",NULL); 
        // need to ensure that it is in clk.o format 
        //need to generalize the bin directory
    }

    pid[1]=fork();
    if(pid[1]==-1){perror("error in forking scheduler");}
    else if (pid[1]==0){
        printf("scheduler pid  %d \n",getpid());
        execl("./scheduler.o","scheduler.o",NULL); 
        
    }
}
