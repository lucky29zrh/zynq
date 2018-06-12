#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/fs.h>  
#include <linux/device.h>  
#include <asm/io.h>  

/*
修改说明： 该代码仍然在init和exit函数中实现硬件操作
修改之处：1) 使用iowrite ioread完成读写数据操作
         2）添加了申请内存request_mem_region函数(不必须)
*/
#define DEVICE_NAME          "axiled"
#define CLASS_NAME           "zynqebb" 

/* XGPIO Physical address */
#define XGPIO_PHY_ADDR 0x41200000      //This Address is based SDK 
/* Register Offset Definitions */
#define XGPIO_DATA_OFFSET   (0x0)	/* Data register  */
#define XGPIO_TRI_OFFSET    (0x4)	/* I/O direction register  */

volatile unsigned long *Gpio_DIR = NULL;
volatile unsigned long *Gpio_DATA = NULL;
  

static int axi_gpio_driver_major;  
static struct class* axi_gpio_driver_class = NULL;  
static struct device* axi_gpio_driver_device = NULL;  
  
unsigned long axi_gpio_virt_addr = 0;        //AXI_GPIO moulde's  visual address  
  
  
static struct file_operations axi_gpio_fops = {  
    .owner = THIS_MODULE,  
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
	printk(" axi_gpio driver initial successfully!\n"); 
	printk( "ioremap called: phys %#08x -> virt %#08x\n",XGPIO_PHY_ADDR, axi_gpio_virt_addr );
	//指定需要操作的寄存器的地址
	Gpio_DIR = (unsigned long *)(axi_gpio_virt_addr + XGPIO_TRI_OFFSET);
	Gpio_DATA = (unsigned long *)(axi_gpio_virt_addr + XGPIO_DATA_OFFSET);
	printk("before Gpio_DIR:%lx,%lx.\n",Gpio_DIR,ioread32(Gpio_DIR));
	printk("before Gpio_DATA:%lx,%lx.\n",Gpio_DATA,ioread32(Gpio_DATA));
	//*Gpio_DIR = (unsigned long )0x00000000;       //设置AXI GPIO的方向输出  
	//*Gpio_DATA = (unsigned long )0x00000005;     //设置AXI GPIO的方向输出全为高  
	iowrite32(0x00000000,Gpio_DIR);  //设置AXI GPIO的方向输出  
	iowrite32(0x00000005,Gpio_DATA); //设置AXI GPIO的高电平亮 
	printk("after Gpio_DIR:%lx,%lx.\n",Gpio_DIR,*Gpio_DIR);
	printk("after Gpio_DATA:%lx,%lx.\n",Gpio_DATA,*Gpio_DATA);
	return 0;
}  
static void __exit axi_gpio_driver_module_exit(void)  
{  


    iounmap((void *)axi_gpio_virt_addr); //撤销映射关系
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