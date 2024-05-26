set(SOURCES Main.cpp)

set (REQUIRED_COMPONENTS wifi_handler mqtt_handler)

idf_component_register(SRCS ${SOURCES} INCLUDE_DIRS "include" REQUIRES ${REQUIRED_COMPONENTS})
