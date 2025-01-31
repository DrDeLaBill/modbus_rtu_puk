cmake_minimum_required(VERSION 3.26)

project(modbus_rtu_puk_lib VERSION 0.0.1 LANGUAGES C)


message(STATUS "modbus_rtu_puk library found.")


file(GLOB ${PROJECT_NAME}_HEADERS "./inc/*.h")
file(GLOB ${PROJECT_NAME}_SOURCES "./src/*.c")
set(ALL_SRCS "${${PROJECT_NAME}_SOURCES};${${PROJECT_NAME}_HEADERS}")
add_library(
    ${PROJECT_NAME}
    STATIC 
    ${ALL_SRCS}
)

SET(DIR_LIST "")
MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${curdir}/${child})
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()
SUBDIRLIST(DIR_LIST "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    "${DIR_LIST}"
)

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)