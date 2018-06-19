#include <linux/fs.h>        //open(), read(), write(), close()
#include <linux/cdev.h>      //register_chardev_region(), cdev_init()
#include <linux/module.h>
#include <linux/io.h>        //ioremap(), iounmap()
#include <linux/uaccess.h>    //copy_from_user, copy_to_user()
#include <linux/gpio.h>      //request_gpio(), gpio_set_val(), gpio_get_val()
#include <linux/interrupt.h>  //gpio_to_irq(), request_val()
#include <linux/timer.h>      //init_timer, mod_timer, del_timer, add_timer()

#define GPIO_MAJOR 200
#define GPIO_MINOR 0
#define GPIO_DEVICE "gpioled"

//Raspi 0,1 PHYSICAL I/O PERI BASE ADDR
//#define BCM_IO_BASE 0x20000000

// Raspi 2,3 PHYSICAL I/O PERI BASE ADDR
#define BCM_IO_BASE 0x3F000000
#define GPIO_BASE (BCM_IO_BASE + 0x200000)
#define GPIO_SIZE 0xB4

#define GPIO_IN(g)  (*(gpio+((g)/10)) &= (1<<(((g)%10)*3))) 
#define GPIO_OUT(g) (*(gpio+((g)/10)) |= (1<<(((g)%10)*3)))


#define GPIO_LED1 21
#define GPIO_LED2 20
#define BUF_SIZE 100

static char msg[BUF_SIZE] = {0};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DH Kim");
MODULE_DESCRIPTION("Raspberry Pi First Device Driver");

struct cdev gpio_cdev;
static int gpio_open(struct inode *inod, struct file *fil);
static int gpio_close(struct inode *inod, struct file *fil);
static ssize_t gpio_write(struct file *inode, const char *buff, size_t len, loff_t *off);
static ssize_t gpio_read(struct file *inode, char *buff, size_t len, loff_t *off);

static struct timer_list timer;
static struct file_operations gpio_fops = {
	.owner = THIS_MODULE,
	.read  = gpio_read,
	.write = gpio_write,
	.open = gpio_open,
	.release = gpio_close,
};

volatile unsigned int *gpio;

static void timer_func(unsigned long data)
{
  printk(KERN_INFO "timer_func : %ld\n", data);
  
  gpio_set_value(GPIO_LED1, data);
  if(data)
    timer.data=0;
  else
    timer.data=1;
  
  timer.expires = jiffies + msecs_to_jiffies(500);
  add_timer(&timer);
}

  
static int gpio_open(struct inode *inod, struct file *fil)
{
	try_module_get(THIS_MODULE);
	printk(KERN_INFO "GPIO Device opened()\n");
	return 0;
}

static int gpio_close(struct inode *inod, struct file *fil)
{
	module_put(THIS_MODULE);
	printk(KERN_INFO "GPIO Device closed()\n");
	return 0;
}

static ssize_t gpio_write(struct file *inode, const char *buff, size_t len, loff_t *off)
{
  short count;
  memset(msg, 0, BUF_SIZE);
  count = copy_from_user(msg, buff, len);

  if(!strcmp(msg,"0"))
    del_timer_sync(&timer);
  else
  {
    init_timer(&timer);
    timer.function=timer_func; //expire시 호출하는 함수
    timer.data=1;              //timer_func으로 전달하는 인자값
    timer.expires=jiffies+msecs_to_jiffies(500);
    add_timer(&timer);
  }
  
  gpio_set_value(GPIO_LED1,(!strcmp(msg,"0"))?0:1);
  printk(KERN_INFO "GPIO Device Write : %s\n", msg);
  return count;
}

static ssize_t gpio_read(struct file *inode, char *buff, size_t len, loff_t *off)
{
  int count;

  if(gpio_get_value(GPIO_LED1))
	  msg[0] = '1';
  else
	  msg[0] = '0';

  if(gpio_get_value(GPIO_LED2))
	  msg[1] = '1';
  else
	  msg[1] = '0';

  strcat(msg, "_from kernel");
  
  count = copy_to_user(buff, msg, strlen(msg)+1);

  printk(KERN_INFO "GPIO Device Read : %s\n", msg);
  return count;
}

static int __init initModule(void)
{
	dev_t devno;
	unsigned int count;

	int err;
	// 0. 함수 호출 유무를 확인하기 위해
	printk(KERN_INFO "Init gpio_module\n");

	// 1. 문자 디바이스 번호와 이름을 등록
	devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
	printk(KERN_INFO "devno=0x%x\n",devno);
	register_chrdev_region(devno,1,GPIO_DEVICE);

	// 2. 문자 디바이스를 위한 구조체를 초기화 한다.
	cdev_init(&gpio_cdev, &gpio_fops);
	count = 1;

	// 3. 문자 디바이스 추가
	err = cdev_add(&gpio_cdev, devno, count);
	if(err<0)
	{
		printk(KERN_INFO "Error : cdev_add()\n");
		return -1;
	}

	printk(KERN_INFO "'mknod /dev/%s c %d 0'\n", GPIO_DEVICE, GPIO_MAJOR);
	printk(KERN_INFO "'chmod 666 /dev/%s'\n", GPIO_DEVICE);

	//gpio.h
	
	err=gpio_request(GPIO_LED1,"LED1");
	if(err == -EBUSY)
	{
		printk(KERN_INFO "Error gpio_request\n");
		return -1;
	}
	gpio_direction_output(GPIO_LED1,0);

	err=gpio_request(GPIO_LED2,"LED2");
	if(err == -EBUSY)
	{
		printk(KERN_INFO "Error gpio_request\n");
		return -1;
	}

	gpio_direction_output(GPIO_LED2,0);

	return 0;
}

static void __exit cleanupModule(void)
{
  dev_t devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
  
  del_timer_sync(&timer);
  //1. 문자 디바이스의 등록(장치번호, 장치명)을 해제한다.
  unregister_chrdev_region(devno, 1);

  //2. 문자 디바이스의 구조체를 제거한다.
  cdev_del(&gpio_cdev);

  gpio_direction_output(GPIO_LED2,0);
  gpio_free(GPIO_LED2);

  gpio_direction_output(GPIO_LED1,0);
  gpio_free(GPIO_LED1);
  
  //3. 문자 디바이스의 가상번지를 삭제한다.
  if(gpio)
    iounmap(gpio);
  printk(KERN_INFO "Exit gpio_module : Good-Bye\n");
}

//내가 생성하고자 하는 초기화함수 이름을 적어준다.
module_init(initModule);
//내가 생성하고자 하는 종료함수 이름을 적어준다.
module_exit(cleanupModule);
