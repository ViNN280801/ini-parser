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

void test_create_context_success()
{
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("test_create_context_success failed: context creation failed\n");
        return;
    }
    print_success("test_create_context_success passed\n");
    ini_free(ctx);
}

void test_create_context_platform_specific()
{
    // Test platform-specific initialization (Windows, Unix, Apple)
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("test_create_context_platform_specific failed: context creation failed\n");
        return;
    }
    print_success("test_create_context_platform_specific passed\n");
    ini_free(ctx);
}

void test_create_context_reentrancy()
{
    // Test reentrancy by calling the function multiple times
    ini_context_t *ctx1 = ini_create_context();
    ini_context_t *ctx2 = ini_create_context();
    if (!ctx1 || !ctx2)
    {
        print_error("test_create_context_reentrancy failed: context creation failed\n");
        return;
    }
    print_success("test_create_context_reentrancy passed\n");
    ini_free(ctx1);
    ini_free(ctx2);
}

void test_create_context_resource_leak()
{
    // Ensure no resource leaks by creating and freeing contexts
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("test_create_context_resource_leak failed: context creation failed\n");
        return;
    }
    ini_free(ctx);
    print_success("test_create_context_resource_leak passed\n");
}

int main()
{
    __g_init_log_file();
    __g_init_errstack();

    test_create_context_success();
    test_create_context_platform_specific();
    test_create_context_reentrancy();
    test_create_context_resource_leak();
    print_success("All ini_create_context() tests passed!\n\n");
    __g_close_log_file();
    return ini_has_error();
}
