#include <Main.hpp>


static Main main_entry;

esp_err_t Main::setup()
{
    WifiHandler wifi_handler;
    ESP_ERROR_CHECK(wifi_handler.wifi_handler_init());
    ESP_ERROR_CHECK(wifi_handler.wifi_handler_start());
    return ESP_OK;
}

void Main::loop(){
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(main_entry.setup());
}
