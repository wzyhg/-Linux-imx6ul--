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
#include "chip_led_gpio.h"
#include "led_drv.h"
struct iomux {
	volatile unsigned int unnames[23];
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO00;  /* offset 0x5c*/
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO01;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO02;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO04;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO05;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO06;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO07;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO08;
	volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO09;
};

/* enable GPIO5 clock*/
static volatile unsigned int* CCM_CCGR1;

/* set GPIO5_IO03 as GPIO */
static volatile unsigned int* IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3;
//static volatile unsigned int* IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER4;  
/* GPIO5 registers */
static struct imx6ull_gpio_registers* gpio5_registers;
static struct imx6ull_gpio_registers* gpio1_registers;
static struct iomux* iomux;
/*提供给上一层的接口*/
static struct led_operations led_opr = {
	.init = led_gpio_init,
	.ctl = led_gpio_write,
};

/*初始化led，which指定哪一个led*/
static int led_gpio_init(int which) {
	/* 将物理地址映射到虚拟地址上 */
	CCM_CCGR1 = ioremap(0x20C406C, 4);
	IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = ioremap(0x2290014, 4);
	iomux = ioremap(0x20e0000, sizeof(struct iomux));
	gpio5_registers = ioremap(0x20AC000, sizeof(struct imx6ull_gpio_registers));
	gpio1_registers = ioremap(0x209C000, sizeof(struct imx6ull_gpio_registers));
	/* 默认就是第0个按键 */
	if (which == 0) {
		/* 1. enable GPIO5 clock*/
		*CCM_CCGR1 |= (3 << 30);
		/* 2. set GPIO5_IO01 as GP*/
		*IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = 5;
		/* 3. set GPIO5_IO01 as input*/
		//gpio1_registers->GDIR |= (1 << 3);
		gpio5_registers->GDIR |= (1 << 3);
		//gpio1_registers->GDIR |= (1 << 5);
		//gpio1_registers->GDIR |= (1 << 6);
	}
	else if (which == 1) {

		*CCM_CCGR1 |= (3 << 26);
		/* 2. set GPIO5_IO01 as GPIO*/
		//*IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 &= ~(0xf); //低四位清0
		iomux->IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03 = 5;//低四位MUX_MODE置0101
		/* 3. set GPIO5_IO01 as input*/
		gpio1_registers->GDIR |= (1 << 3);
	}
	else if (which == 2)
	{
		/* 1. enable GPIO1
		 * CG13, b[27:26] = 0b11
		 */
		*CCM_CCGR1 |= (3 << 26);

		/* 2. set GPIO1_IO05 as GPIO
		 * MUX_MODE, b[3:0] = 0b101
		 */
		iomux->IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO05 = 5;

		/* 3. set GPIO1_IO05 as output
		 * GPIO1 GDIR, b[5] = 0b1
		 */
		gpio1_registers->GDIR |= (1 << 5);
	}
	else if (which == 3)
	{
		/* 1. enable GPIO1
		 * CG13, b[27:26] = 0b11
		 */
		*CCM_CCGR1 |= (3 << 26);

		/* 2. set GPIO1_IO06 as GPIO
		 * MUX_MODE, b[3:0] = 0b101
		 */
		iomux->IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO06 = 5;

		/* 3. set GPIO1_IO06 as output
		 * GPIO1 GDIR, b[6] = 0b1
		 */
		gpio1_registers->GDIR |= (1 << 6);
	}


	/* 后面如果想使用多个按键的话可以用if else判断 */
	return 0;
}

/*写DR寄存器，写1灯亮，写0灯灭*/
static int led_gpio_write(int which, int status) {
	if (which == 0)
	{
		if (status == 1) {
			gpio5_registers->DR |= (1 << 3);
		}
		else {
			//status为0则让灯灭，DR第三位置1
			gpio5_registers->DR &= ~(1 << 3);
		}
	}
	if (which == 1)
	{
		if (status == 1) {
			gpio1_registers->DR |= (1 << 3);
		}
		else {
			//status为0则让灯灭，DR第三位置1
			gpio1_registers->DR &= ~(1 << 3);
		}
	}
	if (which == 2)
	{
		if (status == 1) {
			gpio1_registers->DR |= (1 << 5);
		}
		else {
			//status为0则让灯灭，DR第三位置1
			gpio1_registers->DR &= ~(1 << 5);
		}
	}
	if (which == 3)
	{
		if (status == 1) {
			gpio1_registers->DR |= (1 << 6);
		}
		else {
			//status为0则让灯灭，DR第三位置1
			gpio1_registers->DR &= ~(1 << 6);
		}
	}
	return 0;
}

/*入口函数，模块一被装载就执行*/
static int chip_led_gpio_drv_init(void) {
	//将led_opr传给上一层
	register_led_operations(&led_opr);
	//创建从设备
	led_device_create(0);
	led_device_create(1);
	led_device_create(2);
	led_device_create(3);
	return 0;
}

/*入口函数，模块一被装载就执行*/
static void chip_led_gpio_drv_exit(void) {
	//销毁从设备
	led_device_destroy(0);
	led_device_destroy(1);
	led_device_destroy(2);
	led_device_destroy(3);
}

module_init(chip_led_gpio_drv_init);
module_exit(chip_led_gpio_drv_exit);
MODULE_LICENSE("GPL");