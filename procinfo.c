#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"


int main(int argc, char *argv[])
{
    if(argc <=1)
    {
        printf(1,"Not Enough Arguments\n");
        exit(-1);
    }
    int pid = atoi(argv[1]);
    int status  =  procinfo(pid);
    if(status ==-1)
        printf(1,"pid Not Found\n");
    exit(status);
}