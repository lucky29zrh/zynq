

mydrv.c
-------
/*
修改说明： 该代码AXI key按键值读取
修改之处：1) 基地址为0x41210000
         2）open函数实现方向配置(输出)
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

#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
   
#define DEVICE_NAME          "axigpio"
#define CLASS_NAME           "zynqebb" 


/* Register Offset Definitions */
#define XGpio_DATA_OFFSET   (0x0)	/* Data register  */
#define XGPIO_TRI_OFFSET    (0x4)	/* I/O direction register  */

volatile unsigned long *Gpioled_DIR = NULL; //LED的控制寄存器、数据寄存器
volatile unsigned long *Gpioled_DATA = NULL;
  

volatile unsigned long *Gpiokey_DIR = NULL;  //按键的控制寄存器、数据寄存器
volatile unsigned long *Gpiokey_DATA = NULL;
  
static int axi_gpio_driver_major;  
static struct class* axi_gpio_driver_class = NULL;  
static struct device* axi_gpio_driver_device = NULL;  
  
unsigned long axi_led_virt_addr = 0;        //AXI_GPIO led visual address  
unsigned long axi_key_virt_addr = 0;        //AXI_GPIO key visual address   

ssize_t axi_gpio_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	printk("axi_gpio_read\n");
	
	unsigned long key_val = ioread32(Gpiokey_DATA);
	printk("after Gpioled_DATA:%lx,%lx.\n",Gpiokey_DATA,key_val);
	/* 将按键状态复制给用户,成功返回0 */
        copy_to_user(buf, (const void *)&key_val,sizeof(key_val));
        key_val = 0;

        return 0;

}

static ssize_t axi_gpio_write(struct file * file, const char * buf, size_t count,loff_t *off)
{
	//printk("axi_gpio_write\n");
	int  val;
	copy_from_user(&val,buf,count);
	printk("before Gpioled_DATA:%lx,%lx.\n",Gpioled_DATA,ioread32(Gpioled_DATA));
	
	if(val == 1)
	{
		//点灯
		//*Gpioled_DATA = (unsigned long )0x00000005;     //设置AXI GPIO的方向输出全为高  
		iowrite32(0x0000000F,Gpioled_DATA); //设置AXI GPIO的方向输出全为高  
	}
	else
	{ 
		//灭灯 
		iowrite32(0x00000000,Gpioled_DATA); //设置AXI GPIO的方向输出全为低  
	}
	printk("after Gpioled_DATA:%lx,%lx.\n",Gpioled_DATA,ioread32(Gpioled_DATA));
	return 0;

}
static int axi_gpio_open(struct inode *inode, struct file *filp)
{
	//printk("axi_gpio_open\n");

	//配置key管脚为输入
	printk("before Gpioled_DIR:%lx,%lx.\n",Gpioled_DIR,ioread32(Gpioled_DIR));
	
	iowrite32(0x00000000,Gpioled_DIR);  //设置AXI GPIO的方向输出 
	
	printk("after Gpioled_DIR:%lx,%lx.\n",Gpioled_DIR,ioread32(Gpioled_DIR));
	
	return 0;
}
  
static struct file_operations axi_gpio_fops = {  
    .owner = THIS_MODULE,  
	.read       = axi_gpio_read,
	.write       = axi_gpio_write,
	.open        = axi_gpio_open,
};  


static int  axi_gpio_driver_module_init(void)  
{  
  
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
     
	
	return 0;
}  
static void  axi_gpio_driver_module_exit(void)  
{  

	//撤销映射关系 
       iounmap((void *)axi_led_virt_addr); 
       iounmap((void *)axi_key_virt_addr); 
#if 0
	release_mem_region(XGPIO_PHY_ADDR, 0x1000);
#endif
    device_destroy(axi_gpio_driver_class, MKDEV(axi_gpio_driver_major, 0));  
    class_unregister(axi_gpio_driver_class);  
    class_destroy(axi_gpio_driver_class);  
    unregister_chrdev(axi_gpio_driver_major, DEVICE_NAME);  
    printk("axi_gpio module exit.\n");  
}  
  
//module_init(axi_gpio_driver_module_init);  
//module_exit(axi_gpio_driver_module_exit);  

static int leddrv_probe(struct platform_device *pdev)
{
	int ret;	
	printk("match ok!");
	struct resource *r_irq; /* Interrupt resources */
	struct resource *r_mem; /* IO mem resources */

	//To get Custom IP--gpio moudle's virtual address 
#if 0	
	if(request_mem_region(XGPIO_PHY_ADDR, 0x1000,DEVICE_NAME) == NULL ){
		printk( "request_mem_region failed\n");
		return -1;
	}
#endif  
       // axi_led_virt_addr = (unsigned long)ioremap(XGPIO_PHY_ADDR, sizeof(u32));
	axi_led_virt_addr = (unsigned long)ioremap(pdev->resource[0].start,pdev->resource[0].end - pdev->resource[0].start);
	axi_key_virt_addr = (unsigned long)ioremap(pdev->resource[1].start,pdev->resource[1].end - pdev->resource[1].start);
       //将模块的物理地址映射到虚拟地址上  
	printk( "ioremap led: phys %#08x -> virt %#08x\n",pdev->resource[0].start, axi_led_virt_addr );
	printk( "ioremap key: phys %#08x -> virt %#08x\n",pdev->resource[1].start, axi_key_virt_addr );
       //测试新的API
	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r_mem) {
		printk("invalid address \n");
		return -ENODEV;
	}
	printk("0 start: %#08x end:%08x\n",r_mem->start,r_mem->end);

	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!r_mem) {
		printk("invalid address\n");
		return -ENODEV;
	}
	printk("1 start: %#08x end:%08x\n",r_mem->start,r_mem->end);
	//指定需要操作LED的寄存器的地址
	Gpioled_DIR =  (unsigned long *)(axi_led_virt_addr + XGPIO_TRI_OFFSET);
	Gpioled_DATA = (unsigned long *)(axi_led_virt_addr + XGpio_DATA_OFFSET);
	//指定需要操作key的寄存器的地址
	Gpiokey_DIR =  (unsigned long *)(axi_key_virt_addr + XGPIO_TRI_OFFSET);
	Gpiokey_DATA = (unsigned long *)(axi_key_virt_addr + XGpio_DATA_OFFSET);
	axi_gpio_driver_module_init();
	return 0;	
}

 
static int leddrv_remove(struct platform_device *pdev)
{	
	printk("match quit!");
	axi_gpio_driver_module_exit();
	return 0;
}


static struct platform_driver leddrv_driver=
{
    .driver.name = "axigpio",
    .probe = leddrv_probe,
    .remove = leddrv_remove,
};
 
 
static int leddrv_init(void)
{
	printk("leddrv_init\n");
	
	return platform_driver_register(&leddrv_driver);
}
 
static void leddrv_exit(void)
{
	printk("leddrv_exit\n");
	platform_driver_unregister(&leddrv_driver);
	
	return;
}

module_init(leddrv_init);
module_exit(leddrv_exit);



MODULE_AUTHOR("Xilinx ");  
MODULE_DESCRIPTION("AXI GPIO moudle dirver");  
MODULE_VERSION("v1.0");  
MODULE_LICENSE("GPL"); 



mydev.c
------
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
 
static struct resource gpio_resource[] =
{
	[0] ={
		.start = 0x41210000,
		.end =   0x41210000 + 0x04,
		.flags = IORESOURCE_MEM,  //led
	},
 
	[1] ={
		.start = 0x41200000,
		.end =   0x41200000 + 0x04,
		.flags = IORESOURCE_MEM,  //sws_key
	},
	[2] ={
		.flags = IORESOURCE_IRQ,
	}
	
};
 
static void leddev_release(struct device *dev)
{
	printk("leddev_release\n");
	return ;
}
 
 
 
static struct platform_device leddev_device=                            
{
    .name = "axigpio",
    .id = -1,
    .dev.release = leddev_release,
    .num_resources = ARRAY_SIZE(gpio_resource),
    .resource = gpio_resource,
};
 
static int leddev_init(void)
{
	printk("leddev_init\n");
	return platform_device_register(&leddev_device);
}
 
static void leddev_exit(void)
{
	printk("leddev_exit\n");
	platform_device_unregister(&leddev_device);
	return;
}
 
MODULE_LICENSE("GPL");
module_init(leddev_init);
module_exit(leddev_exit);

-----
指令：
petalinux-create -t project --template zynq  -n zc706plkey
cd plInterrupt.sdk/
petalinux-config --get-hw-description -p ../zc706plkey/
cd ../zc706plkey/
petalinux-build

petalinux-create -t modules --name mymodule --enable
petalinux-build -c mymodule -x do_build
petalinux-build -c mymodule -x do_install
cp build/tmp/work/plnx_arm-xilinx-linux-gnueabi/mymodule/1.0-r0/mymodule.ko /home/pp/nfsShare/


 petalinux-package --boot --fsbl ./images/linux/zynq_fsbl.elf --fpga ./images/linux/system_wrapper.bit   --u-boot --force

arm-xilinx-linux-gnueabi-gcc testAxiled.c -o testAxiled -static
