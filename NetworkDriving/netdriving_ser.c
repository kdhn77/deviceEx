#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <wiringPi.h>
#include "driving.h"

#define BUF_SIZE 1
#define MAX_CLNT 2

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

//?�역변??
int velocity = 0;
int frontwheel = FW_PARA_0;
int tilt = TI_PARA_0;
int camera = CAM_PARA_0;
int fd;
unsigned char buffer[3] = {0};

int reg_read8(unsigned char addr)
{
	int length = 1;
	buffer[0] = addr;

	if(write(fd, buffer,length)!=length)
	{
		printf("Failed to write from the i2c bus\n");
	}
	
	if(read(fd,buffer, length) != length)
	{
		printf("Failed to read from the i2c bus\n");
	}
	//printf("addr[%d] = %d\n", addr, buffer[0]);
	
	return 0;
}

int reg_write8(unsigned char addr, unsigned char data)
{

	int length=2;
	
	buffer[0] = addr;
	buffer[1] = data;

	if(write(fd,buffer,length)!=length)
	{
		printf("Failed to write from the i2c bus\n");
		return -1;
	}
	
	return 0;
}

int reg_read16(unsigned char addr)
{
	unsigned short temp;
	reg_read8(addr);
	temp = 0xff & buffer[0];
	reg_read8(addr+1);
	temp |= (buffer[0] <<8);
	//printf("addr=0x%x, data=%d\n", addr, temp);
	 
	return 0;	
}

int reg_write16(unsigned char addr, unsigned short data)
{
	int length =2;
	reg_write8(addr, (data & 0xff));
	reg_write8(addr+1, (data>>8) & 0xff);
	return 0;
}

int pca9685_restart(void)
{
	int length;
	
	reg_write8(MODE1, 0x00);
	reg_write8(MODE2, 0x04);
	return 0;
}

int pca9685_freq()
{
	int length = 2, freq = 50;
	uint8_t pre_val = (CLOCK_FREQ / 4096 / freq) -1; 
	//printf("prescale_val = %d\n", pre_val);
	 
	reg_write8(MODE1, 0x10);				// OP : OSC OFF
	reg_write8(PRE_SCALE, pre_val);	// OP : WRITE PRE_SCALE VALUE
	reg_write8(MODE1, 0x80);				// OP : RESTART
	reg_write8(MODE2, 0x04);				// OP : TOTEM POLE 
	return 0;
}

void servoOFF(void)
{
		reg_write8(MODE1, 0x10);				// OP : OSC OFF
}

/************************************************************************************************/
int Accelerate(void)
{
  int acc=0;
  if(velocity<0)
    acc=velocity*-1;
  else
    acc=velocity;
  
    reg_write16(LED4_ON_L, 0);
    reg_read16(LED4_ON_L);
    reg_write16(LED4_OFF_L, acc);
    reg_read16(LED4_OFF_L);
    reg_write16(LED5_ON_L, 0);
    reg_read16(LED5_ON_L);
    reg_write16(LED5_OFF_L, acc);
    reg_read16(LED5_OFF_L);
  
  
  printf("Velocity %d\n",velocity);
  return 0;
}

int Wheel_Rightturn(void)
{
  frontwheel+=FW_PARA_STEP;
  if(frontwheel<=FW_PARA_P90)
  {
    reg_write16(LED0_ON_L, 0);
    reg_read16(LED0_ON_L);
    reg_write16(LED0_OFF_L, frontwheel);
    reg_read16(LED0_OFF_L);
  }
  else if(frontwheel>FW_PARA_P90)
  {
    frontwheel=FW_PARA_P90;
  }
  printf("wheel %d\n",frontwheel);
  return 0;
}
int Wheel_Leftturn(void)
{
   frontwheel-=FW_PARA_STEP;
  if(frontwheel>=FW_PARA_M90)
  {
    reg_write16(LED0_ON_L, 0);
    reg_read16(LED0_ON_L);
    reg_write16(LED0_OFF_L, frontwheel);
    reg_read16(LED0_OFF_L);
  }
  else if(frontwheel<FW_PARA_M90)
  {
    frontwheel=FW_PARA_M90;
  }
  printf("wheel %d\n",frontwheel);
  return 0;
}

int Tilt_Leftturn(void)
{
  tilt+=TI_PARA_STEP;
  if(tilt<=TI_PARA_P90)
  {
    reg_write16(LED14_ON_L, 0);
    reg_read16(LED14_ON_L);
    reg_write16(LED14_OFF_L, tilt);
    reg_read16(LED14_OFF_L);
  }
  else if(tilt>FW_PARA_P90)
  {
    tilt=TI_PARA_P90;
  }
  printf("tilt %d\n",tilt);
  return 0;
}
int Tilt_Rightturn(void)
{
  tilt-=TI_PARA_STEP;
  if(tilt>=TI_PARA_M90)
  {
    reg_write16(LED14_ON_L, 0);
    reg_read16(LED14_ON_L);
    reg_write16(LED14_OFF_L, tilt);
    reg_read16(LED14_OFF_L);
  }
  else if(tilt<FW_PARA_M90)
  {
    tilt=TI_PARA_M90;
  }
  printf("tilt %d\n",tilt);
  return 0;
}

int Camera_up(void)
{
  camera+=CAM_PARA_STEP;
  if(camera<=CAM_PARA_P90)
  {
    reg_write16(LED15_ON_L, 0);
    reg_read16(LED15_ON_L);
    reg_write16(LED15_OFF_L, camera);
    reg_read16(LED15_OFF_L);
  }
  else if(camera>CAM_PARA_P90)
  {
    camera=CAM_PARA_P90;
  }
  printf("tilt %d\n",camera);
  return 0;
}
int Camera_down(void)
{
  camera-=CAM_PARA_STEP;
  if(camera>=CAM_PARA_M90)
  {
    reg_write16(LED15_ON_L, 0);
    reg_read16(LED15_ON_L);
    reg_write16(LED15_OFF_L, camera);
    reg_read16(LED15_OFF_L);
  }
  else if(camera<CAM_PARA_M90)
  {
    camera=CAM_PARA_M90;
  }
  printf("tilt %d\n",camera);
  return 0;
}


int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&mutx);
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	return 0;
}
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg);
	int str_len=0;
	char msg;
	int i;
	unsigned short value=2047;
	
	 wiringPiSetup();
	  pinMode (0, OUTPUT) ;		// BCM GPIO17
	  pinMode (2, OUTPUT) ;		// BCM GPIO27
	printf("wiringPi setup\n"); 
	if((fd=open(I2C_DEV, O_RDWR))<0)
	{
		printf("Failed open i2c-1 bus\n");
		return NULL;
	}
  	printf("fd open!\n");
	if(ioctl(fd,I2C_SLAVE,PCA_ADDR)<0)
	{
		printf("Failed to acquire bus access and/or talk to slave\n");
		return NULL;
	}	
	pca9685_restart();
	pca9685_freq();
	printf("pca freq!\n");

	while((str_len=read(clnt_sock, &msg, sizeof(char)))!=0)
	{
		if(msg == 'w')
		{
       printf("w\n");
			if(velocity < MAX_SPEED)
			  velocity+=100;
			if(velocity>0)
			{ // Set CW
			 	digitalWrite (0, LOW) ;
			 	digitalWrite (2, LOW) ;
			}
			Accelerate();
		}
		else if(msg == 'a')
		{
   printf("a\n");
			Wheel_Leftturn();
		}
		else if(msg == 'd')
		{
   printf("d\n");
			Wheel_Rightturn();
		}
		else if(msg == 's')
		{
   printf("s\n");
			if(velocity>MAX_SPEED*-1)
			  velocity-=100;
			if(velocity<0)
			{   // Set CCW
			  	digitalWrite (0, HIGH);
			   	digitalWrite (2, HIGH);
			}
			Accelerate();
		}
		else if(msg == 'i')
		{
   printf("i\n");
			 Camera_up();
		}
		else if(msg == 'j')
		{
   printf("j\n");
			Tilt_Leftturn();
		}
		else if(msg == 'k')
		{
   printf("k\n");
			Camera_down();
		}
		else if(msg == 'l')
		{
   printf("l\n");
			Tilt_Rightturn();
		}
		else if(msg == 27)
		{
			printf("exit\n");
			break;
		}
	}
	servoOFF();
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}
void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}


void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
