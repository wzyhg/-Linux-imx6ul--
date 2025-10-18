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
#include "button_drv.h"

/*确定主设备号，可以自己指定，也可以设置为0让系统自动分配*/
static int major = 0;
static struct class* button_class;
/*指向button_operations的指针*/
struct button_operations* p_button_opr;

/*创建button设备，minor是从设备号*/
void button_device_create(int minor) {
	device_create(button_class, NULL, MKDEV(major, minor), NULL, "lzp_button%d", minor);
}

/*销毁对应的button设备*/
void button_device_destroy(int minor) {
	device_destroy(button_class, MKDEV(major, minor));
}

/*获取chip_button_gpio中的button_operations*/
void register_button_operations(struct button_operations* opr) {
	p_button_opr = opr;
}

/*使用EXPORT_SYMBOL可以将一个函数以符号的方式导出给其他模块使用*/
EXPORT_SYMBOL(button_device_create);
EXPORT_SYMBOL(button_device_destroy);
EXPORT_SYMBOL(register_button_operations);


/*实现对应的open/read/write等函数，填入file_operations结构体*/
static ssize_t button_drv_read(struct file* file, char __user* buf, size_t size, loff_t* offset) {
	/*获取次设备号，即read函数想要操作的设备*/
	unsigned int minor = iminor(file_inode(file));
	/*读取按键电平*/
	char level = p_button_opr->read(minor);
	/*将传回给用户空间，不能直接访问buf，需要用到copy_to_user函数*/
	copy_to_user(buf, &level, 1);
	return 0;
}

static int button_drv_open(struct inode* node, struct file* file) {
	/*获取次设备号*/
	int minor = iminor(node);
	/*根据次设备号初始化button*/
	p_button_opr->init(minor);

	return 0;
}

/*定义自己button的file_operations结构体*/
static struct file_operations button_drv = {
	.open = button_drv_open,
	.read = button_drv_read,
};

/*把file_operations结构体告诉内核：注册驱动程序*/
/*入口函数：安装驱动程序时，就会去调用这个入口函数*/
static int button_init(void) {
	int err;
	/*注册字符设备，把file_operations结构体告诉内核，获取主设备号*/
	major = register_chrdev(0, "lzp_button", &button_drv);
	button_class = class_create(THIS_MODULE, "lzp_button_class");
	err = PTR_ERR(button_class);
	if (IS_ERR(button_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "lzp_button");
		return -1;
	}
	return 0;
}


/*出口函数：卸载驱动程序时，就会去调用这个出口函数*/
static void button_exit(void)
{
	class_destroy(button_class);
	unregister_chrdev(major, "lzp_button");
}


module_init(button_init);
module_exit(button_exit);

MODULE_LICENSE("GPL");

