#include <stdio.h>

#include "driver/pcnt.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_http_server.h"
#include "nvs_flash.h"

// ------------------------------------------
// AIS includes
// ------------------------------------------
#include "AIS_config.h"
#include "credentials.h"

// ------------------------------------------
// data struct definition:
// ------------------------------------------

// data struct definition:
typedef struct {
    uint16_t cnt; // temp 32 bit 
    uint16_t light;
    uint8_t temperature;
}data_pack;


// ------------------------------------------
// Global variables:
// ------------------------------------------

// measurements:

uint8_t latestMeasurement = 0;
data_pack measurements [2];

// freeRTOS:

// queue:
static QueueHandle_t data_queue;

// ------------------------------------------
// networking:
// ------------------------------------------

// WiFi:

// handlers and variables:
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_count = 0;

// event handler:
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
    
        esp_wifi_connect();
    
    }else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){
        
        if(s_retry_count < MAX_RETRY_COUNT){
            esp_wifi_connect();
            s_retry_count ++;
            printf("Retry to connect to the AP \n\r");
        }else{
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        printf("Connect to the AP fail \n\r");

    }else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("Got IP: " IPSTR "\n\r", IP2STR(&event->ip_info.ip));
        s_retry_count = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    
    }
}

//init:
void wifi_init_sta(){
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &event_handler,
            NULL,
            &instance_any_id
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &event_handler,
            NULL,
            &instance_got_ip
        )
    );


    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASSPHRASE,
            
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            
            .pmf_cfg = {
                .capable = true,
                .required = false,
            }
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    printf("Wifi initialization finished. \n\r");

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    if(bits & WIFI_CONNECTED_BIT) {
        printf("ESP connected to AP: %s \n\r", SSID);
    }else if(bits & WIFI_FAIL_BIT){
        printf("Failed to connect AP: %s \n\r", SSID);
    }else{
        printf("Unexpected event! \n\r");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
    return;
}

// HTTP:

/* URI handler functions: */
esp_err_t get_handler(httpd_req_t *req){
    const char resp[] = "URI GET Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t post_handler(httpd_req_t *req){
    return ESP_OK;
}

esp_err_t get_default_handler(httpd_req_t *req){
    printf("Someone accessed home! \n\r");
    httpd_resp_send(req, "ESP32 node is online... \n\r", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_measure_handler(httpd_req_t *req){
    
    char meas_msg_json[100] = "";
    
    sprintf(meas_msg_json, "{ \"pulses\" : %d, \"light\" : %d, \"temperature\" : %d}\n\r",
                    measurements[latestMeasurement].cnt,
                    measurements[latestMeasurement].light,
                    measurements[latestMeasurement].temperature
                    );
    printf(meas_msg_json);

    httpd_resp_send(req, meas_msg_json, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

/* URI handlers structure for GET/POST /uri */
httpd_uri_t uri_get = {
    .uri      = "/uri",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_post = {
    .uri      = "/uri",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_get_default = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = get_default_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_get_measure = {
    .uri      = "/measure",
    .method   = HTTP_GET,
    .handler  = get_measure_handler,
    .user_ctx = NULL
};

/* start stop functions */
httpd_handle_t startServer(){
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server_handle = NULL;

    if(httpd_start(&server_handle, &server_config) == ESP_OK){
        httpd_register_uri_handler(server_handle, &uri_get);
        httpd_register_uri_handler(server_handle, &uri_post);
        httpd_register_uri_handler(server_handle, &uri_get_default);
        httpd_register_uri_handler(server_handle, &uri_get_measure);
    }

    return server_handle;
}

void stopServer(httpd_handle_t server_handle){
    if(server_handle){
        httpd_stop(server_handle);
    }
}

// ------------------------------------------
// init structs:
// ------------------------------------------

// pcnt:
pcnt_config_t pcntInitStruct = {
        .pulse_gpio_num = PCNT_GPIO_NUM_PULSE,
        .ctrl_gpio_num = PCNT_GPIO_NUM_CTRL,
        .lctrl_mode = PCNT_MODE_KEEP,
        .hctrl_mode = PCNT_MODE_KEEP,
        .pos_mode = PCNT_COUNT_INC, 
        .neg_mode = PCNT_COUNT_DIS,
        .counter_h_lim = 0, 
        .counter_l_lim = 0, 
        .unit = PCNT_UNIT_0,
        .channel = PCNT_CHANNEL_0
    };

// i2c:
i2c_config_t init_i2c = {
    
        .mode = I2C_MODE_MASTER,
        
        .sda_io_num = I2C_SDA_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        
        .scl_io_num = I2C_SCL_PIN,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,

        .master.clk_speed = I2C_SCL_FREQ
    };

// gpio:
gpio_config_t init_gpio = {

    .pin_bit_mask = GPIO_SELECTED_PIN_MSK,
    .mode = GPIO_MODE_OUTPUT,
    .intr_type = GPIO_INTR_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .pull_up_en = GPIO_PULLUP_DISABLE,
};

// ------------------------------------------
// init functions:
// ------------------------------------------

// pcnt:
void pcntInit(){

    pcnt_unit_config(&pcntInitStruct);
    pcnt_intr_disable(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);
    return;
}

// i2c:
void I2CInit(){
    
    i2c_param_config(I2C_NUM_0, &init_i2c);
    i2c_driver_install(I2C_NUM_0, init_i2c.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);   
    return;
}

// veml7700:
void VEMLInit(){

    uint8_t init_command[3] = {0,0,0};

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, (VEML_ADDR | I2C_MASTER_WRITE), true);

    i2c_master_write(cmd, init_command, 3, true);

    i2c_master_stop(cmd);

    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);

    return;
}

// gpio (pump):
void gpioInit(){
    gpio_config(&init_gpio);
    return;
}

// ------------------------------------------
// sensor read functions:
// ------------------------------------------

// veml7700
void readVEML (uint8_t* data, uint8_t size){

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (VEML_ADDR | I2C_MASTER_WRITE), true);
    i2c_master_write_byte(cmd, VEML_COMMAND, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (VEML_ADDR | I2C_MASTER_READ), true);
    i2c_master_read(cmd, data, 1, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, data+1, I2C_MASTER_NACK);
    
    i2c_master_stop(cmd);

    // esp_err_t error2 = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return;
}

// lm-75
void readLM (uint8_t* data, uint8_t size){

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, (LM75_ADDR | I2C_MASTER_READ), true);

    i2c_master_read(cmd, data, size, I2C_MASTER_ACK);

    i2c_master_stop(cmd);

    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);

    i2c_cmd_link_delete(cmd);

    return;
}

// ------------------------------------------
// processing functions:
// ------------------------------------------

data_pack xAverage(data_pack* input, uint8_t size){
    
    data_pack result;
    
    uint32_t cnt = 0;
    uint32_t light = 0;
    uint32_t temperature = 0;

    for(uint8_t i = 0; i < size; i ++){
        cnt += input[i].cnt;
        light += input[i].light;
        temperature += input[i].temperature;
    }
    
    // rounds are small enough
    result.cnt = cnt / size;
    result.light = light / size;
    result.temperature = temperature / size;

    return result;
}

// ------------------------------------------
// Tasks:
// ------------------------------------------

// executes a measurement cycle
TaskHandle_t measureTaskHandle;
static void measureTask (void* vParam){
    
    uint8_t lm_data[2] = {0,0};
    uint8_t veml_data[2] = {0,0};
    uint16_t pCounter = 0;
    data_pack data;

    printf("measure scheduled \n\r"); 

    while (1)
    {
        
        pcnt_counter_clear(PCNT_UNIT_0);
        vTaskDelay(100/portTICK_PERIOD_MS);                         // 100 ms
        pcnt_get_counter_value(PCNT_UNIT_0, (int16_t*)&pCounter);
        
        readLM(lm_data, 2);
        readVEML(veml_data, 2);

        // -------------------------------------
        data.cnt = pCounter;
        data.temperature = lm_data[0];
        data.light = (256*veml_data[1]+veml_data[0]);
        // -------------------------------------

        // temporary:
        // printf("measured values %d, %d, %d \n\r", temperature, light, cnt);
        if(xQueueSend(data_queue, &data, 0) != pdTRUE){
            printf("Failed to wrtie data queue! Queue is full ... \r\n");
        }

        // vTaskDelay(meas_TIME_MS/portTICK_PERIOD_MS);
        vTaskSuspend(NULL); // suspend itself
    }  
}

// SW timer callback function
static void resumeMeasure(){
    
    xTaskResumeFromISR(measureTaskHandle);

    return;
}

// schedules the measurement tasks
TaskHandle_t processTaskHandle;
static void processTask(void* vParam){

    data_pack final_data;

    uint8_t idx = 0;
    data_pack raw_data_array[10];
    printf("process scheduled \n\r");

    while(1){
        if(idx > (meas_LENGTH-1)){ 
            
            idx = 0;
            
            final_data = xAverage(raw_data_array, meas_LENGTH);

            if(latestMeasurement){
                measurements[0] = final_data;
                latestMeasurement = 0;
            }else{
                measurements[1] = final_data;
                latestMeasurement = 1;
            }
            
        }else{
            // read from queue
            if(xQueueReceive(data_queue, (void*)&(raw_data_array[idx]), 0) == pdTRUE){
                idx++;
            }
            // if cant receive, queue is empty
            vTaskDelay(process_DELAY_MS/portTICK_PERIOD_MS);
        }
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    printf("ESP WiFi mode station \n\r");
    wifi_init_sta();

    startServer();

    gpioInit();
    pcntInit();
    I2CInit();
    VEMLInit();

    data_queue = xQueueCreate(queue_LENGTH, queue_LENGTH*sizeof(data_pack));

    TimerHandle_t measure_timer = xTimerCreate(
                                        "measure scheduler timer",
                                        pdMS_TO_TICKS(meas_TIME_MS),
                                        pdTRUE,
                                        (void*)0,
                                        resumeMeasure
                                    );

    if(measure_timer == NULL){
        printf("Timer initialization failed \n\r");
    }else{
        vTaskDelay(1000/portTICK_PERIOD_MS);
        
        xTimerStart(measure_timer, portMAX_DELAY);

        printf("Timer Initialized! \r\n");
    }


    xTaskCreate(
        processTask,                       // tsk function
        "process task",                    // task name 
        2048,                              // stack size
        NULL,                              // parameter list
        processTsk_PRIORITY,               // priority
        &processTaskHandle                 // task handle address
    );


    xTaskCreate(
        measureTask,                    // tsk function
        "measure task",                 // task name 
        2048,                           // stack size
        NULL,                           // parameter list
        measureTsk_PRIORITY,            // priority
        &measureTaskHandle              // task handle address
    );
    
    
    
    // scheduler already running. 

    // suspend own task.
    vTaskSuspend(NULL);
    
}

