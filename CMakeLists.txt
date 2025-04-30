cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME iniparser)
project(${PROJECT_NAME}
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(.)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
endif()

add_executable(${PROJECT_NAME}
    main.cpp
    iniparser.c
    strutils.cpp
)
