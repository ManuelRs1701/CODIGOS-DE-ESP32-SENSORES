#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc_cal.h"
#include "driver/adc.h"

#define DEFAULT_VREF    1100
#define NO_OF_SAMPLES   20      // Numero de muestras de ADC

double Level, Vout, P, Vs = 4.7;
double aux;
double tolP = -0.27; // Ajusta la medida de presión
int rho = 997;
double g = 9.8;

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_0;     // Canal del ADC
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

void app_main()
{
    //Configuracion del ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel, atten);

    //Caracteristicas del ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    while(1) {
        //Voltaje del Sensor MPC5050DP
        aux = 0;
        for(int i = 0; i < NO_OF_SAMPLES; i++) {
            uint32_t adc_reading = 0;
            for (int j = 0; j < 10; j++) {
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
            }
            adc_reading /= 10;
            uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
            aux += (double)voltage / 1000.0;
            vTaskDelay(pdMS_TO_TICKS(5));
        }
        Vout = aux / (double)NO_OF_SAMPLES;

        //Presion en Kpa segun grafica 2 del Datasheet
        P = (Vout - 0.018 * Vs) / (0.09 * Vs) + tolP; //Kpa

        printf("Presion: %2.2f kPa\n", P);

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
