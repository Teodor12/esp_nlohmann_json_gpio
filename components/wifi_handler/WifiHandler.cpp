#include "include/WifiHandler.hpp"


namespace wifi
{
    WifiHandler::WifiHandler() {
        wifi_state = wifi_state_t::NOT_INITIALISED;
        disconnected_cb_requested = false;
        memset(mac_address_cstr, 0, MAC_ADDR_CS_LEN);

    }

    WifiHandler::~WifiHandler() {
        vSemaphoreDelete(wifi_init_mutex);
        vSemaphoreDelete(wifi_state_mutex);
    }

    void wifi_event_handler(WifiHandler *wifi_handler, esp_event_base_t event_base, int32_t event_id, void* event_data)
    {
        /* int32_t -> wifi_event_t enum */
        auto wifi_event_num = static_cast<wifi_event_t>(event_id);

        switch (wifi_event_num) {
            case WIFI_EVENT_STA_START: {
                wifi_handler->_set_wifi_state(WifiHandler::wifi_state_t::WAITING_FOR_IP);
                ESP_LOGI(WIFI_LOG_TAG, "Calling esp_wifi_connect...");
                esp_wifi_connect();
                break;
            }
            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_handler->_set_wifi_state(WifiHandler::wifi_state_t::DISCONNECTED);
                /* Call the callback only once, eg: close the mqtt client, socket only once */
                if(wifi_handler->disconnected_cb_requested ) {
                    wifi_handler->f_disconnected();
                    wifi_handler->disconnected_cb_requested = false;
                }
                ESP_LOGI(WIFI_LOG_TAG, "device disconnected, reconnecting to AP...");
                vTaskDelay(pdMS_TO_TICKS(4500));
                esp_wifi_connect();
                break;
            }
            default: {
                break;
            }
        }
    }

    void ip_event_handler(WifiHandler *wifi_handler, esp_event_base_t event_base, int32_t event_id, void* event_data)
    {
        /* int32_t -> ip_event_t enum */
        auto ip_event_num = static_cast<ip_event_t>(event_id);

        switch (ip_event_num) {
            case IP_EVENT_STA_GOT_IP: {
                wifi_handler->_set_wifi_state(WifiHandler::wifi_state_t::CONNECTED);
                wifi_handler->disconnected_cb_requested = true;
                auto *event = static_cast<ip_event_got_ip_t*>(event_data);
                ESP_LOGI(WIFI_LOG_TAG, "Connected to AP. IP-address: " IPSTR, IP2STR(&event->ip_info.ip));
                /* Reset the flag */
                wifi_handler->f_connected();
                break;
            }
            case IP_EVENT_STA_LOST_IP: {
                wifi_handler->_set_wifi_state(WifiHandler::wifi_state_t::WAITING_FOR_IP);
                ESP_LOGI(WIFI_LOG_TAG, "Device ip-address became invalid.");
                break;
            }
            default: {
                break;
            }

        }
    }

    void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
    {
        auto *wifi_handler = reinterpret_cast<WifiHandler*>(arg);
        if(event_base == WIFI_EVENT) {
            return wifi_event_handler(wifi_handler, event_base, event_id, event_data);
        }
        else if(event_base == IP_EVENT) {
            ip_event_handler(wifi_handler, event_base, event_id, event_data);
        }
        else {
            ESP_LOGW(WIFI_LOG_TAG, "unknown event arisen");
        }
    }

    esp_err_t WifiHandler::_init_wifi_mutexes()
    {
        wifi_init_mutex = xSemaphoreCreateMutex();
        wifi_state_mutex = xSemaphoreCreateMutex();
        if(wifi_init_mutex == nullptr || wifi_state_mutex == nullptr){
            return ESP_ERR_NO_MEM;
        }
        return ESP_OK;
    }


    esp_err_t WifiHandler::_init_nvs_partition()
    {
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

    esp_err_t WifiHandler::_init(const std::function<void(void)>& on_connected_cb, const std::function<void(void)>& on_disconnected_cb) {

        esp_err_t ret = ESP_OK;

        ret = _init_wifi_mutexes();
        if(ret != ESP_OK) {
            ESP_LOGE(WIFI_LOG_TAG, "%s", esp_err_to_name(ret));
            return ret;
        }
        std::cout << "Wifi mutexes created." << std::endl;

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

        /* Setting the provided callbacks */
        f_connected = on_connected_cb;
        f_disconnected  = on_disconnected_cb;

        /* 1. init phase */
        if(xSemaphoreTake(wifi_init_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGE(WIFI_LOG_TAG, "unable to obtain 'wifi_init_mutex'");
            return ESP_FAIL;
        }
        ret = esp_netif_init();
        if(ret != ESP_OK){
            ESP_LOGE(WIFI_LOG_TAG, "%s", esp_err_to_name(ret));
            xSemaphoreGive(wifi_init_mutex);
            return ret;
        }
        xSemaphoreGive(wifi_init_mutex);

        if(xSemaphoreTake(wifi_init_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGE(WIFI_LOG_TAG, "unable to obtain 'wifi_init_mutex'");
            return ESP_FAIL;
        }

        ret = esp_event_loop_create_default();
        if(ret != ESP_OK){
            ESP_LOGE(WIFI_LOG_TAG, "%s", esp_err_to_name(ret));
            xSemaphoreGive(wifi_init_mutex);
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
            xSemaphoreGive(wifi_init_mutex);
            return ret;
        }
        xSemaphoreGive(wifi_init_mutex);

        if(xSemaphoreTake(wifi_init_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGE(WIFI_LOG_TAG, "unable to obtain 'wifi_init_mutex'");
            return ESP_FAIL;
        }

        ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, this, nullptr);
        if(ret != ESP_OK) {
            ESP_LOGE(WIFI_LOG_TAG, "%s", esp_err_to_name(ret));
            xSemaphoreGive(wifi_init_mutex);
            return ret;
        }

        ret = esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, this, nullptr);
        if(ret != ESP_OK) {
            ESP_LOGE(WIFI_LOG_TAG, "%s", esp_err_to_name(ret));
            xSemaphoreGive(wifi_init_mutex);
            return ret;
        }
        xSemaphoreGive(wifi_init_mutex);


        if(xSemaphoreTake(wifi_init_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGE(WIFI_LOG_TAG, "unable to obtain 'wifi_init_mutex'");
            return ESP_FAIL;
        }

        ret = _set_wifi_credentials();
        if(ret != ESP_OK){
            xSemaphoreGive(wifi_init_mutex);
            return ESP_FAIL;
        }

        wifi_config_t wifi_config{};
        const char *ssid_cstr = wifi::WifiHandler::ssid.c_str();
        const char *password_cstr = wifi::WifiHandler::password.c_str();

        memcpy(wifi_config.sta.ssid, ssid_cstr, strlen(ssid_cstr));
        memcpy(wifi_config.sta.password, password_cstr, strlen(password_cstr));

        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        wifi_config.sta.pmf_cfg.capable = true;
        wifi_config.sta.pmf_cfg.required = false;

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        xSemaphoreGive(wifi_init_mutex);

        if(xSemaphoreTake(wifi_init_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGE(WIFI_LOG_TAG, "unable to obtain 'wifi_init_mutex'");
            return ESP_FAIL;
        }

        _set_wifi_state(wifi_state_t::INITIALISED);

        xSemaphoreGive(wifi_init_mutex);

        std::cout << " WifiHandler successfully initialized." << std::endl;

        return ret;
    }

    esp_err_t WifiHandler::wifi_handler_init(const on_wifi_connected_callback& on_connected_cb, const on_wifi_disconnected_callback& on_disconnected_cb) {
        return _init(on_connected_cb, on_disconnected_cb);
    }

    esp_err_t WifiHandler::wifi_handler_start() {
        if(wifi_state != wifi_state_t::INITIALISED){
            return ESP_FAIL;
        }
        return _start();
    }

    esp_err_t WifiHandler::_start() {
        esp_err_t ret = esp_wifi_start();
        return ret;
    }

    esp_err_t WifiHandler::_set_wifi_credentials() {

        std::string config_ssid = CONFIG_ESP_WIFI_SSID;
        size_t ssid_size = config_ssid.length();
        if(ssid_size == 0 || ssid_size > 32){
            ESP_LOGE(WIFI_LOG_TAG, "invalid ssid");
            return ESP_FAIL;
        }

        std::string config_password = CONFIG_ESP_WIFI_PASSWORD;
        size_t password_size = config_password.length();
        if(password_size > 64){
            ESP_LOGE(WIFI_LOG_TAG, "invalid password");
            return ESP_FAIL;
        }

        ssid = config_ssid;
        password = config_password;

        return ESP_OK;
    }

    esp_err_t WifiHandler::_set_wifi_state(wifi_state_t new_state) {
        if(xSemaphoreTake(wifi_state_mutex, pdMS_TO_TICKS(5)) != pdTRUE) {
            ESP_LOGE(WIFI_LOG_TAG, "unable to obtain 'wifi_state_mutex'");
            return ESP_FAIL;
        }
        this->wifi_state = new_state;
        xSemaphoreGive(wifi_state_mutex);
        return ESP_OK;
    }

    WifiHandler::wifi_state_t WifiHandler::_get_wifi_state() const {
        wifi_state_t result = this->wifi_state;
        return result;
    }

} // namespace wifi
