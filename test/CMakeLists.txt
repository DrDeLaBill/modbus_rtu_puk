cmake_minimum_required(VERSION 3.26)

project(modbus_rtu_puk_test VERSION 0.0.1 LANGUAGES C)

message(STATUS "modbus_rtu_puk tests enabled")

file(GLOB ${PROJECT_NAME}_HEADERS "./*.h")
file(GLOB ${PROJECT_NAME}_SOURCES "./*.c")
if(MODE_SDCC)
    file(GLOB ${PROJECT_NAME}_HEADERS "${CMAKE_SOURCE_DIR}/*.h")
    file(GLOB ${PROJECT_NAME}_SOURCES "${CMAKE_SOURCE_DIR}/*.c")
endif()
set(ALL_SRCS "${${PROJECT_NAME}_SOURCES};${${PROJECT_NAME}_HEADERS}")

foreach(src_path ${ALL_SRCS})
    message(STATUS "${src_path}")
endforeach()

# Create project
add_executable(${PROJECT_NAME} ${ALL_SRCS})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    "."
)

# Link library
target_link_libraries(
    ${PROJECT_NAME}
    modbus_rtu_puk_lib
)

# Set project properties
set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)