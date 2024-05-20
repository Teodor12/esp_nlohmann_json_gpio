#pragma once

#include "mqtt_client.h"
#include "esp_log.h"

namespace mqtt{

    class MQTTHandler final {

    public:

        explicit MQTTHandler();

        ~MQTTHandler();

        esp_err_t mqtt_handler_init();

        esp_err_t mqtt_handler_start();


    private:
        esp_mqtt_client_handle_t mqtt_client = nullptr;
        SemaphoreHandle_t mqtt_init_mutex;
        esp_err_t _init();
        esp_err_t _start();

        enum class mqtt_state_t {
            MQTT_NOT_INITIALISED,
            MQTT_INITIALISED,
            MQTT_CONNECTED,
            MQTT_DISCONNECTED,
            MQTT_SUBSCRIBED,
            MQTT_UNSUBSCRIBED
        } mqtt_state;

        friend void mqtt_event_handler(void *arg, esp_event_base_t base, int32_t event_id, void *event_data);

    };
}




