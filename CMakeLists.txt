cmake_minimum_required(VERSION 3.26)

project(modbusrtupuk VERSION 0.0.2 LANGUAGES C)

option(MODE_SDCC "SDCC mode." OFF)

if(MODE_SDCC)
    message(STATUS "project: SDCC MODE - ENABLED")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --model-large --stack-auto")
endif()

add_subdirectory(modbus_rtu_puk)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(MODBUS_TEST "Enable modbus_rtu_puk tests." ON)

    if (MODBUS_TEST)
        add_subdirectory(test)
    endif()
endif()