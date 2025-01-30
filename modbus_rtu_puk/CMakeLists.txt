cmake_minimum_required(VERSION 3.26)

project(modbusrtupuklib VERSION 0.0.1 LANGUAGES C)


message(STATUS "modbus_rtu_puk library found.")


file(GLOB ${PROJECT_NAME}_HEADERS "./inc/*.h")
file(GLOB ${PROJECT_NAME}_SOURCES "./src/*.c")
set(ALL_SRCS "${${PROJECT_NAME}_SOURCES};${${PROJECT_NAME}_HEADERS}")


add_library(
    ${PROJECT_NAME}
    STATIC 
    ${ALL_SRCS}
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    ${PROJECT_NAME}
)
set_target_properties(
    modbusrtupuklib PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)