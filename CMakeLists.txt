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
    set(TESTS_NAME ini_good_tests)

    enable_testing()
    add_executable(${TESTS_NAME}
        tests/helper.h
        tests/ini_good_tests.c
    )
    target_link_libraries(${TESTS_NAME} PRIVATE ${PROJECT_NAME})

    add_test(
        NAME ${TESTS_NAME}
        COMMAND ${TESTS_NAME}
    )
endif()
