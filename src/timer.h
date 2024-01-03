#ifndef TIMER_H
#define TIMER_H
#include "driver/gptimer.h"
// 定义函数指针类型
typedef void (*CallbackFunction)(void);

//初始化定时器
void timer_init(gptimer_handle_t timer,int freq);
//绑定溢出回调函数
void bind_timer_callback(gptimer_handle_t timer,int arr, CallbackFunction callback);
#endif