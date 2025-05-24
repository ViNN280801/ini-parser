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
        add_compile_options(/fsanitize=address /Zi /Od)
        add_link_options(/DEBUG /fsanitize=address)

        if(NOT BUILD_SHARED_LIBS)
            add_link_options(/wholearchive:clang_rt.asan_dynamic-x86_64.lib)
        endif()

        set(ENV{ASAN_VCASAN_DEBUGGING} 1)
    endif()
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
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
    ${INI_SOURCE_FILE_DIR}/ini_filesystem.c
    ${INI_SOURCE_FILE_DIR}/ini_hash_table.c
    ${INI_SOURCE_FILE_DIR}/ini_mutex.c
    ${INI_SOURCE_FILE_DIR}/ini_parser.c
    ${INI_SOURCE_FILE_DIR}/ini_status.c
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
    set(INI_FILESYSTEM_TESTS ini_filesystem_tests)
    set(INI_HASH_TABLE_TESTS ini_hash_table_tests)
    set(INI_MUTEX_TESTS ini_mutex_tests)
    set(INI_PARSER_TESTS ini_parser_tests)

    set(INI_FUNCTIONAL_TESTS ini_functional_tests)
    set(INI_INTEGRATION_TESTS ini_integration_tests)
    set(INI_STRESS_TESTS ini_stress_tests)

    enable_testing()

    # ====== File System Tests =================================================== #
    add_executable(${INI_FILESYSTEM_TESTS} tests/ini_filesystem_tests.c)
    target_link_libraries(${INI_FILESYSTEM_TESTS} PRIVATE ${PROJECT_NAME})

    # ====== Hash Table Tests ==================================================== #
    add_executable(${INI_HASH_TABLE_TESTS} tests/ini_hash_table_tests.c)
    target_link_libraries(${INI_HASH_TABLE_TESTS} PRIVATE ${PROJECT_NAME})

    # ====== Mutex Tests ========================================================= #
    add_executable(${INI_MUTEX_TESTS} tests/ini_mutex_tests.c)
    target_link_libraries(${INI_MUTEX_TESTS} PRIVATE ${PROJECT_NAME})

    # ====== Parser Tests ========================================================= #
    add_executable(${INI_PARSER_TESTS} tests/ini_parser_tests.c)
    target_link_libraries(${INI_PARSER_TESTS} PRIVATE ${PROJECT_NAME})

    # ====== Other types of tests ================================================== #
    add_executable(${INI_FUNCTIONAL_TESTS} tests/ini_functional_tests.c)
    target_link_libraries(${INI_FUNCTIONAL_TESTS} PRIVATE ${PROJECT_NAME})
    add_executable(${INI_INTEGRATION_TESTS} tests/ini_integration_tests.c)
    target_link_libraries(${INI_INTEGRATION_TESTS} PRIVATE ${PROJECT_NAME})
    add_executable(${INI_STRESS_TESTS} tests/ini_stress_tests.c)
    target_link_libraries(${INI_STRESS_TESTS} PRIVATE ${PROJECT_NAME})

    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
    add_test(NAME ${INI_FILESYSTEM_TESTS} COMMAND ${INI_FILESYSTEM_TESTS})
    add_test(NAME ${INI_HASH_TABLE_TESTS} COMMAND ${INI_HASH_TABLE_TESTS})
    add_test(NAME ${INI_MUTEX_TESTS} COMMAND ${INI_MUTEX_TESTS})
    add_test(NAME ${INI_PARSER_TESTS} COMMAND ${INI_PARSER_TESTS})

    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
    add_test(NAME ${INI_FUNCTIONAL_TESTS} COMMAND ${INI_FUNCTIONAL_TESTS})
    add_test(NAME ${INI_INTEGRATION_TESTS} COMMAND ${INI_INTEGRATION_TESTS})
    add_test(NAME ${INI_STRESS_TESTS} COMMAND ${INI_STRESS_TESTS})

    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
endif()
