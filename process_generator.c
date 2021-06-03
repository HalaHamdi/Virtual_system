#include "headers.h"
#include<string.h>


int msgq_id;

struct processData
{
    long mtype;
    int processinfo[4];
    //int arrivaltime; //total processes number for first item send to scheduler
    //int priority;    //quantum is send if found
    //int runningtime;
    //int id;      //id of the algorithm to execute
};



void clearResources(int);
void getChildren(int arr[]);
int getTotalLines(char fileName[], FILE *fp);
void storeProcessData(struct processData processArray[],FILE *fp,int Totallines);

int main(int argc, char *argv[])
{

    signal(SIGINT, clearResources);
     printf("Process generator id= %d  \n",getpid());
    // * taking terminal parameters 
    //  Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    char fileName[50];  
    int AlgorithmNmber;
    int quantum=-1;
    key_t schedulerKey=1234;
    int pid[2],stat_loc;
    struct processData Initiator;
   
    strcpy(fileName,argv[1]);
    AlgorithmNmber=atoi(argv[3]);
    if(argc>4){
        quantum=atoi(argv[5]);
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


    // 6. Send the Initiator which contains the number of processes , Algorithm number adn any extra parameter (e.g :quantum).
    printf("num of process from generator %d \n",Totallines-1);
    //Initiator.arrivaltime=Totallines-1; //the TotalProcesses will be sent in the arrivaltime  
    //Initiator.id=AlgorithmNmber;         //the AlgorithmNmber will be sent in the id  
    //Initiator.priority=quantum;         //the quantum will be sent in the priority BUT NEEDS  TO BE FLOAT  
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
    while(true){
        
        if(getClk()>=processArray[counter].processinfo[1]){
        printf("current time from generator %d \n",getClk());
        int send_val=msgsnd(msgq_id,&processArray[counter],sizeof(processArray[counter].processinfo),!IPC_NOWAIT);
        if(send_val == -1)
            perror("Errror in send");
        counter++;
        if(counter==Totallines-1){break;}
        }
    }

  /*  printf("ftimeeeeeeee %d \n",processArray[Totallines-1].processinfo[1]+processArray[Totallines-1].processinfo[2]);
    while(true){
        printf("current time %d \n",getClk());
        if(processArray[Totallines-1].processinfo[1]+processArray[Totallines-1].processinfo[2]<getClk()){
            break;
        }
    }*/
    //will wait till the scheduler finishes  sid = waitpid(pid[1],&stat_loc,0); it is better 
    wait(&stat_loc);
    
    
    // 7. Clear clock resources
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
        // need to ensure that it is in clk.o format 
        //need to generalize the bin directory
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
    //store the 
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