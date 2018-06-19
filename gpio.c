#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#define BUF_SIZE 100

struct timeval t_val1, t_val2;
int swflag = 0;

void signal_handler(int signum)
{
  //printf("user app : signal is catched\n");
  if(signum==SIGIO && swflag==0)
  {
    swflag=1;
    gettimeofday(&t_val1,NULL);
    //printf("SIGIO START\n");
   //exit(1);
  }
  else if(signum==SIGIO && swflag==1)
  {
    swflag=0;
    //printf("SIGIO END\n");
   //exit(1);
   }
   
   if(signum==SIGUSR1)
  {
    //printf("SIGUSR1\n");
    if(t_val2.tv_usec-t_val1.tv_usec<0)
      printf(" %ld.%d\n",t_val2.tv_sec-t_val1.tv_sec,(t_val2.tv_usec-t_val1.tv_usec)*-1);
    printf(" %ld.%d\n",t_val2.tv_sec-t_val1.tv_sec,t_val2.tv_usec-t_val1.tv_usec);  
   //exit(1);
   }
}

int main(int argc, char **argv)
{
  char buf[BUF_SIZE];
  int count;
  int fd = -1;
 
  memset(buf, 0, BUF_SIZE);

  signal(SIGIO,signal_handler);
  signal(SIGUSR1,signal_handler);
  
  printf("GPIO Set : %s\n", argv[1]);
 
  fd = open("/dev/gpioled", O_RDWR); 
  while(fd<0)
  {
	  if(errno == ENOENT)
	  {
		  system("sudo mknod /dev/gpioled c 200 0");
		  system("sudo chmod 666 /dev/gpioled");
	  }
	  else if(errno==ENXIO)
	  {
		  system("sudo insmod gpio_signal.ko");
	  }

	  fd = open("/dev/gpioled", O_RDWR);
  }
  
  sprintf(buf,"%s:%d",argv[1],getpid());
  count = write(fd, buf, strlen(buf));
  
  if(count<0)
    printf("Error : write()\n");
  
  while(1)
  {
    if(swflag==1)
    {
      gettimeofday(&t_val2,NULL);
      if(t_val2.tv_usec-t_val1.tv_usec<0)
        printf(" %ld.%d\r",t_val2.tv_sec-t_val1.tv_sec,(t_val2.tv_usec-t_val1.tv_usec)*-1);
      printf(" %ld.%d\r",t_val2.tv_sec-t_val1.tv_sec,t_val2.tv_usec-t_val1.tv_usec);
      
      /*
      if(swflag==0)
        break;
        */
    }
  }

  count = read(fd, buf, 20);
  printf("Read data : %s\n", buf);
  close(fd);
  printf("/dev/gpioled closed\n");
  return 0;
}
