#include "uart.h"

void uart_init(int uart_num,int baud_rate){
    uart_config_t uart_config = {
        .baud_rate = baud_rate, //波特率
        .data_bits = UART_DATA_8_BITS, //数据位
        .parity = UART_PARITY_DISABLE, //奇偶校验
        .stop_bits = UART_STOP_BITS_1, //停止位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE //硬件流控
    };
    //配置串口参数
    uart_param_config(uart_num, &uart_config);
    //设置串口引脚
    uart_set_pin(uart_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    //安装驱动程序
    uart_driver_install(uart_num, 1024 * 2, 0, 0, NULL, 0);
}

void uart_print(int uart_num,char *data){
    uart_write_bytes(uart_num, data, strlen(data));
}
void printf_uart(int uart_num, const char *format, ...){
    char *p = (char *)malloc(256);
    va_list args;
    va_start(args, format);
    vsprintf(p, format, args);
    va_end(args);
    uart_print(uart_num,p);
    free(p);
}