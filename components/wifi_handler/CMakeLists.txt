set(SOURCES WifiHandler.cpp)

set(REQUIRED_COMPONENTS nvs_flash esp_netif esp_wifi lwip esp_event freertos nvs_flash)

idf_component_register(SRCS ${SOURCES} INCLUDE_DIRS "include" REQUIRES ${REQUIRED_COMPONENTS})