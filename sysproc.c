#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  int status;
  if(argint(0, &status) < 0)
    return -1;
  exit(status);
  return 0;  // not reached
}

int
sys_wait(void)
{
  int *p;

  if(argptr(0, (void*)&p, sizeof(int))<0){
      return -1;
  }
  return wait(p);
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


int 
sys_exec_shell(void)
{
  char * buf;
  if(argstr(0, &buf)<0)
    return -1;
 return exec_shell(buf);   
}

int 
sys_helloWorld(void)
{
  cprintf("Hello World\n");
  return 1;
}

int
sys_ps(void)
{
  ps();
  return 1;
} 

int
sys_procinfo(void)
{
  int pid;
  if(argint(0, &pid) < 0)
    return -1;
  return procinfo(pid);
}

int
sys_numOpenFiles(void)
{
  int itr;
  int count=0;
  if(myproc()==0)
    return -1;
  else
  {
    for(itr=0;itr<NOFILE;itr++)
    {
      if(myproc()->ofile[itr])
        count++;
    }
    return count;
  }
}

int
sys_memAlloc(void)
{
  if(myproc()==0)
    return -1;
  else
    return myproc()->total_mem_alloc;
}

int
sys_getprocesstimedetails(void)
{
  if(myproc()==0)
    return -1;
  else
  {
    cprintf("processCreationDateTime: %d:%d:%d %d:%d:%d\n",myproc()->processCreationDateTime.second\
                                                        ,myproc()->processCreationDateTime.minute\
                                                        ,myproc()->processCreationDateTime.hour\
                                                        ,myproc()->processCreationDateTime.day\
                                                        ,myproc()->processCreationDateTime.month\
                                                        ,myproc()->processCreationDateTime.year);

    cprintf("processLastContextSwitchedOutDateTime:  %d:%d:%d %d:%d:%d\n",myproc()->processLastContextSwitchedOutDateTime.second\
                                                                        ,myproc()->processLastContextSwitchedOutDateTime.minute\
                                                                        ,myproc()->processLastContextSwitchedOutDateTime.hour\
                                                                        ,myproc()->processLastContextSwitchedOutDateTime.day\
                                                                        ,myproc()->processLastContextSwitchedOutDateTime.month\
                                                                        ,myproc()->processLastContextSwitchedOutDateTime.year);

    cprintf("processLastContextSwitchedInDateTime:  %d:%d:%d %d:%d:%d\n",myproc()->processLastContextSwitchedInDateTime.second\
                                                                      ,myproc()->processLastContextSwitchedInDateTime.minute\
                                                                      ,myproc()->processLastContextSwitchedInDateTime.hour\
                                                                      ,myproc()->processLastContextSwitchedInDateTime.day\
                                                                      ,myproc()->processLastContextSwitchedInDateTime.month\
                                                                      ,myproc()->processLastContextSwitchedInDateTime.year );

  }
  return 1;
}