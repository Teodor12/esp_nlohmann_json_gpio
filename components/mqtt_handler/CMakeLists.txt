set(SOURCES MQTTHandler.cpp)

set(REQUIRED_COMPONENTS esp_event freertos log mqtt)

idf_component_register(SRCS ${SOURCES} INCLUDE_DIRS "include" REQUIRES ${REQUIRED_COMPONENTS})