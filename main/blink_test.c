#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_err.h"

#include "led_strip.h"
#include "esp_http_server.h"

#define WIFI_SSID      "NewHumboldt"
#define WIFI_PASS      "Moby201124g!"

#define RGB_GPIO        48
#define STRIP_GPIO      15
#define NUM_RGB_LEDS    1
#define NUM_STRIP_LEDS  9

static const char *TAG = "MAIN";

static led_strip_handle_t onboard_rgb;
static led_strip_handle_t external_strip;

static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

static void set_onboard_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    led_strip_set_pixel(onboard_rgb, 0, r, g, b);
    led_strip_refresh(onboard_rgb);
}

static void init_leds(void)
{
    led_strip_config_t onboard_config = {
        .strip_gpio_num = RGB_GPIO,
        .max_leds = NUM_RGB_LEDS,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(
        &onboard_config,
        &rmt_config,
        &onboard_rgb
    ));

    led_strip_config_t strip_config = {
        .strip_gpio_num = STRIP_GPIO,
        .max_leds = NUM_STRIP_LEDS,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(
        &strip_config,
        &rmt_config,
        &external_strip
    ));

    led_strip_clear(onboard_rgb);
    led_strip_clear(external_strip);
}

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi disconnected, retrying...");
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void init_wifi(void)
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    ESP_ERROR_CHECK(esp_event_handler_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        NULL
    ));

    ESP_ERROR_CHECK(esp_event_handler_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &wifi_event_handler,
        NULL
    ));

    wifi_config_t wifi_config = {0};
    strcpy((char *)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, WIFI_PASS);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi started");
}

static void onboard_status_task(void *arg)
{
    while (1) {
        EventBits_t bits = xEventGroupGetBits(wifi_event_group);

        if (bits & WIFI_CONNECTED_BIT) {
            set_onboard_rgb(0, 255, 0);   // Green = WiFi connected
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
            set_onboard_rgb(255, 0, 0);   // Red blink = not connected
            vTaskDelay(pdMS_TO_TICKS(300));
            set_onboard_rgb(0, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(300));
        }
    }
}

static void strip_task(void *arg)
{
    while (1) {
        for (int i = 0; i < NUM_STRIP_LEDS; i++) {
            led_strip_set_pixel(external_strip, i, 0, 0, 255);
            led_strip_refresh(external_strip);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay(pdMS_TO_TICKS(500));

        for (int i = 0; i < NUM_STRIP_LEDS; i++) {
            led_strip_set_pixel(external_strip, i, 0, 0, 0);
            led_strip_refresh(external_strip);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static esp_err_t led_on_handler(httpd_req_t *req)
{
    for (int i = 0; i < NUM_STRIP_LEDS; i++) {
        led_strip_set_pixel(external_strip, i, 255, 0, 0); // Red
    }
    led_strip_refresh(external_strip);

    set_onboard_rgb(0, 255, 0);   // Test
    httpd_resp_send(req, "STRIP ON", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t led_off_handler(httpd_req_t *req)
{
    led_strip_clear(external_strip);

    set_onboard_rgb(255, 0, 0);   // Test
    httpd_resp_send(req, "STRIP OFF", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t led_on = {
            .uri = "/on",
            .method = HTTP_GET,
            .handler = led_on_handler,
            .user_ctx = NULL
        };

        httpd_uri_t led_off = {
            .uri = "/off",
            .method = HTTP_GET,
            .handler = led_off_handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server, &led_on);
        httpd_register_uri_handler(server, &led_off);
    }

    return server;
}

void app_main(void)
{
    init_leds();
    init_wifi();
    start_webserver();

    //xTaskCreate(onboard_status_task, "onboard_status_task", 4096, NULL, 5, NULL);
    //xTaskCreate(strip_task, "strip_task", 4096, NULL, 5, NULL);
}
