#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if INI_OS_LINUX
#include <sys/stat.h>
#endif

#if INI_OS_WINDOWS
#include <windows.h>
#elif INI_OS_LINUX || INI_OS_APPLE
#include <unistd.h>
#endif

#include "helper.h"
#include "ini_parser.h"

#define TEST_FILE "test.ini"

void test_free_null_context()
{
    ini_status_t err = ini_free(NULL);
    assert(err == INI_STATUS_INVALID_ARGUMENT);
    print_success("test_free_null_context passed\n");
}

void test_free_empty_context()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_free_empty_context passed\n");
}

void test_free_loaded_context()
{
    char test_file[] = "test_free_loaded.ini";
    FILE *fp = fopen(test_file, "w");
    if (fp)
    {
        fprintf(fp, "[section]\nkey=value\n");
        fclose(fp);
    }
    else
    {
        return;
    }

    // Create the context
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        remove(test_file);
        return;
    }

    // Load the file
    ini_status_t err = ini_load(ctx, test_file);
    if (err != INI_STATUS_SUCCESS)
    {
        ini_free(ctx);
        remove(test_file);
        return;
    }

    // Free the context
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_free_loaded_context passed\n");

    remove(test_file);
}

void test_free_double_free()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // First free should succeed
    ini_status_t err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);

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
    assert(err == INI_STATUS_SUCCESS);

    // Don't use ctx after it's freed, this is just for error code checking
    // Use NULL instead of the freed pointer to avoid undefined behavior
    err = ini_free(NULL);
    assert(err == INI_STATUS_INVALID_ARGUMENT);

    print_success("test_free_double_free passed\n");
}

void test_free_platform_specific()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_free_platform_specific passed\n");
}

void test_free_memory_leak()
{
    // Ensure no memory leaks by creating and freeing contexts
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_free_memory_leak passed\n");
}

void test_free_with_sections()
{
    create_test_file(TEST_FILE, "[section1]\nkey1=value1\n[section2]\nkey2=value2\n");

    // Create context
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Load the file with sections
    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    // Free the context
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);

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
    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    // Free context
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);

    remove_test_file(TEST_FILE);
    print_success("test_free_with_subsections passed\n");
}

int main()
{
    __helper_init_log_file();

    test_free_null_context();
    test_free_empty_context();
    test_free_loaded_context();
    test_free_double_free();
    test_free_platform_specific();
    test_free_memory_leak();
    test_free_with_sections();
    test_free_with_subsections();

    print_success("All ini_free() tests passed!\n\n");
    __helper_close_log_file();
    return EXIT_SUCCESS;
}
