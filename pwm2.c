#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdint.h>

#define I2C_DEV 			"/dev/i2c-1"
#define CLOCK_FREQ		25000000.0
#define PCA_ADDR			0x40
#define LED_STEP			200
#define PARAM_M90		258
#define PARAM_0			358
#define PARAM_P90		458
#define PARAM_STEP  10

#define ANGLE_M90		0
#define ANGLE_0			1
#define ANGLE_P90		2


// Register Addr
#define MODE1					0x00
#define MODE2					0x01
#define LED0_ON_L			0x06
#define LED0_ON_H			0x07
#define LED0_OFF_L		0x08
#define LED0_OFF_H		0x09

#define LED4_ON_L			0x16
#define LED4_ON_H			0x17
#define LED4_OFF_L		0x18
#define LED4_OFF_H		0x19
#define LED5_ON_L			0x1A
#define LED5_ON_H			0x1B
#define LED5_OFF_L		0x1C
#define LED5_OFF_H		0x1D


#define LED1_ON_L			0x0A
#define LED1_ON_H			0x0B
#define LED1_OFF_L		0x0C
#define LED1_OFF_H		0x0D

#define LED15_ON_L		0x42
#define LED15_ON_H		0x43
#define LED15_OFF_L		0x44
#define LED15_OFF_H		0x45
#define ALL_LED_ON_L    0xFA
#define ALL_LED_ON_H	0xFB
#define ALL_LED_OFF_L	0xFC
#define ALL_LED_OFF_H	0xFD


#define PRE_SCALE			0xFE

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
	printf("prescale_val = %d\n", pre_val);
	 
	reg_write8(MODE1, 0x10);				// OP : OSC OFF
	reg_write8(PRE_SCALE, pre_val);	// OP : WRITE PRE_SCALE VALUE
	reg_write8(MODE1, 0x80);				// OP : RESTART
	reg_write8(MODE2, 0x04);				// OP : TOTEM POLE 
	return 0;
}

int led_on(unsigned short value)
{
	unsigned short time_val=4095;
	char key;
	while(key != 'b')
	{
		printf("key insert :");
		key=getchar();
		if(key=='a')
		{
			if(value< 3800)
			{
				value += LED_STEP;
				reg_write16(LED15_ON_L, time_val - value);
				reg_read16(LED15_ON_L);
				reg_write16(LED15_OFF_L, time_val);
				reg_read16(LED15_OFF_L);
			}
			else
			{
				printf("Overflow\n");
			}
		}
		else if(key=='s')
		{
			if(value > LED_STEP)
			{
				value -= LED_STEP;
				reg_write16(LED15_ON_L, time_val - value);
				reg_read16(LED15_ON_L);
				reg_write16(LED15_OFF_L, time_val);
				reg_read16(LED15_OFF_L);
			}
			else
			{
				printf("Underflow\n");
			}		
		}
	}
	return 0;	
				
}

void servoOFF(void)
{
		reg_write8(MODE1, 0x10);				// OP : OSC OFF
}

int blinkLED(void)
{
	int i;
	unsigned short value;
	unsigned short max=4095;
	while(1)
	{
		{
			for(i=0;i<max;i+=5)
			{
				if(i>1024)
					i+=15;
				value = i;
				reg_write16(LED15_ON_L, max - value);
				reg_read16(LED15_ON_L);
				reg_write16(LED15_OFF_L, max);
				reg_read16(LED15_OFF_L);
				usleep(20);
			}
			
			for(i=0;i<max;i+=5)
			{
				if(i<3072)
					i+=15;
				value = i;
				reg_write16(LED15_ON_L, value);
				reg_read16(LED15_ON_L);
				reg_write16(LED15_OFF_L, max);
				reg_read16(LED15_OFF_L);
				usleep(20);
			}
		}
	}
	return 0;	
				
}

int testServo(int angle)
{
	switch(angle)
	{
		case ANGLE_M90:
			reg_write16(LED0_ON_L, 0);
			reg_read16(LED0_ON_L);
			reg_write16(LED0_OFF_L, PARAM_M90);
			reg_read16(LED0_OFF_L);
			break;
			
		case ANGLE_0:
			reg_write16(LED0_ON_L, 0);
			reg_read16(LED0_ON_L);
			reg_write16(LED0_OFF_L, PARAM_0);
			reg_read16(LED0_OFF_L);
			break;
			
		case ANGLE_P90:
			reg_write16(LED0_ON_L, 0);
			reg_read16(LED0_ON_L);
			reg_write16(LED0_OFF_L, PARAM_P90);
			reg_read16(LED0_OFF_L);
			break;
		
		default:
			reg_write16(LED0_ON_L, 0);
			reg_read16(LED0_ON_L);
			reg_write16(LED0_OFF_L, PARAM_0);
			reg_read16(LED0_OFF_L);
	}		
	return 0;
}

int testServo2(void)
{
  int value=PARAM_0;
	char key;
  printf("left : a right : d left90 : q right90 : e 0 : s break : b\n");
	while(key != 'b')
	{
		printf("key insert :");
		key=getchar();
		if(key=='a')
		{
			//if(value >= PARAM_M90)
			//{
        printf("value : %d\n",value);
				value -= PARAM_STEP;
			  reg_write16(LED0_ON_L, 0);
			  reg_read16(LED0_ON_L);
			  reg_write16(LED0_OFF_L, value);
			  reg_read16(LED0_OFF_L);
			//}
			/*
      else
			
      {
				printf("Overflow\n");
			}
      */
		}
		else if(key=='d')
		{
			//if(value <= PARAM_P90)
			//{
        printf("value : %d\n",value);
				value += PARAM_STEP;
				reg_write16(LED0_ON_L, 0);
			  reg_read16(LED0_ON_L);
			  reg_write16(LED0_OFF_L, value);
			  reg_read16(LED0_OFF_L);
			//}
      /*
			else
			{
				printf("Underflow\n");
			}
      */		
		}
    else if(key=='q')
    {
   	  reg_write16(LED0_ON_L, 0);
			reg_read16(LED0_ON_L);
			reg_write16(LED0_OFF_L, PARAM_M90);
			reg_read16(LED0_OFF_L);
    }
   	else if(key=='e')
    {
   	  reg_write16(LED0_ON_L, 0);
			reg_read16(LED0_ON_L);
			reg_write16(LED0_OFF_L, PARAM_P90);
			reg_read16(LED0_OFF_L);
    }
    else if(key=='s')
    {
      reg_write16(LED0_ON_L, 0);
			reg_read16(LED0_ON_L);
			reg_write16(LED0_OFF_L, PARAM_0);
			reg_read16(LED0_OFF_L);
    }
	}
	return 0;	
}

int test_rearwheel()
{
  int i;
  unsigned short value=2048;
    while(1)
    {
      printf("value : %d\n",value);
      reg_write16(LED4_ON_L, 0);
      reg_read16(LED4_ON_L);
      reg_write16(LED4_OFF_L, value);
      reg_read16(LED4_OFF_L);
      reg_write16(LED5_ON_L, 0);
      reg_read16(LED5_ON_L);
      reg_write16(LED5_OFF_L, value);
      reg_read16(LED5_OFF_L);
      value++;
      if(value>=4095)
        break;
    }
  return 0;
}
			
int main(void)
{
	int i;
	unsigned short value=2047;
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

  /*
	for(i=0;i<5;i++)
	{
		testServo(ANGLE_M90);
		sleep(1);

		testServo(ANGLE_0);
		sleep(1);
		
		testServo(ANGLE_P90);
		sleep(1);
		
		testServo(ANGLE_0);
		sleep(1);
	}
 */
	
  //testServo2();
  
  test_rearwheel();
	servoOFF();

	return 0;
}
