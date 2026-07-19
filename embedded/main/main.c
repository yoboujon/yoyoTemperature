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

#include "dht.h"

#define THERM_R0 100000.0f
#define THERM_BETA 4255.0f
#define KELVIN 273.15f
#define THERM_T0 (25.0f + KELVIN)

typedef struct __attribute__((packed)) yoyotemp_packet {
    uint16_t humidity;
    float temp;
} yoyotemp_packet_t;

static yoyotemp_packet_t pkt;

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

    // ADC shit
    adc_oneshot_unit_handle_t adc;
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_cali_handle_t cali_handle;
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = ADC_CHANNEL_6,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_oneshot_chan_cfg_t cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc, ADC_CHANNEL_6, &cfg));
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));

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

    uint32_t sample = 0;
    int16_t humidity, unused;
    int raw, voltage_mv;
    float temperature, avg_temp, total_temp = 0;

    xEventGroupWaitBits(wifi_event_group, IP_ASSIGNED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    while (1)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc, ADC_CHANNEL_6, &raw));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw, &voltage_mv));

        const float voltage = voltage_mv / 1000.0f;
        const float resistor = (3.3f - voltage) * THERM_R0 / voltage;
        const float tempK = 1.0f / (1.0f / THERM_T0 + logf(resistor / THERM_R0) / THERM_BETA);
        temperature = tempK - KELVIN;
        total_temp += temperature;
        sample++;
        avg_temp = total_temp / sample;

        if (dht_read_data(DHT_TYPE_DHT11, GPIO_NUM_22, &humidity, &unused) == ESP_OK)
        {
            printf("Humidity: %.1f%%   Average (%lu): %.3f°C   Actual: %.3f°C\n", humidity / 10.0f, sample, avg_temp, temperature);
        }
        else
        {
            printf("Thermistor: %.3f°C\n", temperature);
        }

        if(sample == 600) {
            pkt.humidity = humidity;
            pkt.temp = avg_temp;
            send_udp_packet();
            sample = 0;
            total_temp = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}