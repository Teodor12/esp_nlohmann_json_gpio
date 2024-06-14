#pragma once

#include "json.hpp"
#include "esp_system.h"

struct GpioRequest {

public:
    int gpio_num;
    bool state;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GpioRequest, gpio_num, state)
};

GpioRequest create_gpio_request(const std::string& json_request_str);

esp_err_t gpio_init();

esp_err_t update_gpio_levels(const GpioRequest& gpio_state);


