cmake_minimum_required(VERSION 3.26)

project(modbus_rtu_puk VERSION 0.0.2 LANGUAGES C)

option(MODE_SDCC "SDCC mode." OFF)

if(MODE_SDCC)
    message(STATUS "project: SDCC MODE - ENABLED")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --model-large --stack-auto")
else()
    message(STATUS "project: SDCC MODE - DISABLED")
endif()

file(GLOB ${PROJECT_NAME}_HEADERS "modbus_rtu_puk/inc/*.h")
file(GLOB ${PROJECT_NAME}_SOURCES "modbus_rtu_puk/src/*.c")
set(ALL_SRCS "${${PROJECT_NAME}_SOURCES};${${PROJECT_NAME}_HEADERS}")
add_library(
    ${PROJECT_NAME}
    STATIC 
    ${ALL_SRCS}
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    "modbus_rtu_puk/inc"
)

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    message(STATUS "modbus_rtu_puk generated as current project")    
    option(MODBUS_TEST "enable modbus_rtu_puk tests" ON)

    message(STATUS "enable modbus_rtu_puk testing")
    
    add_subdirectory(test)
else()
    message(STATUS "modbus_rtu_puk added as a library")

    target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        ${PROJECT_NAME}
    )
endif()