#include "json_handler.hpp"

json json_handler_create_json(uint8_t gpio_num, bool gpio_state){
    auto ret = json::object();
    ret["gpio_num"] = gpio_num;
    ret["gpio_state"] = gpio_state;
    return ret;
}

json json_handler_parse_str(const std::string &json_string){
    json json_instance;
    json_instance = json::parse(json_string);
    return json_instance;
}

std::string json_handler_serialize_json(const json &json_instance){
    auto ret = json_instance.dump();
    return ret;
}