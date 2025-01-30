cmake_minimum_required(VERSION 3.26)

project(modbusrtupuk-test VERSION 0.0.1 LANGUAGES C)


message(STATUS "modbus_rtu_puk tests enabled.")


file(GLOB ${PROJECT_NAME}_HEADERS "./*.h")
file(GLOB ${PROJECT_NAME}_SOURCES "./*.c")
set(ALL_SRCS "${${PROJECT_NAME}_SOURCES};${${PROJECT_NAME}_HEADERS}")
if(MODE_SDCC)
    file(GLOB ${PROJECT_NAME}_LIB_SOURCES "../modbus_rtu_puk/src/*.c")
    set(ALL_SRCS "${ALL_SRCS};${${PROJECT_NAME}_LIB_SOURCES}")
endif()


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
set_target_properties(
    modbusrtupuktest PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)