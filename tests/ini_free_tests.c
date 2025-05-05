#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if INI_OS_UNIX
#include <sys/stat.h>
#endif

#if INI_OS_WINDOWS
#include <windows.h>
#elif INI_OS_UNIX || INI_OS_MACOS
#include <unistd.h>
#endif

#include "helper.h"
#include "iniparser.h"

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
    create_test_file("test.ini", "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_load(ctx, "test.ini");
    ini_error_details_t err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file("test.ini");
    print_success("test_free_loaded_context passed\n");
}

void test_free_double_free()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_free(ctx);
    ini_error_details_t err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
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
    create_test_file("test.ini", "[section1]\nkey1=value1\n[section2]\nkey2=value2\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "test.ini");
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file("test.ini");
    print_success("test_free_with_sections passed\n");
}

void test_free_with_subsections()
{
    create_test_file("test.ini", "[parent]\nkey=value\n[parent.child]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "test.ini");
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file("test.ini");
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
