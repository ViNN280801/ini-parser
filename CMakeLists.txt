cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME iniparser)
project(${PROJECT_NAME}
    VERSION 1.0.0
    LANGUAGES C CXX
    DESCRIPTION "Lightweight INI parser library for C. Also, it has a C++11 wrapper."
)

# ================ Main settings ================
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(INIPARSER_TESTS "Build tests" ON)
option(INIPARSER_EXAMPLES "Build examples" ON)

# Build type settings - default to STATIC for tests
if(NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS OFF)
endif()

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

# C Library
include_directories(${INI_INCLUDE_DIRS})
add_library(${PROJECT_NAME} ${INI_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Set C library properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    OUTPUT_NAME ${PROJECT_NAME}
    POSITION_INDEPENDENT_CODE ON
)

# Define INIPARSER_EXPORTS for the library itself
target_compile_definitions(${PROJECT_NAME} PRIVATE INIPARSER_EXPORTS)

# For static libraries, consumers should not import symbols
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} INTERFACE INI_IMPLEMENTATION)
endif()

# Windows DLL export settings
if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

# ================ C++ Wrapper Library ====================
set(CPP_WRAPPER_NAME ${PROJECT_NAME}_cpp)
set(CPP_SOURCE_FILES
    ${INI_SOURCE_FILE_DIR}/IniParser.cpp
)

# C++ Wrapper Library
add_library(${CPP_WRAPPER_NAME} ${CPP_SOURCE_FILES})

target_include_directories(${CPP_WRAPPER_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link C++ wrapper to C library
target_link_libraries(${CPP_WRAPPER_NAME} PUBLIC ${PROJECT_NAME})

# Set C++ library properties
set_target_properties(${CPP_WRAPPER_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME ${PROJECT_NAME}_cpp
    POSITION_INDEPENDENT_CODE ON
)

# Windows DLL export settings for C++ wrapper
if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(${CPP_WRAPPER_NAME} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

# ================ Examples ======================
if(INIPARSER_EXAMPLES)
    set(CPP_EXAMPLE_NAME cpp_example)

    # C++ Example
    add_executable(${CPP_EXAMPLE_NAME}
        examples/cpp_example.cpp
    )

    # Link example to C++ wrapper (which automatically pulls in C library)
    target_link_libraries(${CPP_EXAMPLE_NAME} PRIVATE ${CPP_WRAPPER_NAME})

    # Set C++ example properties
    set_target_properties(${CPP_EXAMPLE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
    )
endif()

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
    # For some reasons, Github Actions fails to run filesystem tests.
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
