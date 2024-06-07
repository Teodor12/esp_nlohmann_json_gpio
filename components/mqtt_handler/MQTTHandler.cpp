#include <iostream>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "include/MQTTHandler.hpp"

namespace mqtt{

#define MQTT_LOG_TAG "MQTT_HANDLER"

    MQTTHandler::MQTTHandler() {
        mqtt_init_mutex = xSemaphoreCreateMutex();
        mqtt_state = mqtt_state_t::MQTT_NOT_INITIALISED;
    }

    MQTTHandler::~MQTTHandler() {
        vSemaphoreDelete(mqtt_init_mutex);
    }

    void mqtt_event_handler(void *arg, esp_event_base_t base, int32_t event_id, void *event_data)
    {
        MQTTHandler *mqtt_handler = reinterpret_cast<MQTTHandler *>(arg);

        auto mqtt_event_num = static_cast<esp_mqtt_event_id_t>(event_id);

        auto mqtt_event_data = reinterpret_cast<esp_mqtt_event_handle_t>(event_data);

        switch (mqtt_event_num) {
            case MQTT_EVENT_CONNECTED: {
                mqtt_handler->set_mqtt_state(MQTTHandler::mqtt_state_t::MQTT_CONNECTED);
                ESP_LOGI(MQTT_LOG_TAG, "device established connection with broker");
                esp_mqtt_client_subscribe(mqtt_handler->mqtt_client, CONFIG_MQTT_BROKER_TOPIC, 0);
                break;
            }
            case MQTT_EVENT_DISCONNECTED: {
                mqtt_handler->set_mqtt_state(MQTTHandler::mqtt_state_t::MQTT_DISCONNECTED);
                ESP_LOGI(MQTT_LOG_TAG, "device lost connection with broker");
                vTaskDelay(pdMS_TO_TICKS(5000));
                ESP_LOGI(MQTT_LOG_TAG, "reconnecting to broker...");
                esp_mqtt_client_reconnect(mqtt_handler->mqtt_client);
                break;
            }
            case MQTT_EVENT_SUBSCRIBED: {
                mqtt_handler->set_mqtt_state(MQTTHandler::mqtt_state_t::MQTT_SUBSCRIBED);
                ESP_LOGI(MQTT_LOG_TAG, "device successfully subscribed to topic");
                break;
            }
            case MQTT_EVENT_UNSUBSCRIBED: {
                mqtt_handler->set_mqtt_state(MQTTHandler::mqtt_state_t::MQTT_UNSUBSCRIBED);
                ESP_LOGI(MQTT_LOG_TAG, "device successfully unsubscribed from topic");
                break;
            }
            case MQTT_EVENT_PUBLISHED: {
                break;
            }
            case MQTT_EVENT_DATA: {
                ESP_LOGD(MQTT_LOG_TAG, "Topic:%.*s", mqtt_event_data->topic_len, mqtt_event_data->topic);
                ESP_LOGD(MQTT_LOG_TAG, "Data:%.*s", mqtt_event_data->data_len, mqtt_event_data->data);
                std::string gpio_request_json(mqtt_event_data->data, mqtt_event_data->data_len);
                GpioRequest gpio_request = create_gpio_request(gpio_request_json);
                update_gpio_levels(gpio_request);
                break;
            }
            default: {
                break;
            }
        }
    }

    esp_err_t MQTTHandler::mqtt_handler_init() {
        return _init();
    }

    esp_err_t MQTTHandler::mqtt_handler_start() {
        return _start();
    }

    esp_err_t MQTTHandler::mqtt_handler_suspend() {
        return _suspend();
    }

    esp_err_t MQTTHandler::_init() {

        esp_err_t ret = ESP_OK;

        if(xSemaphoreTake(mqtt_init_mutex, pdMS_TO_TICKS(10)) != pdTRUE){
            ESP_LOGE(MQTT_LOG_TAG, "unable to obtain 'mqtt_init_mutex'");
            return ESP_FAIL;
        }

        esp_mqtt_client_config_t mqtt_cfg = {};
        mqtt_cfg = {
                .broker{
                        .address{
                                .hostname = CONFIG_MQTT_BROKER_IP,
                                .transport = MQTT_TRANSPORT_OVER_TCP,
                                .port = CONFIG_MQTT_BROKER_PORT,
                        }
                }
        };
        mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

        ret = esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, this);
        if(ret != ESP_OK) {
            ESP_LOGE(MQTT_LOG_TAG, "%s", esp_err_to_name(ret));
            xSemaphoreGive(mqtt_init_mutex);
            return ret;
        }

        mqtt_state = mqtt_state_t::MQTT_INITIALISED;
        xSemaphoreGive(mqtt_init_mutex);

        std::cout << " MQTTHandler successfully initialized." << std::endl;
        return ret;
    }

    esp_err_t MQTTHandler::_start() {
        if (mqtt_state != mqtt_state_t::MQTT_INITIALISED) {
            return ESP_FAIL;
        }

        esp_err_t ret = esp_mqtt_client_start(mqtt_client);
        if (ret != ESP_OK) {
            ESP_LOGE(MQTT_LOG_TAG, "%s", esp_err_to_name(ret));
        }
        return ret;
    }

    esp_err_t MQTTHandler::_suspend() {
        return esp_mqtt_client_disconnect(mqtt_client);
    }

    MQTTHandler::mqtt_state_t MQTTHandler::get_mqtt_state() const {
        return mqtt_state;
    }

    void MQTTHandler::set_mqtt_state(MQTTHandler::mqtt_state_t mqttState) {
        mqtt_state = mqttState;
    }
} //namespace mqtt
