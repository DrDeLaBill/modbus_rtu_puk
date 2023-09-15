# Copyright © 2023 Georgy E. All rights reserved.

if (WIN32)
    message("- Set MinGW Makefiles")
    set (CMAKE_GENERATOR "MinGW Makefiles" CACHE INTERNAL "" FORCE)
elseif (UNIX)
    message("- Set Unix Makefiles")
    set (CMAKE_GENERATOR "Unix Makefiles" CACHE INTERNAL "" FORCE)
else ()
    message(FATAL_ERROR "Unacceptable OS")
endif ()