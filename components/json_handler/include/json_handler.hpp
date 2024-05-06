#pragma once

#include <string>
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

json json_handler_create_json(uint8_t gpio_num, bool gpio_state);

json json_handler_parse_str(const std::string& json_string);

std::string json_handler_serialize_json(const json& json_instance);
