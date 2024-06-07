# ESP32 LED Control with JSON over MQTT

> This is a basic test project for an ESP32 devkitc-wroom board to switch 3 of it's pins based on a JSON object.

1. configure your Wi-Fi's SSID and password in the "Wifi configuration" menu  Any errors will be asserted with ESP_ERROR_CHECK()!
2. To set the 3 gpio pins, use the "GPIO configuration" menu(using idf.py menuconfig). I recommend connecting three LEDS on the according pins.
3. Open your terminal and type: mosquitto_pub -h mqtt.eclipseprojects.io -t esp_gpio -m "{\"gpio_num\":25,\"state\":true}" / mosquitto_pub -h mqtt.eclipseprojects.io -t esp_gpio -m '{"gpio_num":26,"state":true}'. The project is using the
most simple mqtt setup. The fields 'gpio_num' and 'state' can be modified freely. Invalid JSON request will be discarded.
