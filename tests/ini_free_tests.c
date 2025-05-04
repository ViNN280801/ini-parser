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
    if (err.error != INI_INVALID_ARGUMENT)
    {
        print_error("test_free_null_context failed: expected INI_INVALID_ARGUMENT, got %d\n", err.error);
        return;
    }
    print_success("test_free_null_context passed\n");
}

void test_free_empty_context()
{
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("test_free_empty_context failed: context creation failed\n");
        return;
    }
    ini_error_details_t err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
    {
        print_error("test_free_empty_context failed: expected INI_SUCCESS, got %d\n", err.error);
        return;
    }
    print_success("test_free_empty_context passed\n");
}

void test_free_loaded_context()
{
    create_test_file("test.ini", "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("test_free_loaded_context failed: context creation failed\n");
        remove_test_file("test.ini");
        return;
    }
    ini_load(ctx, "test.ini");
    ini_error_details_t err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
    {
        print_error("test_free_loaded_context failed: expected INI_SUCCESS, got %d\n", err.error);
        remove_test_file("test.ini");
        return;
    }
    print_success("test_free_loaded_context passed\n");
    remove_test_file("test.ini");
}

void test_free_double_free()
{
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("test_free_double_free failed: context creation failed\n");
        return;
    }
    ini_free(ctx);
    ini_error_details_t err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
    {
        print_error("test_free_double_free failed: double free should fail\n");
        return;
    }
    print_success("test_free_double_free passed\n");
}

void test_free_platform_specific()
{
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("test_free_platform_specific failed: context creation failed\n");
        return;
    }
    ini_error_details_t err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
    {
        print_error("test_free_platform_specific failed: expected INI_SUCCESS, got %d\n", err.error);
        return;
    }
    print_success("test_free_platform_specific passed\n");
}

void test_free_memory_leak()
{
    // Ensure no memory leaks by creating and freeing contexts
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("test_free_memory_leak failed: context creation failed\n");
        return;
    }
    ini_free(ctx);
    print_success("test_free_memory_leak passed\n");
}

void test_free_with_sections()
{
    create_test_file("test.ini", "[section1]\nkey1=value1\n[section2]\nkey2=value2\n");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("test_free_with_sections failed: context creation failed\n");
        remove_test_file("test.ini");
        return;
    }
    ini_load(ctx, "test.ini");
    ini_error_details_t err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
    {
        print_error("test_free_with_sections failed: expected INI_SUCCESS, got %d\n", err.error);
        remove_test_file("test.ini");
        return;
    }
    print_success("test_free_with_sections passed\n");
    remove_test_file("test.ini");
}

void test_free_with_subsections()
{
    create_test_file("test.ini", "[parent]\nkey=value\n[parent.child]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("test_free_with_subsections failed: context creation failed\n");
        remove_test_file("test.ini");
        return;
    }
    ini_load(ctx, "test.ini");
    ini_error_details_t err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
    {
        print_error("test_free_with_subsections failed: expected INI_SUCCESS, got %d\n", err.error);
        remove_test_file("test.ini");
        return;
    }
    print_success("test_free_with_subsections passed\n");
    remove_test_file("test.ini");
}

int main()
{
    __g_init_log_file();
    __g_init_errstack();

    test_free_null_context();
    test_free_empty_context();
    test_free_loaded_context();
    test_free_double_free();
    test_free_platform_specific();
    test_free_memory_leak();
    test_free_with_sections();
    test_free_with_subsections();
    print_success("All ini_free() tests passed!\n\n");
    __g_close_log_file();
    return ini_has_error();
}
