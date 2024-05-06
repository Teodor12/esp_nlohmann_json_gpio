#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "json_handler.hpp"
#include "WifiHandler.hpp"

using namespace wifi;

class Main final {

public:

    static constexpr char *MAIN_LOG_TAG{"MAIN"};
    esp_err_t setup();
    void loop();
};