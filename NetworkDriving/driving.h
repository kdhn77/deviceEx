#define I2C_DEV 			"/dev/i2c-1"
#define CLOCK_FREQ		25000000.0
#define PCA_ADDR			0x40

//Front wheel servo Register Addr & Parameter 
#define FW_PARA_M90		248
#define FW_PARA_0			358
#define FW_PARA_P90		448
#define FW_PARA_STEP    10

#define LED0_ON_L			0x06
#define LED0_ON_H			0x07
#define LED0_OFF_L		0x08
#define LED0_OFF_H		0x09

//Rear wheel Register Addr
#define LED4_ON_L			0x16
#define LED4_ON_H			0x17
#define LED4_OFF_L		0x18
#define LED4_OFF_H		0x19

#define LED5_ON_L			0x1A
#define LED5_ON_H			0x1B
#define LED5_OFF_L		0x1C
#define LED5_OFF_H		0x1D

#define MAX_SPEED     4000
//Tilt servo Register Addr & Parameter
#define TI_PARA_M90		108
#define TI_PARA_0			318
#define TI_PARA_P90		528
#define TI_PARA_STEP    20

#define LED14_ON_L		0x3E
#define LED14_ON_H		0x3F
#define LED14_OFF_L		0x40
#define LED14_OFF_H		0x41

//Camera servo Register Addr & Parameter
#define CAM_PARA_M90		278
#define CAM_PARA_0			318
#define CAM_PARA_P90		528
#define CAM_PARA_STEP    20

#define LED15_ON_L		0x42
#define LED15_ON_H		0x43
#define LED15_OFF_L		0x44
#define LED15_OFF_H		0x45

//etc Register Addr
#define MODE1					0x00
#define MODE2					0x01

#define ALL_LED_ON_L    0xFA
#define ALL_LED_ON_H	0xFB
#define ALL_LED_OFF_L	0xFC
#define ALL_LED_OFF_H	0xFD

#define PRE_SCALE			0xFE
