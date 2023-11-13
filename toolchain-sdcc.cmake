cmake_minimum_required(VERSION 3.27)

if("${MODE_SDCC}" STREQUAL "ON")	
    # the name of the target operating system
    SET(CMAKE_SYSTEM_NAME Generic)

    # which compiler to use for C (dummy for CXX)
    SET(CMAKE_C_COMPILER sdcc)
    SET(CMAKE_CXX_COMPILER true) 

    # here is the target environment located
    # SET(CMAKE_FIND_ROOT_PATH "C:/SDCC")

    SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

    # set proper linker flags
    SET(CMAKE_EXE_LINKER_FLAGS "--out-fmt-ihx" CACHE STRING "Linker flags" FORCE)
endif()