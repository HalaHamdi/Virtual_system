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
int getTotalLines(char fileName[], FILE *fp);
void storeProcessData(struct processData processArray[],FILE *fp,int Totallines);

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);

    // * taking terminal parameters 
    //  Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    char fileName[50];  
    int AlgorithmNmber;
    int quantum=-1;
    key_t schedulerKey=1234;
    int pid[2],stat_loc;
    int msgq_id;
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
   
    getChildren(pid); //sometimes after clck terminates still running
    printf("clk id= %d ,scheduler id=%d \n",pid[0],pid[1]);

    //  initialize clock.
    initClk();


    // Create a data structure for processes and provide it with its parameters.
    key_t scheduler=ftok("schedulerKey",'S');
    msgq_id=msgget(scheduler,0666|IPC_CREAT);
   printf("msgq_id in pgenerator %d \n",msgq_id);

    if(msgq_id==-1){
        printf("error in creating msgQueue \n");
         exit(-1);}


    // 6. Send the Initiator which contains the number of processes , Algorithm number adn any extra parameter (e.g :quantum).

    Initiator.arrivaltime=Totallines-1;  //the TotalProcesses will be sent in the arrivaltime  
    Initiator.id=AlgorithmNmber;         //the AlgorithmNmber will be sent in the id  
    Initiator.priority=quantum;         //the quantum will be sent in the priority BUT NEEDS  TO BE FLOAT  
    msgsnd(msgq_id,&Initiator,sizeof(&Initiator),!IPC_NOWAIT);
    printf("sent Initiator \n");

    //send each process when its time comes
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
        execl("./clk.o","clk.o",NULL); 
        // need to ensure that it is in clk.o format 
        //need to generalize the bin directory
    }

    pid[1]=fork();
   
    if(pid[1]==-1){perror("error in forking scheduler");}
    else if (pid[1]==0){

        execl("./scheduler.o","scheduler.o",NULL); 

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
        fscanf(fp,"%d",&p.id);
        fscanf(fp,"%d",&p.arrivaltime);
        fscanf(fp,"%d",&p.runningtime);
        fscanf(fp,"%d",&p.priority);
        processArray[i]=p;
    }
    fclose(fp);
}