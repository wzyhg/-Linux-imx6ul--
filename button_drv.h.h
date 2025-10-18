#ifndef _BUTTON_DRV_H
#define _BUTTON_DRV_H
	
#include "chip_button_gpio.h"
 struct button_operations {
	int count;
	void (*init) (int which);
	int (*read) (int which);
};
void button_device_create(int minor);
void button_device_destroy(int minor);
void register_button_operations(struct button_operations *opr);
 
#endif

