#pragma once

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "WifiHandler.hpp"
#include "MQTTHandler.hpp"

using namespace wifi;
using namespace mqtt;

class Main final {

public:

    esp_err_t setup();
    [[noreturn]] void loop();

    WifiHandler wifi_handler;
    MQTTHandler mqtt_handler;

};

void wifi_connected_cb();

void wifi_disconnected_cb();
