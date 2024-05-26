#pragma once

#include <string>
#include "esp_system.h"
#include "json.hpp"

using json = nlohmann::json;

esp_err_t parse_json_str(const std::string& json_str, json& json_arg);
