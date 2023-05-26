#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <driver/gpio.h>

#define LED_PIN GPIO_NUM_2

static const char *TAG = "ESP32_LED_Control";

/* Configura el pin del LED como salida */
void setup_gpio()
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}

/* Manejador de la solicitud GET para la página raíz */
esp_err_t root_get_handler(httpd_req_t *req)
{
    const char *html_response =
        "<html><head><title>Prueba 1</title></head>"
        "<body><h1>Prueba 1</h1>"
        "<button type=\"button\">Botón</button>"
        "</body></html>";

    httpd_resp_send(req, html_response, strlen(html_response));
    return ESP_OK;
}

/* Manejador de la solicitud POST para controlar el LED */
esp_err_t led_post_handler(httpd_req_t *req)
{
    gpio_set_level(LED_PIN, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(LED_PIN, 0);

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

/* Configura y arranca el servidor HTTP */
httpd_handle_t start_webserver()
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Configurando servidor HTTP en el puerto %d", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t root_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_get_handler,
            .user_ctx = NULL};

        httpd_uri_t led_uri = {
            .uri = "/led",
            .method = HTTP_POST,
            .handler = led_post_handler,
            .user_ctx = NULL};

        httpd_register_uri_handler(server, &root_uri);
        httpd_register_uri_handler(server, &led_uri);

        ESP_LOGI(TAG, "Servidor HTTP iniciado correctamente");
        return server;
    }

    ESP_LOGI(TAG, "Error al iniciar el servidor HTTP");
    return NULL;
}

/* Manejador de eventos de Wi-Fi */
esp_err_t wifi_event_handler(void *ctx, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Conectando a la red Wi-Fi...");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(TAG, "Conectado a la red Wi-Fi");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        char ip_address[16];
        sprintf(ip_address, IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Dirección IP obtenida: %s", ip_address);
        start_webserver();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "Desconectado de la red Wi-Fi. Intentando reconectar...");
        esp_wifi_connect();
    }

    return ESP_OK;
}

/* Configura la conexión Wi-Fi */
void wifi_setup()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t sta_config = {
        .sta = {
            .ssid = "Flia. Roman",
            .password = "micolas1723",
            .bssid_set = false},
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main()
{
    setup_gpio();
    wifi_setup();
}
