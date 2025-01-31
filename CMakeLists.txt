cmake_minimum_required(VERSION 3.26)

project(modbus_rtu_puk VERSION 0.0.2 LANGUAGES C)

option(MODE_SDCC "SDCC mode." OFF)

if(MODE_SDCC)
    message(STATUS "project: SDCC MODE - ENABLED")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --model-large --stack-auto")
endif()

add_subdirectory(modbus_rtu_puk)

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    message(STATUS "modbus_rtu_puk generated as current project")    
    option(MODBUS_TEST "enable modbus_rtu_puk tests" ON)

    message(STATUS "enable modbus_rtu_puk testing")
    
    add_subdirectory(test)
else()
    message(STATUS "modbus_rtu_puk added as a library")
endif()