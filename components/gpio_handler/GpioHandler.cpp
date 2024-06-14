#include "include/GpioHandler.hpp"
#include "esp_log.h"
#include "driver/gpio.h"
#include "json_validator.hpp"

#define GPIO_LOG_TAG "GPIO_HANDLER"

static constexpr gpio_num_t PIN_1_NUM  = static_cast<gpio_num_t>(CONFIG_GPIO_PIN1_NUMBER);
static constexpr gpio_num_t PIN_2_NUM  = static_cast<gpio_num_t>(CONFIG_GPIO_PIN2_NUMBER);
static constexpr gpio_num_t PIN_3_NUM  = static_cast<gpio_num_t>(CONFIG_GPIO_PIN3_NUMBER);

static esp_err_t init_gpio_pins()
{
    esp_err_t ret = ESP_OK;
    ret |= gpio_set_direction(PIN_1_NUM, GPIO_MODE_OUTPUT);
    ret |= gpio_set_direction(PIN_2_NUM, GPIO_MODE_OUTPUT);
    ret |= gpio_set_direction(PIN_3_NUM, GPIO_MODE_OUTPUT);
    return ret;
}

static esp_err_t update_gpio(gpio_num_t gpio_pin_number, uint32_t gpio_level)
{
    esp_err_t ret = ESP_OK;
    switch (gpio_pin_number) {
        case PIN_1_NUM: {
            ret = gpio_set_level(PIN_1_NUM, gpio_level);
            break;
        }
        case PIN_2_NUM: {
            ret = gpio_set_level(PIN_2_NUM, gpio_level);
            break;
        }
        case PIN_3_NUM: {
            ret = gpio_set_level(PIN_3_NUM, gpio_level);
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

#define GPIO_REQUEST_INVALID_MASK (int)(-1)
static GpioRequest create_request(const std::string& json_request_str)
{
    GpioRequest gpio_request = {GPIO_REQUEST_INVALID_MASK, false};
    json temp_json = json({});
    if(parse_json_str(json_request_str, temp_json) != ESP_OK) {
        return gpio_request;
    }

    try {
        gpio_request = temp_json.get<GpioRequest>();
        ESP_LOGI(GPIO_LOG_TAG, "GPIO Number: %d, GPIO state: %d", gpio_request.gpio_num, gpio_request.state);
    }
    catch (const json::type_error &type_error) {
        ESP_LOGE(GPIO_LOG_TAG, "%s =====> %s", temp_json.dump().c_str(), type_error.what());
    }
    catch (const json::other_error &other_error) {
        ESP_LOGE(GPIO_LOG_TAG, "%s =====> %s", temp_json.dump().c_str(), other_error.what());
    }

    return gpio_request;
}

static bool is_valid_request(const GpioRequest& gpio_request)
{
    return (gpio_request.gpio_num != GPIO_REQUEST_INVALID_MASK);
}

GpioRequest create_gpio_request(const std::string& json_request_str)
{
    return create_request(json_request_str);
}

esp_err_t gpio_init()
{
    esp_err_t ret = init_gpio_pins();
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

