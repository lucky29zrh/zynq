/*
修改说明： 该代码实现的是ioctl实现点灯、灭灯功能。
修改之处：1) ./ioctldrv_test 1 灯亮，其他情况，灯灭
         2）主要学习ioctl 幻数知识点
         3）request_mem_region放在init中，ioremap放在open中，实现每次打开文件时进行映射
         4) 本代码使用新的API进行设备号的申请和释放
         5）注意设备文件、设备类、设备节点的区分
https://blog.csdn.net/deep_l_zh/article/details/48550127
*/

#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/fs.h>  
#include <asm/irq.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/cdev.h>  
#include <linux/device.h>  		  //包含了device、class 等结构的定义
#include <asm/io.h>               //包含了ioremap、iowrite等内核访问IO内存等函数
#include <linux/uaccess.h>        //包含了copy_to_user、copy_from_user等

 
#define LED_MAGIC  'L'  
#define LED_ON     _IO(LED_MAGIC,0)  
#define LED_OFF    _IO(LED_MAGIC,1)

#define DEVICE_NAME          "myled"
#define CLASS_NAME           "zynqebb" 
  


struct cdev gpio_cdev;            //定义设备的结构体 
dev_t gpio_devno; 				  //动态分配设备号
#define NUMBER_OF_DEVICES 1 

static struct class*  gpio_driver_class = NULL;  
static struct device* gpio_driver_device = NULL; 


/* XGPIO Physical address */
#define XGPIO_PHY_ADDR 0x41200000      //This Address is based SDK 
/* Register Offset Definitions */
#define XGPIO_DATA_OFFSET   (0x0)	/* Data register  */
#define XGPIO_TRI_OFFSET    (0x4)	/* I/O direction register  */

volatile unsigned long *Gpio_DIR = NULL;
volatile unsigned long *Gpio_DATA = NULL;
unsigned long axi_gpio_virt_addr = 0;        //AXI_GPIO 虚拟地址


int axi_gpio_open(struct inode *inode,struct file *filep) 
{  
   
    axi_gpio_virt_addr = (unsigned long)ioremap(XGPIO_PHY_ADDR, sizeof(u32));
    //将模块的物理地址映射到虚拟地址上  
	printk( "ioremap called: phys %#08x -> virt %#08x\n",XGPIO_PHY_ADDR, axi_gpio_virt_addr );
	//指定需要操作的寄存器的地址
	Gpio_DIR =  (unsigned long *)(axi_gpio_virt_addr + XGPIO_TRI_OFFSET);
	Gpio_DATA = (unsigned long *)(axi_gpio_virt_addr + XGPIO_DATA_OFFSET);

    return 0;  
} 


static ssize_t axi_gpio_write(struct file * file, const char * buf, size_t count,loff_t *off)
{
	
    return 0;  
} 

//2.6.26内核代码之后
static int axi_gpio_ioctl(struct file *filep,unsigned int cmd,unsigned long arg) 
{ 
	switch(cmd)  
    {  
       case LED_ON:  
           iowrite32(0x0000000F,Gpio_DATA); //设置AXI GPIO的方向输出全为高  
           return 0;  
        
       case LED_OFF:  
           iowrite32(0x00000000,Gpio_DATA); //设置AXI GPIO的方向输出全为低 
           return 0 ;  
        
       default:  
           return -EINVAL;  
    
    }  

	return 0;
}
static struct file_operations led_fops=   // 对应的对led的操作  
{  
    .open	= axi_gpio_open,  
	.write  = axi_gpio_write,
	.unlocked_ioctl= axi_gpio_ioctl,  
	.owner = THIS_MODULE,  
};



static int __init axi_gpio_driver_module_init(void)
{  
	int ret;  

    //次设备号为0，1个设备，设备名称为zynqgpio，把动态分配得到的设备号给gpio_devno  
	ret = alloc_chrdev_region(&gpio_devno, 0, NUMBER_OF_DEVICES, "zynqgpio");   // /proc/devices/

	//printk("major=%d, minor=%d\n", MAJOR(gpio_devno), MINOR(gpio_devno));
    if(ret < 0){  
        printk("%s register chrdev error\n",__func__);  
        return ret;  
    }  
	//创建设备类  
	gpio_driver_class = class_create(THIS_MODULE, CLASS_NAME);   //   /sys/CLASS_NAME
    if (IS_ERR(gpio_driver_class)){  
        printk("failed to create zxi_gpio moudle class.\n");  
        unregister_chrdev_region(gpio_devno,NUMBER_OF_DEVICES);  
        return -1;  
    }  
    //去/sysfs下寻找对应的类从而/dev目录下创建相应的设备节点
	gpio_driver_device = device_create(gpio_driver_class, NULL, gpio_devno, NULL, DEVICE_NAME); // /sys/CLASS_NAME/DEVICE_NAME  
    if (IS_ERR(gpio_driver_device)){  
        printk("failed to create device .\n");  
        unregister_chrdev_region(gpio_devno,NUMBER_OF_DEVICES);  
        return -1;  
    }  
  
	cdev_init(&gpio_cdev,&led_fops);//字符设备初始化函数，将cdev和f_ops绑定起来  
	gpio_cdev.owner = THIS_MODULE;
    cdev_add(&gpio_cdev,gpio_devno,NUMBER_OF_DEVICES);//注册设备，gpio_devno为主设备号，设备数为1 

	 //To get Custom IP--gpio moudle's virtual address 
#if 1	
	if(request_mem_region(XGPIO_PHY_ADDR, 0x10,DEVICE_NAME) == NULL ){
		printk( "request_mem_region failed\n");
		return -1;
	}
#endif  
    return 0;  
}  

static void __exit axi_gpio_driver_module_exit(void)
{  
	//撤销映射关系 
    iounmap((void *)axi_gpio_virt_addr); 
#if 1
	release_mem_region(XGPIO_PHY_ADDR, 0x10);
#endif
    cdev_del(&gpio_cdev);//删除驱动gpio_cdev
	device_destroy(gpio_driver_class, gpio_devno);  
    class_unregister(gpio_driver_class);  
    class_destroy(gpio_driver_class);  
    unregister_chrdev_region(gpio_devno,NUMBER_OF_DEVICES);//释放设备号  
}  
   
module_init(axi_gpio_driver_module_init);
module_exit(axi_gpio_driver_module_exit);  


MODULE_AUTHOR("Xilinx ");  
MODULE_DESCRIPTION("AXI GPIO moudle dirver");  
MODULE_VERSION("v1.0");  
MODULE_LICENSE("GPL");  




////////////////////////////////////////////////////////////
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <sys/ioctl.h>  
#include <stdio.h>

#define LED_MAGIC  'L'  
#define LED_ON     _IO(LED_MAGIC,0)  
#define LED_OFF    _IO(LED_MAGIC,1)


int main(int argc, char **argv)
{
	int fd;
	int cmd;//定义cmd  
	fd = open("/dev/myled", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}
	
		cmd= atoi(argv[1]);//atoi用来把字符串转换成整形数，因为我们的cmd其实都是一个整形数  

		 
		if(cmd==1)  
		   ioctl(fd,LED_ON);  
		else  
		   ioctl(fd,LED_OFF);  
	close(fd);
	return 0;
}


/*
开发板上电：
insmod mytest.ko
cat proc/devices    zynqgpio
./ledapp 1
./ledapp 0
rmmod led
*/