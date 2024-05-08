#include "include/WifiHandler.hpp"


namespace wifi
{

    WifiHandler::WifiHandler()
    {
        wifi_init_mutex = xSemaphoreCreateMutex();
        wifi_state = wifi_state_t::NOT_INITIALISED;
    }

    esp_err_t WifiHandler::_init_nvs_partition() {
        xSemaphoreTake(this->wifi_init_mutex, pdMS_TO_TICKS(10));
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        xSemaphoreGive(this->wifi_init_mutex);
        return ret;
    }

    esp_err_t WifiHandler::_get_mac_address()
    {
        xSemaphoreTake(this->wifi_init_mutex, pdMS_TO_TICKS(10));

        /* base MAC address, with length of 6 bytes */
        uint8_t mac_address_bytes[MAC_ADDR_BYTE_LENGTH] = {};

        /* Query the factory-programmed MAC address of the device */
        esp_err_t ret = esp_efuse_mac_get_default(mac_address_bytes);

        if(ret != ESP_OK){
            ESP_LOGE(WIFI_LOG_TAG, "unable to query mac address from EFUSE");
            return ret;
        }

        snprintf(static_cast<char *>(this->mac_address_cstr), MAC_ADDR_CS_LEN, "%02x:%02x:%02x:%02x:%02x:%02x",
                 mac_address_bytes[0],
                 mac_address_bytes[1],
                 mac_address_bytes[2],
                 mac_address_bytes[3],
                 mac_address_bytes[4],
                 mac_address_bytes[5]);
        xSemaphoreGive(this->wifi_init_mutex);

        return ret;
    }

    esp_err_t WifiHandler::_init() {

        esp_err_t ret = ESP_OK;

        /* Custom pre-init phase */
        ret = _init_nvs_partition();
        if(ret != ESP_OK) {
            ESP_LOGE(WIFI_LOG_TAG, "%s", esp_err_to_name(ret));
            return ret;
        }
        std::cout << "NVS-partition initialized." << std::endl;

        ret = _get_mac_address();
        if(ret != ESP_OK) {
            ESP_LOGE(WIFI_LOG_TAG, "%s", esp_err_to_name(ret));
            return ret;
        }
        std::cout << "Preprogrammed mac-address obtained: " << mac_address_cstr << std::endl;

        /* 1. init phase */
        if(xSemaphoreTake(wifi_init_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGE(WIFI_LOG_TAG, "unable to obtain 'wifi_init_mutex'");
            return ESP_FAIL;
        }
        ret = esp_netif_init();
        if(ret != ESP_OK){
            ESP_LOGE(WIFI_LOG_TAG, "%s", esp_err_to_name(ret));
            return ret;
        }
        xSemaphoreGive(wifi_init_mutex);

        if(xSemaphoreTake(wifi_init_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGE(WIFI_LOG_TAG, "unable to obtain 'wifi_init_mutex'");
            return ESP_FAIL;
        }        ret = esp_event_loop_create_default();
        if(ret != ESP_OK){
            ESP_LOGE(WIFI_LOG_TAG, "%s", esp_err_to_name(ret));
            return ret;
        }
        xSemaphoreGive(wifi_init_mutex);

        if(xSemaphoreTake(wifi_init_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGE(WIFI_LOG_TAG, "unable to obtain 'wifi_init_mutex'");
            return ESP_FAIL;
        }
        [[maybe_unused]] esp_netif_t *p_netif = esp_netif_create_default_wifi_sta();
        xSemaphoreGive(wifi_init_mutex);

        /* 2. config phase */
        if(xSemaphoreTake(wifi_init_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGE(WIFI_LOG_TAG, "unable to obtain 'wifi_init_mutex'");
            return ESP_FAIL;
        }
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ret = esp_wifi_init(&cfg);
        if(ret != ESP_OK) {
            ESP_LOGE(WIFI_LOG_TAG, "%s", esp_err_to_name(ret));
            return ret;
        }
        xSemaphoreGive(wifi_init_mutex);

        if(xSemaphoreTake(wifi_init_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGE(WIFI_LOG_TAG, "unable to obtain 'wifi_init_mutex'");
            return ESP_FAIL;
        }

        ret = esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&event_handler,nullptr,nullptr);
        if(ret != ESP_OK) {
            ESP_LOGE(WIFI_LOG_TAG, "%s", esp_err_to_name(ret));
            return ret;
        }

        ret = esp_event_handler_instance_register(IP_EVENT,ESP_EVENT_ANY_ID,&event_handler,nullptr,nullptr);
        if(ret != ESP_OK) {
            ESP_LOGE(WIFI_LOG_TAG, "%s", esp_err_to_name(ret));
            return ret;
        }
        xSemaphoreGive(wifi_init_mutex);

        if(xSemaphoreTake(wifi_init_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGE(WIFI_LOG_TAG, "unable to obtain 'wifi_init_mutex'");
            return ESP_FAIL;
        }

        wifi_config_t wifi_config;
        memcpy(wifi_config.sta.ssid, wifi::WifiHandler::ssid, strlen(wifi::WifiHandler::ssid) + 1);
        memcpy(wifi_config.sta.password, wifi::WifiHandler::password, strlen(wifi::WifiHandler::password) + 1);

        ESP_LOGI(WIFI_LOG_TAG, "%s %d", wifi_config.sta.ssid, __LINE__);
        ESP_LOGI(WIFI_LOG_TAG, "%s %d", wifi_config.sta.password, __LINE__);

        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        wifi_config.sta.pmf_cfg.capable = true;
        wifi_config.sta.pmf_cfg.required = false;

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        xSemaphoreGive(wifi_init_mutex);

        wifi_state = wifi_state_t::INITIALISED;
        std::cout << " WifiHandler successfully initialized." << std::endl;

        return ret;
    }

    esp_err_t WifiHandler::wifi_handler_init() {

        return _init();
    }

    esp_err_t WifiHandler::_start() {
        esp_err_t ret = esp_wifi_start();
        return ret;
    }

    esp_err_t WifiHandler::wifi_handler_start() {
        if(wifi_state != wifi_state_t::INITIALISED){
            return ESP_FAIL;
        }
        return _start();
    }

    static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
    {
        /* int32_t -> enum */
        auto wifi_event_num = static_cast<wifi_event_t>(event_id);

        switch (wifi_event_num) {
            case WIFI_EVENT_STA_START: {
                ESP_LOGI(WIFI_LOG_TAG, "Calling esp_wifi_connect...");
                esp_wifi_connect();
                break;
            }
            case WIFI_EVENT_STA_DISCONNECTED: {
                vTaskDelay(pdMS_TO_TICKS(5000));
                ESP_LOGI(WIFI_LOG_TAG, "reconnecting to AP...");
                esp_wifi_connect();
                break;
            }
            default: {
                break;
            }

        }
    }

    static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
    {
        auto ip_event_num = static_cast<ip_event_t>(event_id);

        switch (ip_event_num) {
            case IP_EVENT_STA_GOT_IP: {
                auto *event = static_cast<ip_event_got_ip_t*>(event_data);
                ESP_LOGI(WIFI_LOG_TAG, "Connected to AP. IP-address: " IPSTR, IP2STR(&event->ip_info.ip));
                break;
            }
            case IP_EVENT_STA_LOST_IP: {
                ESP_LOGI(WIFI_LOG_TAG, "%d", IP_EVENT_STA_LOST_IP);
                break;
            }
            default: {
                break;
            }

        }
    }

    void WifiHandler::event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
    {
        if(event_base == WIFI_EVENT) {
            return wifi_event_handler(arg, event_base, event_id, event_data);
        }
        else if(event_base == IP_EVENT) {
            ip_event_handler(arg, event_base, event_id, event_data);
        }
        else {
            ESP_LOGW(WIFI_LOG_TAG, "unknown event arisen");
        }
    }

} // namespace wifi
