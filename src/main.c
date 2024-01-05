#include <stdio.h>
#include "driver/uart.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/dac.h"
#include  "uart.h"
#include "timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freeRTOS/timers.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_tls.h"
#include "esp_log.h"
#include "esp_flash.h"
#include "esp_partition.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_flash_spi_init.h"

#define SSID "BGD"
#define PSW "123456bgd"


int MIN(int a, int b) {
    return a < b ? a : b;
}

esp_netif_t* wifi_init_softap(void){
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t* esp_netif_ap = esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = SSID,
            .ssid_len = strlen(SSID),
            .password = PSW,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    return esp_netif_ap;
}

esp_netif_t* wifi_init_sta(void){
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();//初始化NVS
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());//初始化网络接口
    ESP_ERROR_CHECK(esp_event_loop_create_default());//创建默认事件循环
    esp_netif_t*esp_netif_sta = esp_netif_create_default_wifi_sta();//创建默认wifi接口
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
     wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = SSID,
            .password = PSW,
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .failure_retry_cnt = 5,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
            * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    return esp_netif_sta;
}

static esp_err_t index_html_handler(httpd_req_t *req){


    const char* html = "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP32 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP32!</h1>\
    <h3>CO2:  </h3>\
    <h3>TVOC:  </h3>\
    <h3>max_CO2:  </h3>\
  </body>\
</html>";
    httpd_resp_send(req, html, strlen(html));
    return ESP_OK;
}

static esp_err_t hello_get_handler(httpd_req_t *req){
    httpd_resp_send(req, "Hello World!", 13);
    return ESP_OK;
}

static esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        /* Log data received */
        ESP_LOGI("TAG", "=========== RECEIVED DATA ==========");
        ESP_LOGI("TAG", "%.*s", ret, buf);
        ESP_LOGI("TAG", "====================================");
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI("TAG", "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI("TAG", "Registering URI handlers");
        
    
        static const httpd_uri_t echo = {
            .uri       = "/echo",
            .method    = HTTP_POST,
            .handler   = echo_post_handler,
            .user_ctx  = NULL
        };

        static const httpd_uri_t hello = {
            .uri       = "/hello",
            .method    = HTTP_GET,
            .handler   = hello_get_handler,
            .user_ctx  = NULL
        };

        static const httpd_uri_t index_html = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = index_html_handler,
            .user_ctx  = NULL
        };


        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &index_html);
        httpd_register_uri_handler(server, &echo);
        return server;
    }

    ESP_LOGI("TAG", "Error starting server!");
    return NULL;
}

//串口回调函数
void callback(void)
{
    uart_print(0,"hello world\r\n");
}
//LED闪烁任务
void LED_task(void *arg)
{
    for(;;)
    {
        gpio_set_level(GPIO_NUM_2,1);
        vTaskDelay(100);
        gpio_set_level(GPIO_NUM_2,0);
        vTaskDelay(100);
    }
}

//初始化外置flash
esp_flash_t * flash_init(void){
    const spi_bus_config_t bus_config = {
        .mosi_io_num = 23,
        .miso_io_num = 19,
        .sclk_io_num = 18,
        .quadwp_io_num = 22,
        .quadhd_io_num = 21,
    };
    const esp_flash_spi_device_config_t device_config = {
        .host_id = SPI2_HOST,
        .cs_id = 0,
        .cs_io_num = 5,
        .io_mode = SPI_FLASH_DIO,
        .freq_mhz = 40,
    };
    ESP_LOGI("TAG", "Initializing external SPI Flash...");
        
}   


void app_main() {
    uint32_t ret;
    //初始化串口0
    uart_init(0,115200);
    //初始化定时器
    gptimer_handle_t timer1=NULL;
    timer_init(timer1,1000000);
    gptimer_start(timer1);
    //初始化GPIO
    gpio_config_t  io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask =  (1ULL<<GPIO_NUM_2);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    gpio_set_level(GPIO_NUM_2,1);
    /*单次转换ADC*/
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_0,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_CHANNEL_6, &config));


    /*连续转换ADC*/
    //初始化ADC
    // adc_continuous_handle_t adc_handle = NULL;
    // uint16_t adc_data[256];
    // adc_continuous_handle_cfg_t adc_config = {
    //     .max_store_buf_size = 1024,
    //     .conv_frame_size = 256,
    // };
    // ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_handle)); 
    // adc_continuous_config_t dig_cfg = {
    //     .sample_freq_hz = 20000,//采样频率
    //     .conv_mode = ADC_CONV_SINGLE_UNIT_1,//单次采样
    //     .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,//输出格式
    // };
    // adc_digi_pattern_config_t adc_pattern[1] = {
    //     {
    //         .atten = ADC_ATTEN_DB_0,//衰减
    //         .channel = ADC1_CHANNEL_7,//通道
    //         .unit = ADC_UNIT_1,//选用ADC
    //         .bit_width = ADC_BITWIDTH_DEFAULT,//位宽
    //     },
    // };
    // dig_cfg.adc_pattern = adc_pattern;
    // ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &dig_cfg));
    // ESP_ERROR_CHECK(adc_continuous_start(adc_handle));
    //DAC
    dac_output_enable(DAC_CHANNEL_1);
    dac_output_voltage(DAC_CHANNEL_1, 200);

    //wifi
    // wifi_init_softap();
    // esp_netif_t *esp_netif_sta = wifi_init_sta();
    esp_netif_t *esp_netif_ap = wifi_init_softap();
    esp_netif_set_default_netif(esp_netif_ap);
    //httpd
    static httpd_handle_t server = NULL;
    server = start_webserver();

    /*创建LED闪烁任务*/
    xTaskCreate(LED_task, "LED_task", 1024, NULL, 10, NULL);
    /*创建软件定时器*/
    TimerHandle_t timer = xTimerCreate("hello_world", 100, pdTRUE, NULL, callback);
    // xTimerStart(timer, 1000/portTICK_PERIOD_MS);
    //循环发送数据
    while (1) {

        /*单次转换ADC*/
        adc_oneshot_read(adc1_handle,ADC1_CHANNEL_7,&ret);
        // printf_uart(0,"adc_data:%d\r\n",ret);
        /*连续转换ADC*/
        // adc_continuous_read(adc_handle, adc_data, 256,&ret, ADC_MAX_DELAY);
        
        vTaskDelay(100);

    }
}