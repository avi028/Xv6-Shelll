#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
    
void test_helloworld(void);
void testcase_openFiles(void);
void testcase_mem(int );
    
int main(int argc, char *argv[])
{
  test_helloworld();
  testcase_openFiles();
  testcase_mem(1000);
  getprocesstimedetails();
  exit(0);
}
 
void test_helloworld(void)
{
  helloWorld();
}

void testcase_openFiles(void)
{
   printf(1, "Total Number of Open Files: %d\n", numOpenFiles());
    int fd;
    fd = open("backup", O_CREATE | O_RDWR);
    printf(1, "Total Number of Open Files: %d\n", numOpenFiles());
    close(fd);
    printf(1, "Total Number of Open Files: %d\n", numOpenFiles());
}

void testcase_mem(int in)
{
      printf(1,"Memory allocated till now: %d bytes\n", memAlloc());
  sleep(in);

  char* x = sbrk(1024);
  printf(1,"Memory allocated till now: %d bytes\n", memAlloc());
  
  //free(x);
  char* y = sbrk(4096);
  printf(1,"Memory allocated till now: %d bytes\n", memAlloc());


  printf(1,"Memory allocated till now: %d bytes\n", memAlloc());
  

  char* z = sbrk(-10);
  printf(1,"Memory allocated till now: %d bytes\n", memAlloc());

  char* a = sbrk(-20000);
  printf(1,"Memory allocated till now: %d bytes\n", memAlloc());

  free(x);
  free(y);
  free(z);  
  free(a);
}

