#ifndef MOTORS_H_
#define MOTORS_H_

#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>


esp_err_t set_motors(void);
esp_err_t motorA_adelante(void);
esp_err_t motorA_atras(void);
esp_err_t motorB_adelante(void);
esp_err_t motorB_atras(void);

#endif 