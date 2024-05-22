#pragma once

#include "json.hpp"
#include "esp_system.h"


struct GpioRequest {

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GpioRequest, gpio_num, state);

    unsigned gpio_num;
    bool state;
};

esp_err_t gpio_init(uint8_t gpio_pin_1, uint8_t gpio_pin_2, uint8_t gpio_pin_3);

esp_err_t update_gpio_level(uint8_t gpio_pin_number, bool gpio_state);

esp_err_t update_gpio_level(const GpioRequest& gpio_state);