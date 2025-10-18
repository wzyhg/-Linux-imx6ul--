
#include <linux/platform_device.h>
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
#include <asm/io.h>
#include "chip_button_gpio.h"
#include "button_drv.h"

struct iomux {
	volatile unsigned int unnames[23];
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO00; /* offset 0x5c */
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO01;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO02;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO04;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO05;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO06;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO07;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO08;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO09;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_UART1_TX_DATA;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_UART1_RX_DATA;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_UART1_CTS_B;
};

/* enable GPIO5 clock*/
static volatile unsigned int* CCM_CCGR1;

/* set GPIO5_IO01 as GPIO */
static volatile unsigned int* IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER1;

static struct iomux* iomux;
/* GPIO registers */
static struct imx6ull_gpio_registers* gpio5_registers;
static struct imx6ull_gpio_registers* gpio1_registers;

/*提供给上一层的接口*/
static struct button_operations button_opr = {
	.init = button_gpio_init,
	.read = button_gpio_read,
	.count = 2,
};

/*初始化button，which指定哪一个button*/
static void button_gpio_init(int which) {
	if (!CCM_CCGR1)
	{
		/* 将物理地址映射到虚拟地址上 */
		CCM_CCGR1 = ioremap(0x20C406C, 4);
		IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER1 = ioremap(0x229000C, 4);
		iomux = ioremap(0x20e0000, sizeof(struct iomux));
		gpio1_registers = ioremap(0x209C000, sizeof(struct imx6ull_gpio_registers));
		gpio5_registers = ioremap(0x20AC000, sizeof(struct imx6ull_gpio_registers));
		

		/* 默认就是第0个按键 */
		if (which == 0) {
			/* 1. enable GPIO5 clock*/
			*CCM_CCGR1 |= (3 << 30); //30-31位置1

			/* 2. set GPIO5_IO01 as GPIO*/

			*IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER1 = 5; //低四位MUX_MODE置0101

			/* 3. set GPIO5_IO01 as input*/
			gpio5_registers->GDIR &= ~(1 << 1); //将GDIR寄存器的第一位置0
		}
		/* 后面如果想使用多个按键的话可以用if else判断 */
		else if (which == 1) {
			/* Button1: GPIO5_IO02 */
			/* 2. set GPIO5_IO02 as GPIO */
			*CCM_CCGR1 |= (3 << 26); //30-31位置1
			iomux->IOMUXC_SW_MUX_CTL_PAD_UART1_CTS_B = 5;

			/* 3. set GPIO5_IO02 as input */
			gpio1_registers->GDIR &= ~(1 << 18); // 将GDIR寄存器的第2位置0，表示输入
		}
	}
}

/*读取引脚电平，查看按键是否按下，0表示未按下，1表示按下，-1表示出错*/
static int button_gpio_read(int which) {
	//将psr寄存器的第一位状态返回，该位即为引脚状态。按下按钮时为0，未按下时为1
	if (which == 0)
		return (gpio5_registers->PSR & (1 << 1)) ? 1 : 0;
	else
		return (gpio1_registers->PSR & (1 << 18)) ? 1 : 0;
}


/*入口函数，模块一被装载就执行*/
static int chip_button_gpio_drv_init(void) {
	//将button_opr传给上一层
	register_button_operations(&button_opr);
	//创建从设备
	button_device_create(0);
	button_device_create(1);
	return 0;
}

/*入口函数，模块一被装载就执行*/
static void chip_button_gpio_drv_exit(void) {
	//销毁从设备
	button_device_destroy(0);
	button_device_destroy(1);
	
}

module_init(chip_button_gpio_drv_init);
module_exit(chip_button_gpio_drv_exit);
MODULE_LICENSE("GPL");



