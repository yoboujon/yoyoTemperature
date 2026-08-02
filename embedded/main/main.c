#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "constants.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"

#include "sht3x.h"

#define SDA_GPIO GPIO_NUM_11
#define SCL_GPIO GPIO_NUM_10
#define I2C_PORT 0
#define SHT3X_ADDR 0x44
#define TIME_DELTA_MS 5000

typedef struct __attribute__((packed)) yoyotemp_packet {
    uint16_t humidity;
    float temp;
} yoyotemp_packet_t;

static yoyotemp_packet_t pkt;
static sht3x_t dev;

static const char *TAG = "wifi";
static EventGroupHandle_t wifi_event_group;
#define IP_ASSIGNED_BIT BIT0
static void event_handler(void* arg, esp_event_base_t event_base,
                           int32_t event_id, void* event_data)
{
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, IP_ASSIGNED_BIT);
    }
}

void send_udp_packet(void)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
        return;

    struct sockaddr_in dest = {
        .sin_family = AF_INET,
        .sin_port = htons(UDP_PORT),
    };

    inet_pton(AF_INET, API_IP, &dest.sin_addr);
    sendto(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *)&dest, sizeof(dest));
    close(sock);
}

void app_main(void)
{
    // I2C shit
    ESP_ERROR_CHECK(i2cdev_init());
    memset(&dev, 0, sizeof(sht3x_t));
    ESP_ERROR_CHECK(sht3x_init_desc(&dev, SHT3X_ADDR, I2C_PORT, SDA_GPIO, SCL_GPIO));
    ESP_ERROR_CHECK(sht3x_init(&dev));

    // Wi-Fi shit
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_cfg);
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();

    // event
    wifi_event_group = xEventGroupCreate();
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL);

    const uint32_t max_sample = (uint32_t)(ceil(6e5/TIME_DELTA_MS));
    uint32_t sample = 0;
    float temperature, humidity, avg_temp, total_temp = 0;

    xEventGroupWaitBits(wifi_event_group, IP_ASSIGNED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    while (1)
    {
        const esp_err_t res = sht3x_measure(&dev, &temperature, &humidity);
        if (res == ESP_OK)
        {
            total_temp += temperature;
            sample++;
            avg_temp = total_temp / sample;
            printf("Humidity: %.1f%%   Average (%lu): %.3f°C   Actual: %.3f°C\n", humidity, sample, avg_temp, temperature);
        }
        else
        {
            printf("Measurement failed: %s (0x%x)\n", esp_err_to_name(res), res);
        }

        if(sample == max_sample) {
            pkt.humidity = (uint16_t)(humidity*10.0);
            pkt.temp = avg_temp;
            send_udp_packet();
            sample = 0;
            total_temp = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(TIME_DELTA_MS));
    }
}