#ifndef _GPIO_H_
#define _GPIO_H_

#include "driver/gpio.h"

void gpio_init(int pin, int mode, int pullup, int pulldown, int intr, void (*handler)(void*), void* arg);





#endif