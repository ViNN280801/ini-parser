cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME iniparser)
project(${PROJECT_NAME}
    VERSION 1.0.0
    LANGUAGES C
    DESCRIPTION "Lightweight INI parser library for C"
)

# ================ ASan settings ================
if(MSVC)
    # Enable ASan only in Debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/fsanitize=address /Zi /Od /RTC1)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG /fsanitize=address")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /fsanitize=address")
        set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /fsanitize=address")
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
    endif()
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

# ================ Main settings ================
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(INIPARSER_TESTS "Build tests" ON)

# ================ Library ====================
set(INI_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(INI_SOURCE_FILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INI_SOURCE_FILES
    ${INI_SOURCE_FILE_DIR}/ini_hash_table.c
    ${INI_SOURCE_FILE_DIR}/ini_mutex.c
    ${INI_SOURCE_FILE_DIR}/ini_string.c
)

include_directories(${INI_INCLUDE_DIRS})
add_library(${PROJECT_NAME}
    ${INI_SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# ================ Testing ======================
if(INIPARSER_TESTS)
    set(INI_CREATE_CONTEXT_TESTS ini_create_context_tests)
    set(INI_FREE_TESTS ini_free_tests)
    set(INI_FUNCTIONAL_TESTS ini_functional_tests)
    set(INI_GET_VALUE_TESTS ini_get_value_tests)
    set(INI_GOOD_TESTS ini_good_tests)
    set(INI_INTEGRATION_TESTS ini_integration_tests)
    set(INI_LOAD_TESTS ini_load_tests)
    set(INI_PRINT_TESTS ini_print_tests)
    set(INI_SAVE_SECTION_VALUE_TESTS ini_save_section_value_tests)
    set(INI_SAVE_TESTS ini_save_tests)
    set(INI_STRESS_TESTS ini_stress_tests)

    enable_testing()

    add_executable(${INI_CREATE_CONTEXT_TESTS} tests/ini_create_context_tests.c)
    target_link_libraries(${INI_CREATE_CONTEXT_TESTS} PRIVATE ${PROJECT_NAME})

    add_executable(${INI_FREE_TESTS} tests/ini_free_tests.c)
    target_link_libraries(${INI_FREE_TESTS} PRIVATE ${PROJECT_NAME})

    add_executable(${INI_FUNCTIONAL_TESTS} tests/ini_functional_tests.c)
    target_link_libraries(${INI_FUNCTIONAL_TESTS} PRIVATE ${PROJECT_NAME})

    add_executable(${INI_GET_VALUE_TESTS} tests/ini_get_value_tests.c)
    target_link_libraries(${INI_GET_VALUE_TESTS} PRIVATE ${PROJECT_NAME})

    add_executable(${INI_GOOD_TESTS} tests/ini_good_tests.c)
    target_link_libraries(${INI_GOOD_TESTS} PRIVATE ${PROJECT_NAME})

    add_executable(${INI_INTEGRATION_TESTS} tests/ini_integration_tests.c)
    target_link_libraries(${INI_INTEGRATION_TESTS} PRIVATE ${PROJECT_NAME})

    add_executable(${INI_LOAD_TESTS} tests/ini_load_tests.c)
    target_link_libraries(${INI_LOAD_TESTS} PRIVATE ${PROJECT_NAME})

    add_executable(${INI_PRINT_TESTS} tests/ini_print_tests.c)
    target_link_libraries(${INI_PRINT_TESTS} PRIVATE ${PROJECT_NAME})

    add_executable(${INI_SAVE_SECTION_VALUE_TESTS} tests/ini_save_section_value_tests.c)
    target_link_libraries(${INI_SAVE_SECTION_VALUE_TESTS} PRIVATE ${PROJECT_NAME})

    add_executable(${INI_SAVE_TESTS} tests/ini_save_tests.c)
    target_link_libraries(${INI_SAVE_TESTS} PRIVATE ${PROJECT_NAME})

    add_executable(${INI_STRESS_TESTS} tests/ini_stress_tests.c)
    target_link_libraries(${INI_STRESS_TESTS} PRIVATE ${PROJECT_NAME})

    # add_test(NAME ${INI_CREATE_CONTEXT_TESTS} COMMAND ${INI_CREATE_CONTEXT_TESTS})
    # add_test(NAME ${INI_FREE_TESTS} COMMAND ${INI_FREE_TESTS})
    # add_test(NAME ${INI_FUNCTIONAL_TESTS} COMMAND ${INI_FUNCTIONAL_TESTS})
    # add_test(NAME ${INI_GET_VALUE_TESTS} COMMAND ${INI_GET_VALUE_TESTS})
    # add_test(NAME ${INI_GOOD_TESTS} COMMAND ${INI_GOOD_TESTS})
    # add_test(NAME ${INI_INTEGRATION_TESTS} COMMAND ${INI_INTEGRATION_TESTS})
    # add_test(NAME ${INI_LOAD_TESTS} COMMAND ${INI_LOAD_TESTS})
    # add_test(NAME ${INI_PRINT_TESTS} COMMAND ${INI_PRINT_TESTS})
    # add_test(NAME ${INI_SAVE_SECTION_VALUE_TESTS} COMMAND ${INI_SAVE_SECTION_VALUE_TESTS})
    # add_test(NAME ${INI_SAVE_TESTS} COMMAND ${INI_SAVE_TESTS})
    # add_test(NAME ${INI_STRESS_TESTS} COMMAND ${INI_STRESS_TESTS})
endif()
