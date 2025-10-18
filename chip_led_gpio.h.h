#ifndef _CHIP_LED_GPIO_H
#define _CHIP_LED_GPIO_H
 
/*gpio寄存器，因为每个gpio模块的寄存器地址连续，且各个寄存器之间相隔4B*/
struct imx6ull_gpio_registers {
    volatile unsigned int DR; //gpio data register
    volatile unsigned int GDIR; //gpio direction register
    volatile unsigned int PSR; //gpio pad status register
    volatile unsigned int ICR1; //gpio interrupt configuration register1
    volatile unsigned int ICR2; //gpio interrupt configuration register2
    volatile unsigned int IMR; //gpio interrupt mask register
    volatile unsigned int ISR; //gpio interrupt status register
    volatile unsigned int EDGE_SEL; //gpio edge select register
};
 
//对led的操作
struct led_operations {
        int (*init)  (int which);
        int (*ctl) (int which, int status);
};
 
 
static int led_gpio_init(int which);
static int led_gpio_write(int which, int status);
 
#endif

