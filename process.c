#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char *argv[])
{
    initClk();
     int prvtime=getClk();
    //TODO The process needs to get the remaining time from somewhere
   // remainingtime = table.Procsess[0].runningtime;
    while (remainingtime > 0)
    {
        if(getClk()!=prvtime)
        { remainingtime--; }
    }

    destroyClk(false);

    return 0;
}
