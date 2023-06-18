#ifndef SERVER_H_
#define SERVER_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_http_server.h>
#include <esp_event.h>
#include <esp_system.h>  
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_tls.h"
#include <string.h>
#include "driver/gpio.h"
#include <stdio.h>

void wifi_server_setup(void);

#endif 