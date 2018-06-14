
参考：https://blog.csdn.net/deep_l_zh/article/details/48550127
// 自动分配设备号，使用了新的API（申请、释放）

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

 
#define DEVICE_NAME          "myled"
#define CLASS_NAME           "zynqebb" 
  


struct cdev gpio_cdev;            //定义设备的结构体 
dev_t gpio_devno; 				  //动态分配设备号
#define NUMBER_OF_DEVICES 1 

static struct class*  gpio_driver_class = NULL;  
static struct device* gpio_driver_device = NULL; 


int axi_gpio_open(struct inode *inode,struct file *filep) 
{  
   

    return 0;  
} 


static ssize_t axi_gpio_write(struct file * file, const char * buf, size_t count,loff_t *off)
{

    return 0;  
} 
static struct file_operations led_fops=   // 对应的对led的操作  
{  
    .open	= axi_gpio_open,  
	.write  = axi_gpio_write,
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
    return 0;  
}  

static void __exit axi_gpio_driver_module_exit(void)
{  
    cdev_del(&gpio_cdev);//删除驱动gpio_cdev
	device_destroy(gpio_driver_class, MKDEV(gpio_devno, 0));  
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
