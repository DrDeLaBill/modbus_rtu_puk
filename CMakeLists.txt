# Copyright © 2023 Georgy E. All rights reserved.

cmake_minimum_required(VERSION 3.27)

project("cmake_test" VERSION 1.0 LANGUAGES C)

if (MODE_SDCC)
    message("- SDCC mode")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --model-large --stack-auto")
else()
    message("- GCC mode")
    
    set(CMAKE_C_COMPILER gcc)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
endif()

file(GLOB CSRC "./src/*.c")

add_executable(${PROJECT_NAME} ${CSRC})

