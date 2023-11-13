cmake_minimum_required(VERSION 3.27)

project(modbusrtupuk-lib VERSION 0.0.1 LANGUAGES C)


message(STATUS "modbus_rtu_puk library found.")


file(GLOB ${PROJECT_NAME}_HEADERS "./inc/*.h")
file(GLOB ${PROJECT_NAME}_SOURCES "./src/*.c")
set(ALL_SRCS "${${PROJECT_NAME}_SOURCES};${${PROJECT_NAME}_HEADERS}")


add_library(modbusrtupuklib STATIC ${ALL_SRCS})
target_include_directories(
    modbusrtupuklib
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
set_target_properties(
    modbusrtupuklib PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)