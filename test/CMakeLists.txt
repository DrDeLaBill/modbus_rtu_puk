cmake_minimum_required(VERSION 3.27)

project(modbusrtupuk-test VERSION 0.0.1)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)


message(STATUS "modbus_rtu_puk tests enabled.")

file(GLOB ${PROJECT_NAME}_HEADERS "./*.h")
file(GLOB ${PROJECT_NAME}_SOURCES "./*.c")
set(ALL_SRCS "${${PROJECT_NAME}_SOURCES};${${PROJECT_NAME}_HEADERS}")
add_executable(modbusrtupuktest ${ALL_SRCS})
target_include_directories(
    modbusrtupuktest
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../modbus_rtu_puk/inc
)
target_link_libraries(
    modbusrtupuktest
    modbusrtupuklib
)