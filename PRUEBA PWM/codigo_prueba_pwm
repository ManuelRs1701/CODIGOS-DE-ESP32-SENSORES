#include <stdio.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define POT_PIN 34
#define POT_ADC_CHANNEL ADC1_CHANNEL_6
#define LED_PIN 12
#define LED_CHANNEL LEDC_CHANNEL_0
#define LED_TIMER LEDC_TIMER_0
#define LED_RESOLUTION LEDC_TIMER_12_BIT
#define LED_FREQUENCY 5000 // Frecuencia en Hz del LED PWM
#define MAX_ADC_VALUE 4095 // El valor máximo del ADC de 12 bits
#define MAX_DUTY_CYCLE 4095 // El valor máximo del ciclo de trabajo del LED PWM

void read_pot_task(void *pvParameters);

void app_main()
{
    // Configurar el LED PWM
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LED_RESOLUTION,
        .freq_hz = LED_FREQUENCY,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LED_TIMER,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel = LED_CHANNEL,
        .duty = 0,
        .gpio_num = LED_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LED_TIMER
    };
    ledc_channel_config(&ledc_channel);

    // Inicializar el ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(POT_ADC_CHANNEL, ADC_ATTEN_DB_11);

    // Crear tarea para leer el valor del potenciómetro
    xTaskCreate(read_pot_task, "read_pot_task", 2048, NULL, 5, NULL);
}

void read_pot_task(void *pvParameters)
{
    uint32_t adc_value = 0;
    float pot_value = 0.0;
    uint32_t duty_cycle = 0;

    while (1)
    {
        // Leer el valor del ADC
        adc_value = adc1_get_raw(POT_ADC_CHANNEL);

        // Convertir el valor del ADC a un valor de voltaje entre 0 y 3.3V
        pot_value = ((float)adc_value / MAX_ADC_VALUE) * 3.3;

        // Calcular el ciclo de trabajo del LED PWM
        duty_cycle = (uint32_t)(pot_value * MAX_DUTY_CYCLE / 3.3);

        // Actualizar el ciclo de trabajo del LED PWM
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL, duty_cycle);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL);

        // Imprimir el valor del potenciómetro en la consola
        printf("Valor del potenciómetro: %.2fV\n", pot_value);

        // Esperar 50ms antes de leer de nuevo el potenciómetro
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
