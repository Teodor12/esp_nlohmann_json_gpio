set(SOURCES GpioHandler.cpp)

set(REQUIRED_COMPONENTS log driver)

idf_component_register(SRCS ${SOURCES} INCLUDE_DIRS "include" REQUIRES ${REQUIRED_COMPONENTS})