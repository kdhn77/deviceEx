#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdint.h>
#include <termios.h>
#include <termio.h>
#include <wiringPi.h>
#include "driving.h"

int velocity = 0;
int frontwheel = FW_PARA_0;
int tilt = TI_PARA_0;
int camera = CAM_PARA_0;
int fd;
unsigned char buffer[3] = {0};

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

/* MAIN */			
int main(void)
{
	int i;
	unsigned short value=2047;
  int key;
  
  wiringPiSetup();
  pinMode (0, OUTPUT) ;		// BCM GPIO17
  pinMode (2, OUTPUT) ;		// BCM GPIO27
 
	if((fd=open(I2C_DEV, O_RDWR))<0)
	{
		printf("Failed open i2c-1 bus\n");
		return -1;
	}

	if(ioctl(fd,I2C_SLAVE,PCA_ADDR)<0)
	{
		printf("Failed to acquire bus access and/or talk to slave\n");
		return -1;
	}	
	pca9685_restart();
	pca9685_freq();

  while(1)
  {
    if(kbhit())
    {
      key=getch();
      if(key==27)  //esc
        break;
      switch(key)
      {
      case 119:    //'w'
        if(velocity < MAX_SPEED)
          velocity+=100;
        if(velocity>0)
        { // Set CW
         	digitalWrite (0, LOW) ;
         	digitalWrite (2, LOW) ;
        }
        Accelerate();
        break;
      case 97:     //'a'
        Wheel_Leftturn();
        break;  
      case 100:    //'d'
        Wheel_Rightturn();
        break;
      case 115:    //'s'
        if(velocity>MAX_SPEED*-1)
          velocity-=100;
        if(velocity<0)
        {   // Set CCW
          	digitalWrite (0, HIGH);
           	digitalWrite (2, HIGH);
        }
        Accelerate();
        break;
      case 105:    //'i'
        Camera_up();
        break;
      case 106:    //'j'
        Tilt_Leftturn();
        break;  
      case 107:    //'k'
        Camera_down();
        break;
      case 108:    //'l'
        Tilt_Rightturn();
        break;
        
      default:
        break;
      }
    }
  }
  printf("\n");
  printf("exit\n");
  servoOFF();
	return 0;
}
