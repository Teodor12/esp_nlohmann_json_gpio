#pragma once

#include <algorithm>
#include <functional>
#include <cstring>
#include <iostream>

#include "esp_mac.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"

namespace wifi
{
    #define MAC_ADDR_BYTE_LENGTH 6
    #define MAC_ADDR_CS_LEN     18
    #define WIFI_LOG_TAG "WIFI_HANDLER"

    using on_wifi_connected_callback = std::function<void(void)>;
    using on_wifi_disconnected_callback = std::function<void(void)>;

    class WifiHandler final {

    public:
        /* Constructor for WifiHandler */
        explicit WifiHandler();

        /* Destructor for WifiHandler */
        ~WifiHandler() = default;

        /* Initialization function for WifiHandler instace. Must be called before wifi_handler_start()! */
        esp_err_t wifi_handler_init(const on_wifi_connected_callback& on_connected_cb, const on_wifi_disconnected_callback& on_disconnected_cb);

        /* Function to begin connection to wifi. Must be called after wifi_handler_start()*/
        esp_err_t wifi_handler_start();


    private:
        char mac_address_cstr[MAC_ADDR_CS_LEN]{};
        std::string ssid;
        std::string password;
        bool disconnected_cb_requested;
        enum class wifi_state_t {
            NOT_INITIALISED,
            INITIALISED,
            WAITING_FOR_IP,
            CONNECTED,
            DISCONNECTED
        } wifi_state;

        SemaphoreHandle_t wifi_init_mutex{};
        SemaphoreHandle_t wifi_state_mutex{};

        on_wifi_connected_callback f_connected;
        on_wifi_disconnected_callback f_disconnected;

        esp_err_t _init_wifi_mutexes();
        esp_err_t _init_nvs_partition();
        esp_err_t _get_mac_address();
        esp_err_t _init(const std::function<void(void)>& on_connected_cb, const std::function<void(void)>& on_disconnected_cb);
        esp_err_t _start();
        esp_err_t _set_wifi_credentials();
        esp_err_t _set_wifi_state(wifi_state_t new_state);
        wifi_state_t _get_wifi_state() const;

        friend void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        friend void wifi_event_handler(WifiHandler *wifi_handler, esp_event_base_t event_base, int32_t event_id, void* event_data);
        friend void ip_event_handler(WifiHandler *wifi_handler, esp_event_base_t event_base, int32_t event_id, void* event_data);

    };

} // namespace wifi
