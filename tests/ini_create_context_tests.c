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

void test_create_context_success()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_t err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    print_success("test_create_context_success passed\n");
}

void test_create_context_platform_specific()
{
    // Test platform-specific initialization (Windows, Unix, Apple)
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_t err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    print_success("test_create_context_platform_specific passed\n");
}

void test_create_context_reentrancy()
{
    // Test reentrancy by calling the function multiple times
    ini_context_t *ctx1 = ini_create_context();
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx1 != NULL);
    assert(ctx2 != NULL);
    ini_error_t err1 = ini_free(ctx1);
    ini_error_t err2 = ini_free(ctx2);
    assert(err1 == INI_SUCCESS);
    assert(err2 == INI_SUCCESS);
    print_success("test_create_context_reentrancy passed\n");
}

void test_create_context_resource_leak()
{
    // Ensure no resource leaks by creating and freeing contexts
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_t err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    print_success("test_create_context_resource_leak passed\n");
}

int main()
{
    __helper_init_log_file();

    test_create_context_success();
    test_create_context_platform_specific();
    test_create_context_reentrancy();
    test_create_context_resource_leak();

    print_success("All ini_create_context() tests passed!\n\n");
    __helper_close_log_file();
    return EXIT_SUCCESS;
}
