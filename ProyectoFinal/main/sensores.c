#include "sensores.h"
#include "math.h"

//tag para imprimir en terminal
static const char* TAG = "Main";


//---------------------------------variables para el sensor de presion------------------------------------------------
#define DEFAULT_VREF 1100
#define N_DE_MUESTRAS 20        
double Level, Vout, P, Vs = 4.7;
double aux;
double tolP = -0.27; 
int rho = 997;
double g = 9.8;

//Características del ADC
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_0;     // Canal del ADC EL 0
static const adc_atten_t atten = ADC_ATTEN_DB_0;        // Se va a utilizar una atenuación de señal de 0 dB en el ADC
static const adc_unit_t unit = ADC_UNIT_1;              // Se está utilizando la unidad ADC 1 en el ESP32.


//--------------------------------variables para el sensor de corriente---------------------------------------------
//Pines del I2C (data y clock)
#define SDA_IO GPIO_NUM_21     
#define SCL_IO GPIO_NUM_22                    
//Frecuencia
#define FREQ_HZ (100000)  
//Desabilitamos los buffers
#define TX_BUF_DISABLE (0)              
#define RX_BUF_DISABLE (0)              
#define I2C_RX_BUF_STATE RX_BUF_DISABLE  
#define I2C_TX_BUF_STATE TX_BUF_DISABLE  
//Colocamos que es el controlador numero 0 (por nuestros pines) y modo master
#define I2C_NUM I2C_NUM_0                
#define I2C_MODE I2C_MODE_MASTER         
//No interrupciones
#define I2C_INTR_ALOC_FLAG (0)      

//Sensor
#define MAX_A 30
#define MAX_mV 1000

float I_Calculada = 0;

//Configuracion del I2C
i2c_config_t i2c_cfg = {                     
  .mode = I2C_MODE_MASTER, 
  .sda_io_num = SDA_IO,
  .scl_io_num = SCL_IO,
  .sda_pullup_en = GPIO_PULLUP_DISABLE,
  .scl_pullup_en = GPIO_PULLUP_DISABLE,
  .master.clk_speed = FREQ_HZ,
};

//Configuracion del ads1115
ads1115_t ads1115_cfg = {
  .reg_cfg =  ADS1115_CFG_LS_COMP_MODE_TRAD | // Comparator is traditional
              ADS1115_CFG_LS_COMP_LAT_NON |   // Comparator is non-latching
              ADS1115_CFG_LS_COMP_POL_LOW |   // Alert is active low
              ADS1115_CFG_LS_COMP_QUE_DIS |   // Compator is disabled
              ADS1115_CFG_LS_DR_1600SPS |     // No. of samples to take
              ADS1115_CFG_MS_MODE_SS,         // Mode is set to single-shot
  .dev_addr = 0x48,
};

//---------------------------------------definicion de funciones---------------------------------------------------

//-------------temperatura----------------
//configura el canal adc para leer el sensor de temperatura
esp_err_t set_temperature(void)
{
    //configuracion del ADC1 canal 6 con una atenuacion de 11db
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    //los datos seran leidos en 12 bits
    adc1_config_width(ADC_WIDTH_BIT_12);
    return ESP_OK;
}

//obtiene la lectura del sensor de temperatura
float get_temperature(void)
{
    //obtiene los datos del ADC
    float adc_value = adc1_get_raw(ADC1_CHANNEL_6);
    //conversion a voltaje
    float voltaje = adc_value/650;
    //conversion a temperatura
    float temperatura = voltaje/0.01;

    ESP_LOGE(TAG, "temperatura: %.2f", temperatura);

    return temperatura;
}



//-------------------presion---------------------
//configura el canal adc para leer el sensor de presion
esp_err_t set_presion(void)
{
    //Configuracion del ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel, atten);

    //Caracteristicas del ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    return ESP_OK;
}

float get_presion(void)
{
    //Medición del voltaje del Sensor MPX5050DP
    aux = 0;
    for(int i = 0; i < N_DE_MUESTRAS; i++) {
        uint32_t adc_reading = 0;
        for (int j = 0; j < 10; j++) {
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        }
        adc_reading /= 10;
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        aux += (double)voltage / 1000.0;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    Vout = aux / (double)N_DE_MUESTRAS;

    //Presion en Kpa segun grafica 2 del Datasheet
    P = (Vout - 0.04 * Vs) / (0.018 * Vs) + tolP; //Kpa

    //Impresión de resultados
    printf("Presion: %2.2f kPa\n", P);

    return P;
}



//-------------------corriente-----------------------
esp_err_t set_corriente(void)
{
    ESP_LOGD(TAG,"Starting ADS1115 example..");

    // Setup I2C
    i2c_param_config(I2C_NUM, &i2c_cfg);
    i2c_driver_install(I2C_NUM, I2C_MODE, I2C_RX_BUF_STATE, I2C_TX_BUF_STATE, I2C_INTR_ALOC_FLAG);

    // Setup ADS1115
    ADS1115_initiate(&ads1115_cfg);

    return ESP_OK;
}

float get_corriente(void){

    //extrayendo la raiz cuadrada de la suma de sus infinitos valores 
    //instantaneos (nuestro caso 2000), elevados al cuadrado.

    int16_t bits_ads;

    //Factor de correccion
    float mV_porbit = 0.0625F; 

    float I_eficaz;
    float I_instant;
    float mV_instant;
    float sum_I_instant = 0;

    //muestreo de 500 ciclos
    for (int i = 0; i < 500; i++){
        ADS1115_request_diff_AIN0_AIN1();
        bits_ads = ADS1115_get_conversion();

        mV_instant = bits_ads * mV_porbit;
        I_instant = mV_instant * MAX_A / MAX_mV;
        sum_I_instant += I_instant * I_instant;
    }
    I_eficaz = sqrt(sum_I_instant / 500);
    I_Calculada = I_eficaz * 1.39;                 //valores que cambia con pruebas
    ESP_LOGI(TAG,"corriente.. %f.", I_Calculada);

    return I_Calculada;
}