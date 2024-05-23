#include "include/GpioHandler.hpp"
#include "esp_log.h"
#include "driver/gpio.h"
#include "json_validator.hpp"

#define GPIO_LOG_TAG "GPIO_HANDLER"

static esp_err_t init_gpio_pins(uint8_t gpio_pin_1, uint8_t gpio_pin_2, uint8_t gpio_pin_3)
{
    esp_err_t ret = ESP_OK;
    ret |= gpio_set_direction(static_cast<gpio_num_t>(gpio_pin_1), GPIO_MODE_OUTPUT);
    ret |= gpio_set_direction(static_cast<gpio_num_t>(gpio_pin_2), GPIO_MODE_OUTPUT);
    ret |= gpio_set_direction(static_cast<gpio_num_t>(gpio_pin_3), GPIO_MODE_OUTPUT);
    return ret;
}

static esp_err_t update_gpio(gpio_num_t gpio_pin_number, uint32_t gpio_level)
{
    esp_err_t ret = ESP_OK;

    switch (gpio_pin_number) {
        case GPIO_NUM_25: {
            ret = gpio_set_level(GPIO_NUM_25, gpio_level);
            break;
        }
        case GPIO_NUM_26: {
            ret = gpio_set_level(GPIO_NUM_26, gpio_level);
            break;
        }
        case GPIO_NUM_27: {
            ret = gpio_set_level(GPIO_NUM_27, gpio_level);
            break;
        }
        default: {
            ret = ESP_ERR_INVALID_ARG;
            ESP_LOGW(GPIO_LOG_TAG, "unconfigured gpio pin requested (num: %d)", gpio_pin_number);
            break;
        }
    }
    return ret;
}

static GpioRequest create_request(const std::string& json_request_str)
{
    GpioRequest gpio_request = {};
    json temp_json;
    if(parse_json_str(json_request_str, temp_json) != ESP_OK) {
        gpio_request.gpio_num = -1;
        gpio_request.state = -1;
        return gpio_request;
    }
    gpio_request = temp_json;
    return gpio_request;
}

static bool is_valid_request(const GpioRequest& gpio_request)
{
    return ((gpio_request.state == 0 || gpio_request.state == 1) && gpio_request.gpio_num > 0);
}

GpioRequest create_gpio_request(const std::string& json_request_str)
{
    ESP_LOGI(GPIO_LOG_TAG, "%s", __func__);
    return create_request(json_request_str);
}

esp_err_t gpio_init(uint8_t gpio_pin_1, uint8_t gpio_pin_2, uint8_t gpio_pin_3)
{
    esp_err_t ret = init_gpio_pins(gpio_pin_1, gpio_pin_2, gpio_pin_3);
    if(ret != ESP_OK) {
        ESP_LOGE(GPIO_LOG_TAG, "unable to initialize gpio pins");
    }
    ESP_LOGI(GPIO_LOG_TAG, "gpio pins configured successfully");
    return ret;
}

esp_err_t update_gpio_levels(const GpioRequest& gpio_request)
{
    if(!is_valid_request(gpio_request)) {
        return ESP_ERR_INVALID_ARG;
    }
    gpio_num_t pin_number = static_cast<gpio_num_t>(gpio_request.gpio_num);
    uint32_t pin_level = static_cast<bool>(gpio_request.state);
    return update_gpio(pin_number, pin_level);
}

esp_err_t update_gpio_levels(uint8_t gpio_pin_number, bool gpio_state)
{
    gpio_num_t pin_number = static_cast<gpio_num_t>(gpio_pin_number);
    uint32_t pin_level = static_cast<bool>(gpio_state);
    return update_gpio(pin_number, pin_level);
}
