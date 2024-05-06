#include <Main.hpp>


static Main main_entry;

esp_err_t Main::setup()
{
    esp_err_t ret{ESP_OK};

    WifiHandler wifi_handler;
    ret = wifi_handler.wifi_handler_init();
    ret= wifi_handler.wifi_handler_start();
    return ret;
}

void Main::loop(void){
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(main_entry.setup());
}
