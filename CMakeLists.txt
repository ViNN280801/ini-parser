cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME iniparser)
project(${PROJECT_NAME}
    VERSION 1.0.0
    LANGUAGES C
    DESCRIPTION "Lightweight INI parser library for C"
)

# ================ Main settings ================
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(INIPARSER_TESTS "Build tests" ON)

# ================ Library ====================
add_library(${PROJECT_NAME}
    iniparser.h
    iniparser.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# ================ Testing ======================
if(INIPARSER_TESTS)
    set(INI_GOOD_TESTS ini_good_tests)
    set(INI_LOAD_TESTS ini_load_tests)

    enable_testing()

    add_executable(${INI_GOOD_TESTS} tests/ini_good_tests.c)
    target_link_libraries(${INI_GOOD_TESTS} PRIVATE ${PROJECT_NAME})
    add_executable(${INI_LOAD_TESTS} tests/ini_load_tests.c)
    target_link_libraries(${INI_LOAD_TESTS} PRIVATE ${PROJECT_NAME})

    add_test(
        NAME ${INI_GOOD_TESTS}
        COMMAND ${INI_GOOD_TESTS}
    )
    add_test(
        NAME ${INI_LOAD_TESTS}
        COMMAND ${INI_LOAD_TESTS}
    )
endif()
