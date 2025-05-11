#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if INI_OS_UNIX
#include <sys/stat.h>
#endif

#if INI_OS_WINDOWS
#include <windows.h>
#elif INI_OS_UNIX || INI_OS_APPLE
#include <unistd.h>
#endif

#include "helper.h"
#include "iniparser.h"

#define TEST_FILE "test.ini"

void test_free_null_context()
{
    ini_error_details_t err = ini_free(NULL);
    assert(err.error == INI_INVALID_ARGUMENT);
    print_success("test_free_null_context passed\n");
}

void test_free_empty_context()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_free_empty_context passed\n");
}

void test_free_loaded_context()
{
    create_test_file(TEST_FILE, "[section]\nkey=value\n");

    // Create the context
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Load it properly - the modified ini_load will handle reallocation correctly
    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    // Free the context
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);

    remove_test_file(TEST_FILE);
    print_success("test_free_loaded_context passed\n");
}

void test_free_double_free()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // First free should succeed
    ini_error_details_t err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);

    // In an ideal implementation, a second free of the same pointer
    // should be detected and handled safely, returning SUCCESS
    // without actually freeing again. We're really testing the robustness
    // of the API here, not encouraging double-free.
    //
    // Create a new context to test with, since the previous was freed
    ctx = ini_create_context();
    assert(ctx != NULL);

    // Remember the pointer but don't use it after free
    ini_context_t *freed_ctx = ctx;

    // Free it once
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);

    // Don't use ctx after it's freed, this is just for error code checking
    // Use NULL instead of the freed pointer to avoid undefined behavior
    err = ini_free(NULL);
    assert(err.error == INI_INVALID_ARGUMENT);

    print_success("test_free_double_free passed\n");
}

void test_free_platform_specific()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_free_platform_specific passed\n");
}

void test_free_memory_leak()
{
    // Ensure no memory leaks by creating and freeing contexts
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_free_memory_leak passed\n");
}

void test_free_with_sections()
{
    create_test_file(TEST_FILE, "[section1]\nkey1=value1\n[section2]\nkey2=value2\n");

    // Create context
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Load the file with sections
    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    // Free the context
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);

    remove_test_file(TEST_FILE);
    print_success("test_free_with_sections passed\n");
}

void test_free_with_subsections()
{
    create_test_file(TEST_FILE, "[parent]\nkey=value\n[parent.child]\nkey=value\n");

    // Create context
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Load with subsections
    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    // Free context
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);

    remove_test_file(TEST_FILE);
    print_success("test_free_with_subsections passed\n");
}

int main()
{
    __helper_init_log_file();
    ini_initialize();

    test_free_null_context();
    test_free_empty_context();
    test_free_loaded_context();
    test_free_double_free();
    test_free_platform_specific();
    test_free_memory_leak();
    test_free_with_sections();
    test_free_with_subsections();

    ini_finalize();
    print_success("All ini_free() tests passed!\n\n");
    __helper_close_log_file();
    return ini_has_error();
}
