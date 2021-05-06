#ifndef AIS_CONFIG_H
#define AIS_CONFIG_H

#include "credentials.h"

// ------------------------------------------
// defines:
// ------------------------------------------


#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_SDA_PIN GPIO_NUM_18
#define I2C_SCL_PIN GPIO_NUM_19
#define I2C_SCL_FREQ 20000                              // 20kHz signal

#define LM75_ADDR (uint8_t)0x90
#define VEML_ADDR (uint8_t)0x20
#define VEML_COMMAND (uint8_t)0x04

#define GPIO_SELECTED_PIN GPIO_NUM_2                    // = 2
#define GPIO_SELECTED_PIN_MSK (1ULL << GPIO_SELECTED_PIN)

#define PCNT_GPIO_NUM_CTRL GPIO_NUM_23
#define PCNT_GPIO_NUM_PULSE GPIO_NUM_22


#define measureTsk_PRIORITY (tskIDLE_PRIORITY + 2)
#define processTsk_PRIORITY (tskIDLE_PRIORITY + 1)

#define meas_TIME_MS 10000                          // 0,5 sec
#define process_DELAY_MS 10                         // data processing tasks delay value in miliseconds
#define meas_LENGTH 10                              // the amount of measurements one cycle contains (average will be the measured value)
#define queue_LENGTH  3                             


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define MAX_RETRY_COUNT 30




#endif