cmake_minimum_required(VERSION 3.27)

project(modbusrtutest VERSION 0.0.2 LANGUAGES C)

add_subdirectory(modbus_rtu_puk)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(MODBUS_TEST "Enable modbus_rtu_puk tests." ON)

    if (MODBUS_TEST)
        enable_testing()
        add_subdirectory(test)
    endif()
endif()