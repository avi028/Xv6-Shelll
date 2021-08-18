#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"


char buf[512];

int executeCommands(int fd)
{
    int n;

    while((n = read(fd, buf, sizeof(buf))) > 0) 
    {
        // int pid=0,status=-1;
        // if((pid=fork()) == 0)
        // {
        // runcmd(parsetokenlist(parseBuf(&buf[0],strlen(buf))));
        // }
        // printf(1,"pid %d forked\n",pid);
        // if(pid == wait(&status)){printf(1,"pid %d exit %d\n",pid,status);}
     //   parseBuf(buf,strlen(buf));
    }
    if(n < 0)
    {
        printf(1, "executeCommands: read error\n");
        return -1;
    }
    return 1;
}

int main(int argc,char * argv[])
{
    if(argc <=1)
    {
        printf(1,"Not Enough Arguments \n");
        exit(-1);
    }   
    int fd; 
    if((fd = open(argv[1], 0)) < 0){
      printf(1, "executeCommands : cannot open %s\n", argv[1]);
      exit(-1);
    }
    int status = executeCommands(fd);
    close(fd);
    exit(status);
}
