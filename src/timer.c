#include "timer.h"

void timer_init(gptimer_handle_t timer,int freq){
    //配置定时器参数
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = freq, // 1MHz, 1 tick=1us
    };
    //初始化定时器
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer));
}
/*代码不可用，可能原因是回调函数传参错误导致配置不成功，开启时钟报错*/
/*因此最好手动配置回调，格式如下*/
// void bind_timer_callback(gptimer_handle_t timer,int arr,CallbackFunction callback){
//     gptimer_event_callbacks_t cbs = {
//         .on_alarm = callback,
//     };
//     ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer, &cbs, NULL));
//     ESP_ERROR_CHECK(gptimer_enable(timer));
//      gptimer_alarm_config_t alarm_config1 = {
//         .reload_count = 0,
//         .alarm_count = arr, // period = 1s
//         .flags.auto_reload_on_alarm = true,
//     };
//     ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &alarm_config1));
//     // ESP_ERROR_CHECK(gptimer_start(timer));
   
// }

