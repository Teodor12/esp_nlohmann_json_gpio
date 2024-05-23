#include "json_validator.hpp"
#include "esp_log.h"

#define JSON_VALIDATOR_LOG_TAG "json_validator"

static esp_err_t validate_json_str(const std::string& json_str)
{
    if(!json::accept(json_str)) {
        ESP_LOGE(JSON_VALIDATOR_LOG_TAG, "invalid json format");
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

esp_err_t parse_json_str(const std::string& json_str, json& json_arg)
{
    esp_err_t ret = validate_json_str(json_str);
    if (ret != ESP_OK) {
        return ret;
    }

    try {
        json_arg = json::parse(json_str);
        return ESP_OK;
    }
    catch (const json::parse_error& parse_error) {
        ESP_LOGW(JSON_VALIDATOR_LOG_TAG, "unable to parse json_str");
        json_arg = json({});
        return ESP_FAIL;
    }
}
