#ifndef SENSORES_H_
#define SENSORES_H_

#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <esp_log.h>
#include <driver/i2c.h>
#include <math.h>
#include "ADS1115.h"

esp_err_t set_temperature(void);
float get_temperature(void);

esp_err_t set_presion(void);
float get_presion(void);

esp_err_t set_corriente(void);
float get_corriente(void);



#endif 