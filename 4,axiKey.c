/*
修改说明： 该代码AXI key按键值读取
修改之处：1) 基地址为0x41210000
         2）open函数实现方向配置(输入)
         3) read函数中实现按键值读取并传到用户空间
注意copy_to_user 传输数据为4字节，读取时也读取了4字节
*/

#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/fs.h>  
#include <asm/irq.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>  		  //包含了device、class 等结构的定义
#include <asm/io.h>               //包含了ioremap、iowrite等内核访问IO内存等函数
#include <linux/uaccess.h>        //包含了copy_to_user、copy_from_user等
   
#define DEVICE_NAME          "axikey"
#define CLASS_NAME           "zynqebb" 

/* XGPIO Physical address */
#define XGPIO_PHY_ADDR 0x41210000      //This Address is based SDK 
/* Register Offset Definitions */
#define XGPIO_DATA_OFFSET   (0x0)	/* Data register  */
#define XGPIO_TRI_OFFSET    (0x4)	/* I/O direction register  */

volatile unsigned long *Gpio_DIR = NULL;
volatile unsigned long *Gpio_DATA = NULL;
  
  
static int axi_gpio_driver_major;  
static struct class* axi_gpio_driver_class = NULL;  
static struct device* axi_gpio_driver_device = NULL;  
  
unsigned long axi_gpio_virt_addr = 0;        //AXI_GPIO moulde's  visual address  
  

ssize_t axi_gpio_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	//printk("axi_gpio_read\n");
	
	unsigned long key_val = ioread32(Gpio_DATA);
	//printk("after Gpio_DATA:%lx,%lx.\n",Gpio_DATA,key_val);
	/* 将按键状态复制给用户,成功返回0 */
    copy_to_user(buf, (const void *)&key_val,sizeof(key_val));
    key_val = 0;

    return 0;

}
static int axi_gpio_open(struct inode *inode, struct file *filp)
{
	//printk("axi_gpio_open\n");

	//配置key管脚为输入
	printk("before Gpio_DIR:%lx,%lx.\n",Gpio_DIR,ioread32(Gpio_DIR));
	
	iowrite32(0x0000000F,Gpio_DIR);  //设置AXI GPIO的方向输入 
	
	printk("after Gpio_DIR:%lx,%lx.\n",Gpio_DIR,ioread32(Gpio_DIR));
	
	return 0;
}
  
static struct file_operations axi_gpio_fops = {  
    .owner = THIS_MODULE,  
	.read       = axi_gpio_read,
	.open        = axi_gpio_open,
};  


static int __init axi_gpio_driver_module_init(void)  
{  
    int ret;  
  
    axi_gpio_driver_major=register_chrdev(0, DEVICE_NAME, &axi_gpio_fops );//内核注册设备驱动  
    if (axi_gpio_driver_major < 0){  
        printk("failed to register device.\n");  
        return -1;  
    }  
  
    axi_gpio_driver_class = class_create(THIS_MODULE, CLASS_NAME);//创建设备类  
    if (IS_ERR(axi_gpio_driver_class)){  
        printk("failed to create zxi_gpio moudle class.\n");  
        unregister_chrdev(axi_gpio_driver_major, DEVICE_NAME);  
        return -1;  
    }  
  
  
    axi_gpio_driver_device = device_create(axi_gpio_driver_class, NULL, MKDEV(axi_gpio_driver_major, 0), NULL, DEVICE_NAME);   
    if (IS_ERR(axi_gpio_driver_device)){  
        printk("failed to create device .\n");  
        unregister_chrdev(axi_gpio_driver_major, DEVICE_NAME);  
        return -1;  
    }  
     
     
    //To get Custom IP--gpio moudle's virtual address 
#if 1	
	if(request_mem_region(XGPIO_PHY_ADDR, 0x1000,DEVICE_NAME) == NULL ){
		printk( "request_mem_region failed\n");
		return -1;
	}
#endif  
    axi_gpio_virt_addr = (unsigned long)ioremap(XGPIO_PHY_ADDR, sizeof(u32));
    //将模块的物理地址映射到虚拟地址上  
	printk( "ioremap called: phys %#08x -> virt %#08x\n",XGPIO_PHY_ADDR, axi_gpio_virt_addr );
	//指定需要操作的寄存器的地址
	Gpio_DIR =  (unsigned long *)(axi_gpio_virt_addr + XGPIO_TRI_OFFSET);
	Gpio_DATA = (unsigned long *)(axi_gpio_virt_addr + XGPIO_DATA_OFFSET);
	
	return 0;
}  
static void __exit axi_gpio_driver_module_exit(void)  
{  

	//撤销映射关系 
    iounmap((void *)axi_gpio_virt_addr); 
#if 1
	release_mem_region(XGPIO_PHY_ADDR, 0x1000);
#endif
    device_destroy(axi_gpio_driver_class, MKDEV(axi_gpio_driver_major, 0));  
    class_unregister(axi_gpio_driver_class);  
    class_destroy(axi_gpio_driver_class);  
    unregister_chrdev(axi_gpio_driver_major, DEVICE_NAME);  
    printk("axi_gpio module exit.\n");  
}  
  
module_init(axi_gpio_driver_module_init);  
module_exit(axi_gpio_driver_module_exit);  



MODULE_AUTHOR("Xilinx ");  
MODULE_DESCRIPTION("AXI GPIO moudle dirver");  
MODULE_VERSION("v1.0");  
MODULE_LICENSE("GPL");  



//-----------------------------------------
//应用层程序
//-----------------------------------------

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
 #include <unistd.h>  //sleep
/* thirddrvtest 
  */
int main(int argc, char **argv)
{
	int fd;
	unsigned long key_val;
	
	fd = open("/dev/axikey", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}

	while (1)
	{
		read(fd, &key_val, 4);
		printf("key_val = 0x%x\n", key_val);
		sleep(1);
	}
	
	return 0;
}

