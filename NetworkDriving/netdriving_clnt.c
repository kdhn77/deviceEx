#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <termios.h>
#include <termio.h>

#define BUF_SIZE 2
	
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
int kbhit(void);
int getch(void);
	
int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	if(argc!=3) {
		printf("Usage : %s <IP> <port> \n", argv[0]);
		exit(1);
	 }
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock);  
	return 0;
}
	
void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg);
	int key;
	char send_msg[BUF_SIZE];
	while(1) 
	{
		if(kbhit())
		{
			key=getch();
			if(key==27) //'Esc'
			{
				write(sock, send_msg, 1);
				close(sock);
				exit(0);
			}
			switch(key)
		      	{
		      	case 119:    //'w'
				printf("w\n");
				sprintf(send_msg,"w");
				write(sock, send_msg, 1);
				break;
		      	case 97:     //'a'
				printf("a\n");
				sprintf(send_msg,"a");
				write(sock, send_msg,1);
				break;
		      	case 100:    //'d'
				printf("d\n");
				sprintf(send_msg,"d");
				write(sock, send_msg, 1);
				break;
		      	case 115:    //'s'
				printf("s\n");
				sprintf(send_msg,"s");
				write(sock, send_msg, 1);
				break;
		     	case 105:    //'i'
				printf("i\n");
				sprintf(send_msg,"i");
				write(sock, send_msg, 1);
				break;
		     	case 106:    //'j'
				printf("j\n");
				sprintf(send_msg,"j");
				write(sock, send_msg, 1);
				break; 
		     	case 107:    //'k'
				printf("k\n");
				sprintf(send_msg,"k");
				write(sock, send_msg, 1);
				break;
		     	case 108:    //'l'
				printf("l\n");
				sprintf(send_msg,"l");
				write(sock, send_msg, 1);
				break;
			}
		}
	}
	return NULL;
}
	
void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg);
	char recv_msg[BUF_SIZE];
	int str_len;
	while(1)
	{
		str_len=read(sock, recv_msg, sizeof(recv_msg));
		if(str_len==-1) 
			return (void*)-1;
		recv_msg[str_len]=0;
		fputs(recv_msg, stdout);
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}

int getch(void) 
{ 
  int ch; 
  struct termios old; 
  struct termios new; 
  tcgetattr(0, &old); 
  new = old; 
  new.c_lflag &= ~(ICANON|ECHO); 
  new.c_cc[VMIN] = 1; 
  new.c_cc[VTIME] = 0; 
  
  tcsetattr(0, TCSAFLUSH, &new); 
  ch = getchar(); 
  tcsetattr(0, TCSAFLUSH, &old); return ch; 
}
