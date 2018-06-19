#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdint.h>

#define I2C_DEV 			"/dev/i2c-1"  //디바이스 경로
#define CLOCK_FREQ		25000000.0    //25MHz 데이터시트에 정의
#define PCA_ADDR			0x40          //I2C addr 01000000 이므로
#define LED_STEP			200

// Register Addr
#define MODE1					0x00    //데이터시트에 정의된 16진수 주소들
#define MODE2					0x01
#define LED15_ON_L		0x42
#define LED15_ON_H		0x43
#define LED15_OFF_L		0x44
#define LED15_OFF_H		0x45
#define PRE_SCALE			0xFE

int fd;
unsigned char buffer[3] = {0};

int reg_read8(unsigned char addr)
{
	int length = 1;
	buffer[0] = addr;

	if(write(fd, buffer,length)!=length)
	{
		printf("Failed to read from the i2c bus\n");
	}
	
	if(read(fd,buffer, length) != length)
	{
		printf("Failed to read from the i2c bus\n");
	}
	printf("addr[%d] = %d\n", addr, buffer[0]);
	
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
	printf("addr=0x%x, data=%d\n", addr, temp);
	 
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
	int length = 2, freq = 60;
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


int main(void)
{
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
	led_on(value);
	
	return 0;
}
