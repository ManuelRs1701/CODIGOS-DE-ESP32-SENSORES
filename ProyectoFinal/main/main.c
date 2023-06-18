#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "motors.h"
#include "sensores.h"
#include "server.h"


void app_main(void)
{
    //configuracion de los motores
    set_motors();

    //configuracion de los sensores a medir
    set_temperature();      
    set_presion();
    set_corriente();

    //se inicia la memoria no volatil
    nvs_flash_init();

    //configuracion y conexion del wifi
    wifi_server_setup();
    

}
