#pragma once

#include "mqtt_client.h"
#include "GpioHandler.hpp"

namespace mqtt{

    class MQTTHandler final {

    public:

        explicit MQTTHandler();

        ~MQTTHandler();

        esp_err_t mqtt_handler_init();

        esp_err_t mqtt_handler_start();

        esp_err_t mqtt_handler_suspend();

    private:
        esp_err_t _init();
        esp_err_t _start();
        esp_err_t _suspend();
        enum class mqtt_state_t;
        mqtt_state_t get_mqtt_state() const;
        void set_mqtt_state(mqtt_state_t new_mqtt_state);
        friend void mqtt_event_handler(void *arg, esp_event_base_t base, int32_t event_id, void *event_data);

        esp_mqtt_client_handle_t mqtt_client = nullptr;
        SemaphoreHandle_t mqtt_init_mutex;
        enum class mqtt_state_t {
            MQTT_NOT_INITIALISED,
            MQTT_INITIALISED,
            MQTT_CONNECTED,
            MQTT_DISCONNECTED,
            MQTT_SUBSCRIBED,
            MQTT_UNSUBSCRIBED
        } mqtt_state;


    };

} //namespace mqtt
