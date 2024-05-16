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
#define WIFI_LOG_TAG "WIFI_HANDLER"

namespace wifi
{
    enum class wifi_state_t;

    class WifiHandler final {

    public:
        /* Constructor for WifiHandler */
        explicit WifiHandler();

        /* Destructor for WifiHandler */
        ~WifiHandler() = default;

        /* Initialization function for WifiHandler instace. Must be called before wifi_handler_start()! */
        esp_err_t wifi_handler_init();

        /* Function to begin connection to wifi. Must be called after wifi_handler_start()*/
        esp_err_t wifi_handler_start();


    private:
        char mac_address_cstr[MAC_ADDR_CS_LEN]{};
        std::string ssid;
        std::string password;
        wifi_state_t state;
        SemaphoreHandle_t wifi_init_mutex;
        esp_err_t _init_nvs_partition();
        esp_err_t _get_mac_address();
        esp_err_t _init();
        esp_err_t _start();
        esp_err_t _set_wifi_credentials();
        void set_wifi_state(wifi_state_t new_state);
        wifi_state_t get_wifi_state() const;

        friend void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        friend void wifi_event_handler(WifiHandler *wifi_handler, esp_event_base_t event_base, int32_t event_id, void* event_data);
        friend void ip_event_handler(WifiHandler *wifi_handler, esp_event_base_t event_base, int32_t event_id, void* event_data);

    };

    enum class wifi_state_t {
        NOT_INITIALISED,
        INITIALISED,
        WAITING_FOR_IP,
        CONNECTED,
        DISCONNECTED
    };


};

