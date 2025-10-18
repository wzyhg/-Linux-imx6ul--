#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include "led_drv.h"

/*确定主设备号，可以自己指定，也可以设置为0让系统自动分配*/
static int major = 0;
static struct class* led_class;
/*指向led_operations的指针*/
struct led_operations* p_led_opr;

/*创建led设备，minor是从设备号*/
void led_device_create(int minor) {
	device_create(led_class, NULL, MKDEV(major, minor), NULL, "lzp_led%d", minor);
}

/*销毁对应的led设备*/
void led_device_destroy(int minor) {
	device_destroy(led_class, MKDEV(major, minor));
}

/*获取chip_led_gpio中的led_operations*/
void register_led_operations(struct led_operations* opr) {
	p_led_opr = opr;
}

/*使用EXPORT_SYMBOL可以将一个函数以符号的方式导出给其他模块使用*/
EXPORT_SYMBOL(led_device_create);
EXPORT_SYMBOL(led_device_destroy);
EXPORT_SYMBOL(register_led_operations);


/*实现对应的open/read/write等函数，填入file_operations结构体*/
static ssize_t led_drv_write(struct file* file, const char __user* buf, size_t size, loff_t* offset) {
	char status;
	/*获取次设备号，即read函数想要操作的设备*/
	unsigned int minor = iminor(file_inode(file));
	/*读取用户传来的status*/
	copy_from_user(&status, buf, 1);
	/*操作led*/
	p_led_opr->ctl(minor, status);
	return 0;
}

static int led_drv_open(struct inode* node, struct file* file) {
	/*获取次设备号*/
	int minor = iminor(node);
	/*根据次设备号初始化button*/
	p_led_opr->init(minor);

	return 0;
}

/*定义自己led的file_operations结构体*/
static struct file_operations led_drv = {
	.open = led_drv_open,
	.write = led_drv_write,
};

/*把file_operations结构体告诉内核：注册驱动程序*/
/*入口函数：安装驱动程序时，就会去调用这个入口函数*/
static int led_init(void) {
	int err;
	/*注册字符设备，把file_operations结构体告诉内核，获取主设备号*/
	major = register_chrdev(0, "lzp_led", &led_drv);
	led_class = class_create(THIS_MODULE, "lzp_led_class");
	err = PTR_ERR(led_class);
	if (IS_ERR(led_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "lzp_led");
		return -1;
	}
	return 0;
}


/*出口函数：卸载驱动程序时，就会去调用这个出口函数*/
static void led_exit(void)
{
	class_destroy(led_class);
	unregister_chrdev(major, "lzp_led");
}


module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");