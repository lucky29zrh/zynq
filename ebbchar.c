//该代码参考一个老外博客，给自己学习内核buf和用户buf的交互有一定的指导作用。

/**
 * @file   ebbchar.c
 * @brief   一个简单的字符设备驱动
 * 该Linux loadable kernel module (LKM)模块可以映射到/dev/ebbchar
 * 在用户控件的C程序可以和该模块进行交互
 */
 
#include <linux/init.h>           //  __init __exit等函数
#include <linux/module.h>         // 
#include <linux/device.h>         // 设备相关
#include <linux/kernel.h>         // 包括类型, 宏, 内核函数
#include <linux/fs.h>             // 文件相关
#include <linux/uaccess.h>        // copy_to_user函数


#define  DEVICE_NAME "ebbchar"    /// /dev/ebbchar 
#define  CLASS_NAME  "ebb"        /// device class 
          
 
static int    majorNumber;                  // 主设备号(动态生成)
static char   message[256] = {0};           // 内核buf,和用户buf对应
static short  size_of_message;              //  buf大小
static int    numberOpens = 0;              //  记录设备被open的次数
static struct class*  ebbcharClass  = NULL; //  The device-driver class struct pointer
static struct device* ebbcharDevice = NULL; //  The device-driver device struct pointer
 
//函数声明
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
 
/** 
 * 文件操作对应函数指针
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};
 
/** @brief LKM初始化函数
 *  __init 宏表示该函数仅仅初始一次
 *  @return 若成功返回0
 */
static int __init ebbchar_init(void){
   printk(KERN_INFO "EBBChar  Init\n");
 
  
   // 动态分配一个主设备号
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "EBBChar failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "EBBChar: registered correctly with major number %d\n", majorNumber);
 
   // 注册设备类
   ebbcharClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(ebbcharClass)){                // 检查错误
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(ebbcharClass);          
   }
   printk(KERN_INFO "EBBChar: device class registered correctly\n");
 
   // 注册设备驱动
   ebbcharDevice = device_create(ebbcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(ebbcharDevice)){               // 检查错误
      class_destroy(ebbcharClass);           
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(ebbcharDevice);
   }
   printk(KERN_INFO "EBBChar: device class created correctly\n");  
   return 0;
}
 
/** @brief LKM清除函数
 *  __exit 宏表示该函数仅仅调用一次
 */
static void __exit ebbchar_exit(void){
   device_destroy(ebbcharClass, MKDEV(majorNumber, 0));     // 移除设备
   class_unregister(ebbcharClass);                          // 注销设备类
   class_destroy(ebbcharClass);                             // 移除设备类
   unregister_chrdev(majorNumber, DEVICE_NAME);             // 注销主设备号
   printk(KERN_INFO "EBBChar: Goodbye from the LKM!\n");
}
 
/** @brief 函数被调用当用户调用Open()函数时
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;
   printk(KERN_INFO "EBBChar: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}
 
/** @brief 函数被调用当用户调用read()函数时
 *  copy_to_user() 将内核message数据传到用户buffer
 *  copy_to_user( * to, *from, size) 成功时返回0
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
    
   error_count = copy_to_user(buffer, message, size_of_message);
   //  成功时返回0
   if (error_count==0){            
      printk(KERN_INFO "EBBChar: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0);   
   }// 失败时返回错误信息
   else {
      printk(KERN_INFO "EBBChar: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;               
   }
}
 
/** @brief  函数被调用当用户调用read()函数时 
 *  sprintf() 将用户buffer指定长度为len的数据拷贝到内核message
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   sprintf(message, "%s(%zu letters)", buffer, len);   // 指定用户空间数据的buffer,len
   size_of_message = strlen(message);                  // 存储实际长度
   printk(KERN_INFO "EBBChar: Received %zu characters from the user\n", len);
   return len;
}
 
/** @brief 函数被调用当用户调用closed/released函数时 
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "EBBChar: Device successfully closed\n");
   return 0;
}
 
/** @brief 
 *  注明入口、出口函数
 */
module_init(ebbchar_init);
module_exit(ebbchar_exit);


MODULE_LICENSE("GPL");            
MODULE_AUTHOR("pp");     
MODULE_DESCRIPTION("A simple Linux char driver");  
MODULE_VERSION("0.1");   



//--------------------------------------------------------
/**
 * @file   testebbchar.c
 * @brief  Linux 应用程序和驱动进行交互
 * 向LKM写入一个字符串，并从LKM中读出
 * 设备文件为 /dev/ebbchar.
*/
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256               //  buf大小
static char receive[BUFFER_LENGTH];     //  接收数据buf

int main(){
   int ret, fd;
   char stringToSend[BUFFER_LENGTH];
   printf("Starting device test code example...\n");
   //打开设备文件、获取文件句柄
   fd = open("/dev/ebbchar", O_RDWR);             
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }
   printf("Type in a short string to send LKM:\n");
   scanf("%[^\n]%*c", stringToSend);                // 可输入空格，回车结束
   printf("Writing message to the device [%s].\n", stringToSend);
   //向LKM写入字符串
   ret = write(fd, stringToSend, strlen(stringToSend)); 
   if (ret < 0){
      perror("Failed to write the message to the device.");
      return errno;
   }

   printf("Press ENTER to read back from the device...\n");
   getchar();

   //从LKM读取字符串
   printf("Reading from the device...\n");
   ret = read(fd, receive, BUFFER_LENGTH);   
   if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }
   printf("The received message is: [%s]\n", receive);
   return 0;
}



//--------------------------------------------------------
// 为什么要加互斥锁？
// 当两个app访问LKM时，app1写入数据后等待，用户2写入数据，之后app1读取数据
// 那么此时app1读取到的数据就是错误的，是app2写入的数据。
// 因此应该保证LKM仅仅被一个app访问
//--------------------------------------------------------
#include <linux/mutex.h>	         /// 包含互斥锁
…
static DEFINE_MUTEX(ebbchar_mutex);  /// 静态定义互斥锁,初值为1(unlocked)
                                     /// DEFINE_MUTEX_LOCKED() 后值为0 (locked)
									 
									 
…
static int __init ebbchar_init(void){
   …
   mutex_init(&ebbchar_mutex);       /// 动态初始化
}

static int dev_open(struct inode *inodep, struct file *filep){
   if(!mutex_trylock(&ebbchar_mutex)){    /// 试图获得锁  
                                          /// 成功时返回0
      printk(KERN_ALERT "EBBChar: Device in use by another process");
      return -EBUSY;
   }
   …
}

static int dev_release(struct inode *inodep, struct file *filep){
   mutex_unlock(&ebbchar_mutex);          /// 释放锁
   …
}

static void __exit ebbchar_exit(void){
   mutex_destroy(&ebbchar_mutex);        /// 销毁互斥锁
   …
}

