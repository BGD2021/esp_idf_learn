#ifndef _UART_H_
#define _UART_H_
#include "driver/uart.h"
#include <stdio.h>
#include "string.h"
void uart_init(int uart_num,int baud_rate);
void uart_print(int uart_num,char *data);
void printf_uart(int uart_num, const char *format, ...);





#endif