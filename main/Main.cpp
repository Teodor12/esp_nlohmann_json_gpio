#include <Main.hpp>

#define MAIN_LOG_TAG "MAIN"

static Main main_entry;

esp_err_t Main::setup()
{
    ESP_ERROR_CHECK(gpio_init());
    ESP_ERROR_CHECK(mqtt_handler.mqtt_handler_init());
    ESP_ERROR_CHECK(wifi_handler.wifi_handler_init(wifi_connected_cb, wifi_disconnected_cb));
    ESP_ERROR_CHECK(wifi_handler.wifi_handler_start());
    return ESP_OK;
}

[[noreturn]] void Main::loop(){
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void wifi_connected_cb() {
    main_entry.mqtt_handler.mqtt_handler_start();
}

void wifi_disconnected_cb() {
    main_entry.mqtt_handler.mqtt_handler_suspend();
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(main_entry.setup());
    vTaskDelete(nullptr);
}
