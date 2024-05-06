#pragma once

#include "esp_mac.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "iostream"
#include <algorithm>
#include <cstring>

#define MAC_ADDR_BYTE_LENGTH 6
#define MAC_ADDR_CS_LEN     18
#define WIFI_HANDLER_MAX_SSID_LENGTH 32
#define WIFI_HANDLER_MAX_PWD_LENGHT 64
#define WIFI_LOG_TAG "WIFI_HANDLER"

namespace wifi
{

    class WifiHandler final {

    public:
        /* Constructor for WifiHandler */
        WifiHandler();
        /* Destructor for WifiHandler */
        ~WifiHandler() = default;

        /* Initialization function for WifiHandler instace. Must be called before wifi_handler_start()! */
        esp_err_t wifi_handler_init();
        /* Function to begin connection to wifi. Must be called after wifi_handler_start()*/
        esp_err_t wifi_handler_start();

        enum class wifi_state_t {
            NOT_INITIALISED,
            INITIALISED,
            READY_TO_CONNECT,
            CONNECTING,
            WAITING_FOR_IP,
            CONNECTED,
            DISCONNECTED,
            ERROR
        };

        private:
            char mac_address_cstr[MAC_ADDR_CS_LEN]{};
            static constexpr char *ssid = {"CUMYNET_REP1"};
            static constexpr char *password = {"123456789a"};
            SemaphoreHandle_t wifi_init_mutex;
            esp_err_t _init_nvs_partition();
            esp_err_t _get_mac_address();
            esp_err_t  _init();
            //esp_err_t _set_wifi_credentials();
            esp_err_t _start();
    };
};


