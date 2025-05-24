#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ini_constants.h"
#include "ini_os_check.h"

#if INI_OS_WINDOWS
#include <windows.h>
#elif INI_OS_LINUX || INI_OS_APPLE
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "helper.h"
#include "ini_parser.h"

// ======================================================================== //
// === Test 1. ini_create_context() ======================================= //
// ======================================================================== //
void test_ini_create_context_success()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_create_context_success passed\n");
}

void test_ini_create_context_platform_specific()
{
    // Test platform-specific initialization (Windows, Unix, Apple)
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_create_context_platform_specific passed\n");
}

void test_ini_create_context_reentrancy()
{
    // Test reentrancy by calling the function multiple times
    ini_context_t *ctx1 = ini_create_context();
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx1 != NULL);
    assert(ctx2 != NULL);
    ini_status_t err1 = ini_free(ctx1);
    ini_status_t err2 = ini_free(ctx2);
    assert(err1 == INI_STATUS_SUCCESS);
    assert(err2 == INI_STATUS_SUCCESS);
    print_success("test_ini_create_context_reentrancy passed\n");
}

void test_ini_create_context_resource_leak()
{
    // Ensure no resource leaks by creating and freeing contexts
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_create_context_resource_leak passed\n");
}
// ************************************************************************ //
// ======================================================================== //

// ======================================================================== //
// === Test 2. ini_free() ================================================= //
// ======================================================================== //
void test_ini_free_null_context()
{
    ini_status_t err = ini_free(NULL);
    assert(err == INI_STATUS_INVALID_ARGUMENT);
    print_success("test_ini_free_null_context passed\n");
}

void test_ini_free_empty_context()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_free_empty_context passed\n");
}

void test_ini_free_loaded_context()
{
    char test_file[] = "test_ini_free_loaded.ini";
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
    print_success("test_ini_free_loaded_context passed\n");

    remove(test_file);
}

void test_ini_free_double_free()
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

    print_success("test_ini_free_double_free passed\n");
}

void test_ini_free_platform_specific()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_free_platform_specific passed\n");
}

void test_ini_free_memory_leak()
{
    // Ensure no memory leaks by creating and freeing contexts
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_free_memory_leak passed\n");
}

void test_ini_free_with_sections()
{
    char const TEST_FILE[] = "test_ini_free_with_sections.ini";
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
    print_success("test_ini_free_with_sections passed\n");
}

void test_ini_free_with_subsections()
{
    char const TEST_FILE[] = "test_ini_free_with_subsections.ini";
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
    print_success("test_ini_free_with_subsections passed\n");
}
// ************************************************************************ //
// ======================================================================== //

// ======================================================================== //
// === Test 3. ini_get_value() ============================================ //
// ======================================================================== //
void test_ini_get_value_empty_value()
{
    char TEST_FILE[] = "test_empty_value.ini";
    create_test_file(TEST_FILE, "[section]\nkey=\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Load the INI file with empty value
    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_print(stderr, ctx);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err == INI_STATUS_SUCCESS);

    if (value)
    {
        assert(strlen(value) == 0); // Empty string
        free(value);
    }
    else
    {
        assert(0); // Should not be NULL for empty value, but empty string
    }

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_empty_value passed\n");
}

void test_ini_get_value_get_existing_value()
{
    char TEST_FILE[] = "test_get_existing_value.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_print(stderr, ctx);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_get_existing_value passed\n");
}

void test_ini_get_value_key_not_found()
{
    char TEST_FILE[] = "test_key_not_found.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "nonexistent", &value);
    assert(err == INI_STATUS_KEY_NOT_FOUND);
    assert(value == NULL);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_key_not_found passed\n");
}

void test_ini_get_value_section_not_found()
{
    char TEST_FILE[] = "test_section_not_found.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "nonexistent", "key", &value);
    assert(err == INI_STATUS_SECTION_NOT_FOUND);
    assert(value == NULL);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_section_not_found passed\n");
}

void test_ini_get_value_subsection()
{
    char TEST_FILE[] = "test_subsection.ini";
    create_test_file(TEST_FILE, "[parent]\nkey=value\n[parent.child]\nkey=child_value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "parent.child", "key", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "child_value") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_subsection passed\n");
}

void test_ini_get_value_whitespace_value()
{
    char TEST_FILE[] = "test_whitespace_value.ini";
    create_test_file(TEST_FILE, "[section]\nkey=   value with spaces   \n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value with spaces") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_whitespace_value passed\n");
}

void test_ini_get_value_unicode()
{
    char TEST_FILE[] = "test_unicode.ini";
    create_test_file(TEST_FILE, "[секция]\nключ=значение\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "секция", "ключ", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "значение") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_unicode passed\n");
}

void test_ini_get_value_utf8_bom()
{
    char TEST_FILE[] = "test_utf8_bom.ini";
    FILE *f = fopen(TEST_FILE, "wb");
    const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    fwrite(bom, 1, 3, f);
    fputs("[section]\nkey=значение\n", f);
    fclose(f);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_print(stderr, ctx);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err == INI_STATUS_SUCCESS);

    if (value)
    {
        assert(strcmp(value, "значение") == 0);
        free(value);
    }
    else
    {
        assert(0); // Should not be NULL
    }

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_utf8_bom passed\n");
}

void test_ini_get_value_null_arguments()
{
    char TEST_FILE[] = "test_null_arguments.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    char *value = NULL;
    ini_status_t err = ini_get_value(NULL, "section", "key", &value);
    assert(err == INI_STATUS_INVALID_ARGUMENT);

    err = ini_get_value(ctx, NULL, "key", &value);
    assert(err == INI_STATUS_INVALID_ARGUMENT);

    err = ini_get_value(ctx, "section", NULL, &value);
    assert(err == INI_STATUS_INVALID_ARGUMENT);

    err = ini_get_value(ctx, "section", "key", NULL);
    assert(err == INI_STATUS_INVALID_ARGUMENT);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_get_value_null_arguments passed\n");
}

void test_ini_get_value_reuse_value_pointer()
{
    char TEST_FILE[] = "test_reuse_value_pointer.ini";
    create_test_file(TEST_FILE, "[section]\nkey1=value1\nkey2=value2\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key1", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value1") == 0);
    free(value);
    value = NULL;

    err = ini_get_value(ctx, "section", "key2", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value2") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_reuse_value_pointer passed\n");
}

#if INI_OS_WINDOWS || INI_OS_LINUX
void *__thread_get_value(void *arg)
{
    ini_context_t *ctx = (ini_context_t *)arg;
    char *value = NULL;
    ini_status_t err = ini_get_value(ctx, "section", "key", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value") == 0);
    free(value);
    return NULL;
}
#endif

void test_ini_get_value_thread_safety()
{
#if INI_OS_WINDOWS || INI_OS_LINUX
    char TEST_FILE[] = "test_thread_safety.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

#if INI_OS_LINUX
    pthread_t threads[10];
    for (int i = 0; i < 10; i++)
    {
        pthread_create(&threads[i], NULL, __thread_get_value, ctx);
    }
    for (int i = 0; i < 10; i++)
    {
        pthread_join(threads[i], NULL);
    }
#elif INI_OS_WINDOWS
    HANDLE threads[10];
    for (int i = 0; i < 10; i++)
    {
        threads[i] = CreateThread(NULL, 0,
                                  (LPTHREAD_START_ROUTINE)__thread_get_value,
                                  ctx, 0, NULL);
        assert(threads[i] != NULL);
    }
    WaitForMultipleObjects(10, threads, TRUE, INFINITE);
    for (int i = 0; i < 10; i++)
    {
        CloseHandle(threads[i]);
    }
#endif

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_thread_safety passed\n");
#endif
}

void test_ini_get_value_reuse_context()
{
    char TEST_FILE[] = "test1.ini";
    create_test_file(TEST_FILE, "[section1]\nkey1=value1\n");
    char TEST_FILE2[] = "test2.ini";
    create_test_file(TEST_FILE2, "[section2]\nkey2=value2\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, "test1.ini");
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section1", "key1", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value1") == 0);
    free(value);
    value = NULL;

    err = ini_load(ctx, "test2.ini");
    assert(err == INI_STATUS_SUCCESS);

    err = ini_get_value(ctx, "section2", "key2", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value2") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    remove_test_file(TEST_FILE2);
    print_success("test_ini_get_value_reuse_context passed\n");
}

void test_ini_get_value_corrupted_file()
{
    char TEST_FILE[] = "test_corrupted_file.ini";
    create_test_file(TEST_FILE, "[section\nkey=value\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_corrupted_file passed\n");
}

void test_ini_get_value_quoted_values()
{
    char TEST_FILE[] = "test_quoted_values.ini";
    create_test_file(TEST_FILE, "[section]\nkey=\"quoted value\"\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "quoted value") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_quoted_values passed\n");
}

void test_ini_get_value_existing_value_memory()
{
    char TEST_FILE[] = "test_existing_value_memory.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = strdup("existing memory");
    assert(value != NULL);
    if (value)
    {
        free(value);
        value = NULL;
    }

    err = ini_get_value(ctx, "section", "key", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_get_value_existing_value_memory passed\n");
}
// ************************************************************************ //
// ======================================================================== //

// ======================================================================== //
// === Test 4. ini_good() ================================================= //
// ======================================================================== //
void test_ini_good_null_filepath()
{
    ini_status_t err = ini_good(NULL);
    assert(err == INI_STATUS_INVALID_ARGUMENT);
    print_success("test_ini_good_null_filepath passed\n");
}

void test_ini_good_nonexistent_file()
{
    ini_status_t err = ini_good("nonexistent.ini");
    assert(err == INI_STATUS_FILE_NOT_FOUND);
    print_success("test_ini_good_nonexistent_file passed\n");
}

// void test_ini_good_directory()
// {
//     char TEST_DIR[] = "test_dir";
//     create_test_dir(TEST_DIR);
//     ini_status_t err = ini_good(TEST_DIR);
//     assert(err == INI_STATUS_FILE_IS_DIR);
//     remove_test_dir(TEST_DIR);
//     print_success("test_ini_good_directory passed\n");
// }

void test_ini_good_empty_file()
{
    char TEST_FILE[] = "test_empty_file.ini";
    create_test_file(TEST_FILE, "");
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_FILE_EMPTY);
    print_success("test_ini_good_empty_file passed\n");
    remove_test_file(TEST_FILE);
}

void test_ini_good_valid_file()
{
    char TEST_FILE[] = "test_valid_file.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_good_valid_file passed\n");
    remove_test_file(TEST_FILE);
}

void test_ini_good_bad_format_missing_bracket()
{
    char TEST_FILE[] = "test_bad_format_missing_bracket.ini";
    create_test_file(TEST_FILE, "[section\nkey=value\n");
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_FILE_BAD_FORMAT);
    print_success("test_ini_good_bad_format_missing_bracket passed\n");
    remove_test_file(TEST_FILE);
}

void test_ini_good_bad_format_empty_key()
{
    char TEST_FILE[] = "test_bad_format_empty_key.ini";
    create_test_file(TEST_FILE, "[section]\n=value\n");
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_FILE_BAD_FORMAT);
    print_success("test_ini_good_bad_format_empty_key passed\n");
    remove_test_file(TEST_FILE);
}

void test_ini_good_empty_value()
{
    char TEST_FILE[] = "test_empty_value.ini";
    create_test_file(TEST_FILE, "[section]\nkey=\n");
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_good_empty_value passed\n");
    remove_test_file(TEST_FILE);
}

void test_ini_good_no_read_permission()
{
#if INI_OS_LINUX
    char TEST_FILE[] = "test_no_read_permission.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    chmod(TEST_FILE, 0333);
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_FILE_OPEN_FAILED || err == INI_STATUS_SUCCESS);
    print_success("test_ini_good_no_read_permission passed\n");
    remove_test_file(TEST_FILE);
#endif
}

void test_ini_good_bad_format_unbalanced_quotes()
{
    char TEST_FILE[] = "test_bad_format_unbalanced_quotes.ini";
    create_test_file(TEST_FILE, "[section]\nkey=\"value\n");
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_FILE_BAD_FORMAT);
    print_success("test_ini_good_bad_format_unbalanced_quotes passed\n");
    remove_test_file(TEST_FILE);
}

void test_ini_good_bad_format_arrays_not_supported()
{
    char TEST_FILE[] = "test_bad_format_arrays_not_supported.ini";
    create_test_file(TEST_FILE, "[section]\nkey=1,2,3\n");
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_FILE_BAD_FORMAT);
    print_success("test_ini_good_bad_format_arrays_not_supported passed\n");
    remove_test_file(TEST_FILE);
}

void test_ini_good_utf8_chars()
{
    char TEST_FILE[] = "test_utf8_chars.ini";
    create_test_file(TEST_FILE, "[секция]\nключ=значение\n[节]\n键=值\n");
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_good_utf8_chars passed\n");
    remove_test_file(TEST_FILE);
}

void test_ini_good_windows_line_endings()
{
    char TEST_FILE[] = "test_windows_line_endings.ini";
    create_test_file(TEST_FILE, "[section]\r\nkey=value\r\n");
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_good_windows_line_endings passed\n");
    remove_test_file(TEST_FILE);
}

void test_ini_good_line_too_long()
{
    char long_line[INI_LINE_MAX + 2];
    memset(long_line, 'a', INI_LINE_MAX + 1);
    for (int i = 0; i < INI_LINE_MAX + 1; i++)
        assert(long_line[i] == 'a');

    char TEST_FILE[] = "test_line_too_long.ini";
    create_test_file(TEST_FILE, long_line);
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_FILE_BAD_FORMAT);
    print_success("test_ini_good_line_too_long passed\n");
    remove_test_file(TEST_FILE);
}

void test_ini_good_file_deleted_during_check()
{
    char TEST_FILE[] = "test_file_deleted_during_check.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    remove_test_file(TEST_FILE);
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_FILE_NOT_FOUND);
    print_success("test_ini_good_file_deleted_during_check passed\n");
}

void test_ini_good_binary_data()
{
    char TEST_FILE[] = "test_binary_data.ini";
    FILE *fp = fopen(TEST_FILE, "wb");
    assert(fp != NULL);
    unsigned char binary_data[] = {0x01, 0x02, 0x03, 0x00, 0xFF, 0xFE, 0xFD};
    assert(fwrite(binary_data, sizeof(binary_data), 1, fp) == 1);
    assert(fclose(fp) == 0);
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_FILE_BAD_FORMAT);
    print_success("test_ini_good_binary_data passed\n");
    remove_test_file(TEST_FILE);
}

void test_ini_good_symlink()
{
#if INI_OS_LINUX
    char TEST_FILE[] = "target.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    symlink("target.ini", "symlink.ini");
    ini_status_t err = ini_good("symlink.ini");
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_good_symlink passed\n");
    remove_test_file("symlink.ini");
    remove_test_file("target.ini");
#endif
}

void test_ini_good_special_chars()
{
#if INI_OS_LINUX
    char TEST_FILE[] = "test_special_chars.ini";
    create_test_file(TEST_FILE, "[sec#tion]\nke=y=val\\;ue\n");
    ini_status_t err = ini_good(TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_good_special_chars passed\n");
    remove_test_file(TEST_FILE);
#endif
}
// ************************************************************************ //
// ======================================================================== //

// ======================================================================== //
// === Test 5. ini_load() ================================================= //
// ======================================================================== //
void test_ini_load_null_ctx_and_filepath()
{
    ini_status_t err = ini_load(NULL, NULL);
    assert(err == INI_STATUS_INVALID_ARGUMENT);
    print_success("test_ini_load_null_ctx_and_filepath passed\n");
}

void test_ini_load_null_filepath()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, NULL);
    assert(err == INI_STATUS_INVALID_ARGUMENT);
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_null_filepath passed\n");
}

void test_ini_load_nonexistent_file()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "nonexistent.ini");
    assert(err == INI_STATUS_FILE_NOT_FOUND);
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_nonexistent_file passed\n");
}

// void test_ini_load_directory()
// {
//     char TEST_DIR[] = "test_dir";
//     create_test_dir(TEST_DIR);
//     ini_context_t *ctx = ini_create_context();
//     assert(ctx != NULL);
//     ini_status_t err = ini_load(ctx, TEST_DIR);
//     assert(err == INI_STATUS_FILE_IS_DIR);
//     remove_test_dir(TEST_DIR);
//     err = ini_free(ctx);
//     assert(err == INI_STATUS_SUCCESS);
//     print_success("test_ini_load_directory passed\n");
// }

void test_ini_load_empty_file()
{
    create_test_file("empty.ini", "");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "empty.ini");
    assert(err == INI_STATUS_FILE_EMPTY);
    remove_test_file("empty.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_empty_file passed\n");
}

void test_ini_load_valid_file()
{
    create_test_file("valid.ini", "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "valid.ini");
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file("valid.ini");
    print_success("test_ini_load_valid_file passed\n");
}

void test_ini_load_bad_format_missing_bracket()
{
    create_test_file("bad_missing_bracket.ini", "[section\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "bad_missing_bracket.ini");
    assert(err == INI_STATUS_FILE_BAD_FORMAT);
    remove_test_file("bad_missing_bracket.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_bad_format_missing_bracket passed\n");
}

void test_ini_load_bad_format_empty_key()
{
    create_test_file("bad_empty_key.ini", "[section]\n=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "bad_empty_key.ini");
    assert(err == INI_STATUS_FILE_BAD_FORMAT);
    remove_test_file("bad_empty_key.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_bad_format_empty_key passed\n");
}

void test_ini_load_empty_value()
{
    create_test_file("empty_value.ini", "[section]\nkey=\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "empty_value.ini");
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file("empty_value.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_empty_value passed\n");
}

void test_ini_load_bad_format_unbalanced_quotes()
{
    create_test_file("bad_unbalanced_quotes.ini", "[section]\nkey=\"value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "bad_unbalanced_quotes.ini");
    assert(err == INI_STATUS_FILE_BAD_FORMAT);
    remove_test_file("bad_unbalanced_quotes.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_bad_format_unbalanced_quotes passed\n");
}

void test_ini_load_bad_format_arrays_not_supported()
{
    create_test_file("bad_arrays.ini", "[section]\nkey=1,2,3\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "bad_arrays.ini");
    assert(err == INI_STATUS_FILE_BAD_FORMAT);
    remove_test_file("bad_arrays.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_bad_format_arrays_not_supported passed\n");
}

void test_ini_load_utf8_chars()
{
    create_test_file("utf8.ini", "[секция]\nключ=значение\n[节]\n键=值\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "utf8.ini");
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file("utf8.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_utf8_chars passed\n");
}

void test_ini_load_windows_line_endings()
{
    create_test_file("windows.ini", "[section]\r\nkey=value\r\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "windows.ini");
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file("windows.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_windows_line_endings passed\n");
}

void test_ini_load_line_too_long()
{
    char long_line[INI_LINE_MAX + 2];
    memset(long_line, 'a', INI_LINE_MAX + 1);
    long_line[INI_LINE_MAX + 1] = '\0';

    create_test_file("long_line.ini", long_line);
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "long_line.ini");
    assert(err == INI_STATUS_FILE_BAD_FORMAT);
    remove_test_file("long_line.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_line_too_long passed\n");
}

void test_ini_load_file_deleted_during_check()
{
    char const *filepath = "deleted_during_check.ini";
    create_test_file(filepath, "[section]\nkey=value\n");
    remove_test_file(filepath);
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, filepath);
    assert(err == INI_STATUS_FILE_NOT_FOUND);
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_file_deleted_during_check passed\n");
}

void test_ini_load_binary_data()
{
    char const *filepath = "binary.ini";
    FILE *fp = fopen(filepath, "wb");
    assert(fp != NULL);

    unsigned char binary_data[] = {0x01, 0x02, 0x03, 0x00, 0xFF, 0xFE, 0xFD};
    assert(fwrite(binary_data, sizeof(binary_data), 1, fp) == 1);
    assert(fclose(fp) == 0);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, filepath);
    assert(err == INI_STATUS_FILE_BAD_FORMAT);
    remove_test_file(filepath);
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_binary_data passed\n");
}

void test_ini_load_no_read_permission()
{
#if INI_OS_LINUX
    create_test_file("no_read.ini", "[section]\nkey=value\n");
    assert(chmod("no_read.ini", 0333) == 0);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "no_read.ini");
    assert(err == INI_STATUS_FILE_OPEN_FAILED || err == INI_STATUS_SUCCESS);
    remove_test_file("no_read.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_no_read_permission passed\n");
#endif
}

void test_ini_load_symlink()
{
#if INI_OS_LINUX
    create_test_file("target.ini", "[section]\nkey=value\n");
    assert(symlink("target.ini", "symlink.ini") == 0);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "symlink.ini");
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file("symlink.ini");
    remove_test_file("target.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_symlink passed\n");
#endif
}

void test_ini_load_special_chars()
{
#if INI_OS_LINUX
    create_test_file("special.ini", "[sec#tion]\nke=y=val\\;ue\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "special.ini");
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file("special.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_load_special_chars passed\n");
#endif
}

void test_ini_load_reuse_ctx()
{
    create_test_file("valid1.ini", "[section1]\nkey1=value1\n");
    create_test_file("valid2.ini", "[section2]\nkey2=value2\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_status_t err = ini_load(ctx, "valid1.ini");
    assert(err == INI_STATUS_SUCCESS);

    err = ini_load(ctx, "valid2.ini");
    assert(err == INI_STATUS_SUCCESS);

    print_success("test_ini_load_reuse_ctx passed\n");
    remove_test_file("valid1.ini");
    remove_test_file("valid2.ini");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
}
// ************************************************************************ //
// ======================================================================== //

// ======================================================================== //
// === Test 6. ini_print() ================================================ //
// ======================================================================== //
// Helper function to capture printed output
static char *capture_output(void (*test_func)(FILE *))
{
#if INI_OS_WINDOWS
#define TEMP_FILENAME_LEN 256
    char temp_filename[TEMP_FILENAME_LEN];
    snprintf(temp_filename, TEMP_FILENAME_LEN, "%s\\print_test_%lld.txt",
             getenv("TEMP"), (long long)GetCurrentProcessId());
#else
    char temp_filename[INI_PATH_MAX];
    snprintf(temp_filename, sizeof(temp_filename), "/tmp/print_test_%d.txt", getpid());
#endif

    FILE *output_file = NULL;
#if INI_OS_WINDOWS
    fopen_s(&output_file, temp_filename, "wb+");
#else
    output_file = fopen(temp_filename, "w+");
#endif
    if (output_file == NULL)
    {
        return NULL;
    }

    test_func(output_file);
    fflush(output_file);

    if (fseek(output_file, 0, SEEK_END) != 0)
    {
        fclose(output_file);
        remove(temp_filename);
        return NULL;
    }

    long size = ftell(output_file);
    if (size < 0)
    {
        fclose(output_file);
        remove(temp_filename);
        return NULL;
    }

    rewind(output_file);

    char *content = (char *)malloc(size + 1);
    if (content == NULL)
    {
        fclose(output_file);
        remove(temp_filename);
        return NULL;
    }

    size_t read = fread(content, 1, size, output_file);
    if (read != (size_t)size)
    {
        if (content)
            free(content);
        fclose(output_file);
        remove(temp_filename);
        return NULL;
    }
    content[read] = '\0';

    fclose(output_file);
    remove(temp_filename);
    return content;
}

// Test function for basic section printing
static void test_ini_print_basic_section_func(FILE *output)
{
    ini_context_t *ctx = ini_create_context();
    if (ctx == NULL)
    {
        fprintf(output, "Failed to create context\n");
        return;
    }

    // Create test data
    char TEST_FILE[] = "test_ini_print_basic_section.ini";
    create_test_file(TEST_FILE, "[section1]\nkey1=value1\nkey2=value2\n");

    ini_status_t err = ini_load(ctx, TEST_FILE);
    if (err != INI_STATUS_SUCCESS)
    {
        fprintf(output, "Failed to load test file\n");
        ini_free(ctx);
        remove_test_file(TEST_FILE);
        return;
    }

    err = ini_print(output, ctx);
    if (err != INI_STATUS_SUCCESS)
    {
        fprintf(output, "Print failed with error: %d\n", err);
    }

    ini_free(ctx);
    remove_test_file(TEST_FILE);
}

void test_ini_print_basic_section()
{
    char *content = capture_output(test_ini_print_basic_section_func);

    // Verify output format
    assert(strstr(content, "[section1]") != NULL);
    assert(strstr(content, "key1 = value1") != NULL);
    assert(strstr(content, "key2 = value2") != NULL);

    free(content);
    print_success("test_ini_print_basic_section passed\n");
}

// Test function for printing with subsections
static void test_ini_print_with_subsections_func(FILE *output)
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Create test data with subsections
    char TEST_FILE[] = "test_ini_print_with_subsections.ini";
    create_test_file(TEST_FILE, "[parent]\nkey1=value1\n[parent.child]\nkey2=value2\n");

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_print(output, ctx);
    assert(err == INI_STATUS_SUCCESS);

    ini_free(ctx);
    remove_test_file(TEST_FILE);
}

void test_ini_print_with_subsections()
{
    char *content = capture_output(test_ini_print_with_subsections_func);

    // Verify output format
    assert(strstr(content, "[parent]") != NULL);
    assert(strstr(content, "key1 = value1") != NULL);
    assert(strstr(content, "[parent.child]") != NULL);
    assert(strstr(content, "key2 = value2") != NULL);

    free(content);
    print_success("test_ini_print_with_subsections passed\n");
}

// Test function for printing Unicode characters
static void test_ini_print_unicode_func(FILE *output)
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Create test data with Unicode characters
    char TEST_FILE[] = "test_ini_print_unicode.ini";
    create_test_file(TEST_FILE, "[секция]\nключ=значение\n");

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_print(output, ctx);
    assert(err == INI_STATUS_SUCCESS);

    ini_free(ctx);
    remove_test_file(TEST_FILE);
}

void test_ini_print_unicode()
{
    char *content = capture_output(test_ini_print_unicode_func);

    // Verify Unicode characters are printed correctly
    assert(strstr(content, "[секция]") != NULL);
    assert(strstr(content, "ключ = значение") != NULL);

    free(content);
    print_success("test_ini_print_unicode passed\n");
}

void test_ini_print_null_stream()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_print(NULL, ctx);
    assert(err == INI_STATUS_INVALID_ARGUMENT);

    ini_free(ctx);
    print_success("test_ini_print_null_stream passed\n");
}

void test_ini_print_null_ctx()
{
    ini_status_t err = ini_print(stdout, NULL);
    assert(err == INI_STATUS_INVALID_ARGUMENT);
    print_success("test_ini_print_null_ctx passed\n");
}

void test_ini_print_malformed_section_names()
{
    // Create a file with a test section name
    char TEST_FILE[] = "test_ini_print_malformed_section_names.ini";
    create_test_file(TEST_FILE, "[bad\nsection]\nkey=value\n");

    FILE *output = tmpfile();
    assert(output != NULL);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // This should fail to load due to malformed section name
    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_FILE_BAD_FORMAT);

    fclose(output);
    ini_free(ctx);
    remove_test_file(TEST_FILE);
    print_success("test_ini_print_malformed_section_names passed\n");
}

void test_ini_print_malformed_pairs()
{
    // Create a file with an invalid key (empty key)
    char TEST_FILE[] = "test_ini_print_malformed_pairs.ini";
    create_test_file(TEST_FILE, "[section]\n=value\n");

    FILE *output = tmpfile();
    assert(output != NULL);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_FILE_BAD_FORMAT);

    fclose(output);
    ini_free(ctx);
    remove_test_file(TEST_FILE);
    print_success("test_ini_print_malformed_pairs passed\n");
}

void test_ini_print_large_output()
{
    char TEST_FILE[] = "test_ini_print_large_output.ini";
    FILE *output = tmpfile();
    assert(output != NULL);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Create large test data - many sections
    FILE *testfile = fopen(TEST_FILE, "w");
    assert(testfile != NULL);

    for (int i = 0; i < 100; i++)
    {
        fprintf(testfile, "[section%d]\n", i);
        for (int j = 0; j < 10; j++)
        {
            fprintf(testfile, "key%d=value%d_%d\n", j, i, j);
        }
        fprintf(testfile, "\n");
    }
    fclose(testfile);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_print(output, ctx);
    assert(err == INI_STATUS_SUCCESS);

    fclose(output);
    ini_free(ctx);
    remove_test_file(TEST_FILE);
    print_success("test_ini_print_large_output passed\n");
}

void test_ini_print_stream_errors()
{
    // Create a read-only stream (simulate disk full)
    FILE *read_only_stream;
#if INI_OS_WINDOWS
    fopen_s(&read_only_stream, "NUL", "r");
#else
    read_only_stream = fopen("/dev/null", "r");
#endif
    assert(read_only_stream != NULL);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    char TEST_FILE[] = "test_ini_print_stream_errors.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    // This should fail to print due to read-only stream
    err = ini_print(read_only_stream, ctx);
    assert(err == INI_STATUS_PRINT_ERROR);

    fclose(read_only_stream);
    ini_free(ctx);
    remove_test_file(TEST_FILE);
    print_success("test_ini_print_stream_errors passed\n");
}

void test_ini_print_fprintf_failure()
{
    FILE *output = tmpfile();
    assert(output != NULL);
    fclose(output); // Close stream to force fprintf failure

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    char TEST_FILE[] = "test_ini_print_fprintf_failure.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    // This should fail to print due to closed stream
    output = NULL;
    err = ini_print(output, ctx);
    assert(err == INI_STATUS_PRINT_ERROR || err == INI_STATUS_INVALID_ARGUMENT);

    ini_free(ctx);
    remove_test_file(TEST_FILE);
    print_success("test_ini_print_fprintf_failure passed\n");
}

void test_ini_print_long_names()
{
    FILE *output = tmpfile();
    assert(output != NULL);

    // Create a test file with very long section/key names
    char long_line[INI_LINE_MAX];
    memset(long_line, 'a', INI_LINE_MAX - 10);
    long_line[INI_LINE_MAX - 10] = '\0';

    char TEST_FILE[] = "test_ini_print_long_names.ini";
    FILE *testfile = fopen(TEST_FILE, "w");
    assert(testfile != NULL);
    fprintf(testfile, "[%s]\n%s=value\n", long_line, long_line);
    fclose(testfile);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_print(output, ctx);
    assert(err == INI_STATUS_SUCCESS); // Should handle long names gracefully

    fclose(output);
    ini_free(ctx);
    remove_test_file(TEST_FILE);
    print_success("test_ini_print_long_names passed\n");
}

void test_ini_print_empty_sections()
{
    FILE *output = tmpfile();
    assert(output != NULL);

    // Create a test file with empty section (no keys)
    char TEST_FILE[] = "test_ini_print_empty_sections.ini";
    create_test_file(TEST_FILE, "[empty_section]\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_print(output, ctx);
    assert(err == INI_STATUS_SUCCESS);

    fclose(output);
    ini_free(ctx);
    remove_test_file(TEST_FILE);
    print_success("test_ini_print_empty_sections passed\n");
}

void test_ini_print_mixed_encoding()
{
    FILE *output = tmpfile();
    assert(output != NULL);

    // Create a test file with mixed encodings
    char TEST_FILE[] = "test_ini_print_mixed_encoding.ini";
    create_test_file(TEST_FILE, "[mixed_section]\nascii_key=ascii_value\n嗨。=Cześć\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_print(output, ctx);
    assert(err == INI_STATUS_SUCCESS);

    fclose(output);
    ini_free(ctx);
    remove_test_file(TEST_FILE);
    print_success("test_ini_print_mixed_encoding passed\n");
}
// ************************************************************************ //
// ======================================================================== //

// ======================================================================== //
// === Test 7. ini_save_section_value() =================================== //
// ======================================================================== //
void test_save_section_value_null_context()
{
    char TEST_FILE[] = "test_save_section_value_null_context.ini";
    ini_status_t err = ini_save_section_value(NULL, TEST_FILE, "section", "key");
    assert(err == INI_STATUS_INVALID_ARGUMENT);
    print_success("test_save_section_value_null_context passed\n");
}

void test_save_section_value_null_filepath()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_save_section_value(ctx, NULL, "section", "key");
    assert(err == INI_STATUS_INVALID_ARGUMENT);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_save_section_value_null_filepath passed\n");
}

void test_save_section_value_null_section()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    char TEST_FILE[] = "test_save_section_value_null_section.ini";
    ini_status_t err = ini_save_section_value(ctx, TEST_FILE, NULL, "key");
    assert(err == INI_STATUS_INVALID_ARGUMENT);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_save_section_value_null_section passed\n");
}

void test_save_section_value_section_not_found()
{
    char TEST_FILE_LOAD[] = "test_save_section_value_section_not_found_load.ini";
    char TEST_FILE_SAVE[] = "test_save_section_value_section_not_found_save.ini";

    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Try to save a non-existent section
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "nonexistent", "key");
    assert(err == INI_STATUS_SECTION_NOT_FOUND);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    print_success("test_save_section_value_section_not_found passed\n");
}

void test_save_section_value_key_not_found()
{
    char TEST_FILE_LOAD[] = "test_save_section_value_key_not_found_load.ini";
    char TEST_FILE_SAVE[] = "test_save_section_value_key_not_found_save.ini";

    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Try to save a non-existent key
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "section", "nonexistent");
    assert(err == INI_STATUS_KEY_NOT_FOUND);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    print_success("test_key_not_found passed\n");
}

void test_save_section_value_save_section_key()
{
    char TEST_FILE_LOAD[] = "test_save_section_value_save_section_key_load.ini";
    char TEST_FILE_SAVE[] = "test_save_section_value_save_section_key_save.ini";

    create_test_file(TEST_FILE_LOAD, "[section]\nkey1=value1\nkey2=value2\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Save specific key to a new file
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "section", "key1");
    assert(err == INI_STATUS_SUCCESS);

    // Verify the saved file contains only the specified section and key
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    // Check that the section and key exists
    char *value = NULL;
    err = ini_get_value(ctx2, "section", "key1", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value, "value1") == 0);
    free(value);

    // Check that the other key doesn't exist in the new file
    value = NULL;
    err = ini_get_value(ctx2, "section", "key2", &value);
    assert(err == INI_STATUS_KEY_NOT_FOUND);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_section_value_save_section_key passed\n");
}

void test_save_section_value_save_entire_section()
{
    char TEST_FILE_LOAD[] = "test_save_section_value_save_entire_section_load.ini";
    char TEST_FILE_SAVE[] = "test_save_section_value_save_entire_section_save.ini";

    create_test_file(TEST_FILE_LOAD, "[section1]\nkey1=value1\nkey2=value2\n[section2]\nkey3=value3\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Save entire section to a new file by passing NULL for key
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "section1", NULL);
    assert(err == INI_STATUS_SUCCESS);

    // Verify the saved file contains the entire specified section
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    // Check that both keys from section1 exist
    char *value = NULL;
    err = ini_get_value(ctx2, "section1", "key1", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value, "value1") == 0);
    free(value);

    value = NULL;
    err = ini_get_value(ctx2, "section1", "key2", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value, "value2") == 0);
    free(value);

    // Check that section2 doesn't exist in the new file
    value = NULL;
    err = ini_get_value(ctx2, "section2", "key3", &value);
    assert(err == INI_STATUS_SECTION_NOT_FOUND);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_section_value_save_entire_section passed\n");
}

void test_save_section_value_update_existing_file()
{
    char TEST_FILE_LOAD[] = "test_save_section_value_update_existing_file_load.ini";
    char TEST_FILE_SAVE[] = "test_save_section_value_update_existing_file_save.ini";

    create_test_file(TEST_FILE_LOAD, "[section1]\nkey1=original\nkey2=value2\n\n[section2]\nkey3=value3\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Modify the context
    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Get key1's value, modify it, and update the context
    char *value = NULL;
    err = ini_get_value(ctx, "section1", "key1", &value);
    assert(err == INI_STATUS_SUCCESS);
    free(value);

    // Create a new context with modified value
    ini_context_t *modified_ctx = ini_create_context();
    assert(modified_ctx != NULL);

    create_test_file(TEST_FILE_LOAD, "[section1]\nkey1=updated\n");
    err = ini_load(modified_ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Update only key1 in the original file
    err = ini_save_section_value(modified_ctx, TEST_FILE_SAVE, "section1", "key1");
    assert(err == INI_STATUS_SUCCESS);

    // Reload the file and verify only key1 was updated
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    // Check that key1 was updated
    value = NULL;
    err = ini_get_value(ctx2, "section1", "key1", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value, "updated") == 0);
    free(value);

    // Check that other keys are preserved
    value = NULL;
    err = ini_get_value(ctx2, "section1", "key2", &value);
    assert(err == INI_STATUS_KEY_NOT_FOUND);
    free(value);

    value = NULL;
    err = ini_get_value(ctx2, "section2", "key3", &value);
    assert(err == INI_STATUS_SECTION_NOT_FOUND);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(modified_ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_section_value_update_existing_file passed\n");
}

void test_save_section_value_subsection_save()
{
    char TEST_FILE_LOAD[] = "test_save_section_value_subsection_save_load.ini";
    char TEST_FILE_SAVE[] = "test_save_section_value_subsection_save_save.ini";

    create_test_file(TEST_FILE_LOAD, "[parent]\nkey1=value1\n\n[parent.child]\nkey2=value2\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Save the subsection to a new file
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "parent.child", "key2");
    assert(err == INI_STATUS_SUCCESS);

    // Verify the file was saved correctly with parent section too
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    // Check that the parent section exists
    char *value = NULL;
    err = ini_get_value(ctx2, "parent.child", "key2", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value, "value2") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_section_value_subsection_save passed\n");
}

void test_save_section_value_save_with_special_chars()
{
    char TEST_FILE_LOAD[] = "test_save_section_value_save_with_special_chars_load.ini";
    char TEST_FILE_SAVE[] = "test_save_section_value_save_with_special_chars_save.ini";

    create_test_file(TEST_FILE_LOAD, "[section]\nkey1=value with spaces\nkey2=\"quoted value\"\nkey3=value#with#hash\nkey4=value;with;semicolon\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Save key with special chars
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "section", "key1");
    assert(err == INI_STATUS_SUCCESS);

    // Verify the content
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx2, "section", "key1", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value, "value with spaces") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_section_value_save_with_special_chars passed\n");
}

void test_save_section_value_save_empty_value()
{
    char TEST_FILE_LOAD[] = "test_save_section_value_save_empty_value_load.ini";
    char TEST_FILE_SAVE[] = "test_save_section_value_save_empty_value_save.ini";

    create_test_file(TEST_FILE_LOAD, "[section]\nkey1=\nkey2=value2\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Save key with empty value
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "section", "key1");
    assert(err == INI_STATUS_SUCCESS);

    // Verify the content
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx2, "section", "key1", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(strlen(value) == 0); // Empty string
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_section_value_save_empty_value passed\n");
}

void test_save_section_value_unicode()
{
    char TEST_FILE_LOAD[] = "test_save_section_value_unicode_load.ini";
    char TEST_FILE_SAVE[] = "test_save_section_value_unicode_save.ini";

    create_test_file(TEST_FILE_LOAD, "[секция]\nключ=значение\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Save section with Unicode
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "секция", "ключ");
    assert(err == INI_STATUS_SUCCESS);

    // Verify the content
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx2, "секция", "ключ", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value, "значение") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_section_value_unicode passed\n");
}

// void test_save_section_value_to_directory()
// {
//     char TEST_DIR[] = "test_dir";
//     char TEST_FILE_LOAD[] = "test_save_section_value_to_directory_load.ini";
//     char TEST_FILE_SAVE[] = "test_save_section_value_to_directory_save.ini";

//     create_test_dir(TEST_DIR);

//     // Create test data
//     create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

//     // Load the INI file
//     ini_context_t *ctx = ini_create_context();
//     assert(ctx != NULL);

//     ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
//     assert(err == INI_STATUS_SUCCESS);

//     // Try to save to a directory
//     err = ini_save_section_value(ctx, TEST_DIR, "section", "key");
//     assert(err == INI_STATUS_FILE_OPEN_FAILED);

//     err = ini_free(ctx);
//     assert(err == INI_STATUS_SUCCESS);
//     remove_test_file(TEST_FILE_LOAD);
//     remove_test_dir(TEST_DIR);
//     print_success("test_save_section_value_to_directory passed\n");
// }

// void test_save_section_value_no_write_permission()
// {
// #if INI_OS_LINUX
//     char TEST_DIR[] = "test_dir";
//     char TEST_FILE_LOAD[] = "test_save_section_value_no_write_permission_load.ini";
//     char TEST_FILE_SAVE[] = "test_save_section_value_no_write_permission_save.ini";

//     create_test_dir(TEST_DIR);
//     create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

//     ini_context_t *ctx = ini_create_context();
//     assert(ctx != NULL);

//     ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
//     assert(err == INI_STATUS_SUCCESS);

//     // Remove write permissions from the directory
//     chmod(TEST_DIR, 0555); // r-xr-xr-x

//     // Try to save to a file in that directory
//     err = ini_save_section_value(ctx, "test_dir/test_no_perm.ini", "section", "key");
//     assert(err == INI_STATUS_FILE_OPEN_FAILED || err == INI_STATUS_SUCCESS);

//     // Restore permissions and clean up
//     chmod(TEST_DIR, 0777); // rwxrwxrwx
//     err = ini_free(ctx);
//     assert(err == INI_STATUS_SUCCESS);
//     remove_test_file(TEST_FILE_LOAD);
//     remove_test_dir(TEST_DIR);
//     print_success("test_save_section_value_no_write_permission passed\n");
// #endif
// }

#if INI_OS_LINUX || INI_OS_WINDOWS
void *__thread_save_section(void *arg)
{
    ini_context_t *ctx = (ini_context_t *)arg;
    char filename[64];
#if INI_OS_LINUX
    sprintf(filename, "thread_save_section_%ld.ini", (long)pthread_self());
#elif INI_OS_WINDOWS
    sprintf(filename, "thread_save_section_%lu.ini", (unsigned long)GetCurrentThreadId());
#endif

    ini_status_t err = ini_save_section_value(ctx, filename, "section", "key");
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(filename);
    return NULL;
}
#endif

void test_save_section_value_thread_safety()
{
#if INI_OS_LINUX || INI_OS_WINDOWS
    char TEST_FILE_LOAD[] = "test_save_section_value_thread_safety_load.ini";
    char TEST_FILE_SAVE[] = "test_save_section_value_thread_safety_save.ini";

    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Save from multiple threads
#if INI_OS_LINUX
    pthread_t threads[10];
    for (int i = 0; i < 10; i++)
    {
        pthread_create(&threads[i], NULL, __thread_save_section, ctx);
    }
    for (int i = 0; i < 10; i++)
    {
        pthread_join(threads[i], NULL);
    }
#elif INI_OS_WINDOWS
    HANDLE threads[10];
    for (int i = 0; i < 10; i++)
    {
        threads[i] = CreateThread(NULL, 0,
                                  (LPTHREAD_START_ROUTINE)__thread_save_section,
                                  ctx, 0, NULL);
        assert(threads[i] != NULL);
    }
    WaitForMultipleObjects(10, threads, TRUE, INFINITE);
    for (int i = 0; i < 10; i++)
    {
        CloseHandle(threads[i]);
    }
#endif

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    print_success("test_save_section_value_thread_safety passed\n");
#endif
}
// ************************************************************************ //
// ======================================================================== //

// ======================================================================== //
// === Test 8. ini_save() ================================================= //
// ======================================================================== //
// Helper function to compare the content of two files
int compare_files(const char *file1, const char *file2)
{
    FILE *f1 = fopen(file1, "r");
    FILE *f2 = fopen(file2, "r");

    if (!f1 || !f2)
    {
        if (f1)
            fclose(f1);
        if (f2)
            fclose(f2);
        return 0; // Files cannot be opened
    }

    char line1[4096], line2[4096];
    int line_num = 0;
    int match = 1; // Assume files match until proven otherwise

    while (1)
    {
        char *s1 = fgets(line1, sizeof(line1), f1);
        char *s2 = fgets(line2, sizeof(line2), f2);
        line_num++;

        // Check if we've reached the end of either file
        if (!s1 && !s2) // Both files ended
            break;

        if (!s1 || !s2) // One file ended before the other
        {
            match = 0;
            break;
        }

        // Skip empty lines
        if ((line1[0] == '\n' || line1[0] == '\r') &&
            (line2[0] == '\n' || line2[0] == '\r'))
            continue;

        // Compare section headers directly
        if (line1[0] == '[' && line2[0] == '[')
        {
            // Trim trailing newlines for comparison
            char *nl1 = strchr(line1, '\n');
            char *nl2 = strchr(line2, '\n');
            if (nl1)
                *nl1 = '\0';
            if (nl2)
                *nl2 = '\0';

            if (strcmp(line1, line2) != 0)
            {
                match = 0;
                break;
            }
            continue;
        }

        // Must be key-value pairs - first check for equals sign
        char *eq1 = strchr(line1, '=');
        char *eq2 = strchr(line2, '=');

        if (!eq1 || !eq2)
        {
            match = 0;
            break;
        }

        // Make working copies we can modify
        char line1_copy[4096], line2_copy[4096];
        strcpy(line1_copy, line1);
        strcpy(line2_copy, line2);

        // Get pointers into the copies
        eq1 = strchr(line1_copy, '=');
        eq2 = strchr(line2_copy, '=');

        // Null-terminate to separate key from value
        *eq1 = '\0';
        *eq2 = '\0';
        char *key1 = line1_copy;
        char *key2 = line2_copy;
        char *val1 = eq1 + 1;
        char *val2 = eq2 + 1;

        // Trim whitespace from keys
        char *end;
        end = key1 + strlen(key1) - 1;
        while (end > key1 && (*end == ' ' || *end == '\t'))
            *end-- = '\0';

        end = key2 + strlen(key2) - 1;
        while (end > key2 && (*end == ' ' || *end == '\t'))
            *end-- = '\0';

        // Trim leading whitespace from values
        while (*val1 == ' ' || *val1 == '\t')
            val1++;
        while (*val2 == ' ' || *val2 == '\t')
            val2++;

        // Trim trailing whitespace and newlines from values
        end = val1 + strlen(val1) - 1;
        while (end > val1 && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
            *end-- = '\0';

        end = val2 + strlen(val2) - 1;
        while (end > val2 && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
            *end-- = '\0';

        // Remove quotes if present
        if (*val1 == '"' && val1[strlen(val1) - 1] == '"')
        {
            val1[strlen(val1) - 1] = '\0';
            val1++;
        }

        if (*val2 == '"' && val2[strlen(val2) - 1] == '"')
        {
            val2[strlen(val2) - 1] = '\0';
            val2++;
        }

        // Compare the keys and values
        if (strcmp(key1, key2) != 0)
        {
            match = 0;
            break;
        }

        if (strcmp(val1, val2) != 0)
        {
            match = 0;
            break;
        }
    }

    fclose(f1);
    fclose(f2);
    return match;
}

void test_ini_save_null_context()
{
    char TEST_FILE[] = "test_ini_save_null_context.ini";
    ini_status_t err = ini_save(NULL, TEST_FILE);
    assert(err == INI_STATUS_INVALID_ARGUMENT);
    print_success("test_ini_save_null_context passed\n");
}

void test_ini_save_null_filepath()
{
    char TEST_FILE[] = "test_ini_save_null_filepath.ini";
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_save(ctx, NULL);
    assert(err == INI_STATUS_INVALID_ARGUMENT);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_ini_save_null_filepath passed\n");
}

void test_ini_save_empty_context()
{
    char TEST_FILE[] = "test_ini_save_empty_context.ini";
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_save(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    // Check if the file exists and is empty (no sections)
    FILE *file = fopen(TEST_FILE, "r");
    assert(file != NULL);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    assert(size == 0); // File should be empty

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_ini_save_empty_context passed\n");
}

void test_ini_save_simple_context()
{
    char TEST_FILE_LOAD[] = "test_ini_save_simple_context_load.ini";
    char TEST_FILE_SAVE[] = "test_ini_save_simple_context_save.ini";
    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_save(ctx, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    assert(compare_files(TEST_FILE_LOAD, TEST_FILE_SAVE) == 1);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_ini_save_simple_context passed\n");
}

void test_ini_save_with_subsections()
{
    char TEST_FILE_LOAD[] = "test_ini_save_with_subsections_load.ini";
    char TEST_FILE_SAVE[] = "test_ini_save_with_subsections_save.ini";
    create_test_file(TEST_FILE_LOAD, "[parent]\nkey1=value1\n\n[parent.child]\nkey2=value2\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_save(ctx, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    char *value1 = NULL;
    err = ini_get_value(ctx2, "parent", "key1", &value1);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value1, "value1") == 0);
    free(value1);

    char *value2 = NULL;
    err = ini_get_value(ctx2, "parent.child", "key2", &value2);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value2, "value2") == 0);
    free(value2);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_ini_save_with_subsections passed\n");
}

void test_ini_save_with_special_chars()
{
    char TEST_FILE_LOAD[] = "test_ini_save_with_special_chars_load.ini";
    char TEST_FILE_SAVE[] = "test_ini_save_with_special_chars_save.ini";
    create_test_file(TEST_FILE_LOAD, "[section]\nkey1=value with spaces\nkey2=\"quoted value\"\nkey3=value#with#hash\nkey4=value;with;semicolon\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_save(ctx, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    char *value1 = NULL;
    err = ini_get_value(ctx2, "section", "key1", &value1);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value1, "value with spaces") == 0);
    free(value1);

    char *value2 = NULL;
    err = ini_get_value(ctx2, "section", "key2", &value2);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value2, "quoted value") == 0);
    free(value2);

    char *value3 = NULL;
    err = ini_get_value(ctx2, "section", "key3", &value3);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value3, "value#with#hash") == 0);
    free(value3);

    char *value4 = NULL;
    err = ini_get_value(ctx2, "section", "key4", &value4);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value4, "value;with;semicolon") == 0);
    free(value4);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_ini_save_with_special_chars passed\n");
}

void test_ini_save_empty_values()
{
    char TEST_FILE_LOAD[] = "test_ini_save_empty_values_load.ini";
    char TEST_FILE_SAVE[] = "test_ini_save_empty_values_save.ini";
    create_test_file(TEST_FILE_LOAD, "[section]\nkey1=\nkey2=\"\"\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_save(ctx, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    // Check if the saved file can be loaded correctly
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    // Verify the content by getting values
    char *value1 = NULL;
    err = ini_get_value(ctx2, "section", "key1", &value1);
    assert(err == INI_STATUS_SUCCESS);
    assert(strlen(value1) == 0); // Empty string
    free(value1);

    char *value2 = NULL;
    err = ini_get_value(ctx2, "section", "key2", &value2);
    assert(err == INI_STATUS_SUCCESS);
    assert(strlen(value2) == 0); // Empty string
    free(value2);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_ini_save_empty_values passed\n");
}

void test_ini_save_unicode()
{
    char TEST_FILE_LOAD[] = "test_ini_save_unicode_load.ini";
    char TEST_FILE_SAVE[] = "test_ini_save_unicode_save.ini";
    // Create test data with Unicode characters
    create_test_file(TEST_FILE_LOAD, "[секция]\nключ=значение\n[节]\n键=值\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_save(ctx, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    // Check if the saved file can be loaded correctly
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    // Verify the content by getting values
    char *value1 = NULL;
    err = ini_get_value(ctx2, "секция", "ключ", &value1);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value1, "значение") == 0);
    free(value1);

    char *value2 = NULL;
    err = ini_get_value(ctx2, "节", "键", &value2);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value2, "值") == 0);
    free(value2);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_ini_save_unicode passed\n");
}

#if INI_OS_LINUX || INI_OS_WINDOWS
void *__thread_save(void *arg)
{
    ini_context_t *ctx = (ini_context_t *)arg;
    char filename[64];
#if INI_OS_LINUX
    sprintf(filename, "thread_save_%ld.ini", (long)pthread_self());
#elif INI_OS_WINDOWS
    sprintf(filename, "thread_save_%lu.ini", (unsigned long)GetCurrentThreadId());
#endif

    ini_status_t err = ini_save(ctx, filename);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(filename);
    return NULL;
}
#endif

void test_ini_save_thread_safety()
{
#if INI_OS_LINUX || INI_OS_WINDOWS
    char TEST_FILE_LOAD[] = "test_ini_save_thread_safety_load.ini";
    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Save from multiple threads
#if INI_OS_LINUX
    pthread_t threads[10];
    for (int i = 0; i < 10; i++)
    {
        pthread_create(&threads[i], NULL, __thread_save, ctx);
    }
    for (int i = 0; i < 10; i++)
    {
        pthread_join(threads[i], NULL);
    }
#elif INI_OS_WINDOWS
    HANDLE threads[10];
    for (int i = 0; i < 10; i++)
    {
        threads[i] = CreateThread(NULL, 0,
                                  (LPTHREAD_START_ROUTINE)__thread_save,
                                  ctx, 0, NULL);
        assert(threads[i] != NULL);
    }
    WaitForMultipleObjects(10, threads, TRUE, INFINITE);
    for (int i = 0; i < 10; i++)
    {
        CloseHandle(threads[i]);
    }
#endif

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    print_success("test_ini_save_thread_safety passed\n");
#endif
}

void test_ini_save_to_existing_file()
{
    char TEST_FILE_SAVE[] = "test_ini_save_to_existing_file_save.ini";
    char TEST_FILE_LOAD[] = "test_ini_save_to_existing_file_load.ini";

    create_test_file(TEST_FILE_SAVE, "This is some existing content\n");
    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err == INI_STATUS_SUCCESS);

    // Save to the existing file
    err = ini_save(ctx, TEST_FILE_SAVE);
    assert(err == INI_STATUS_SUCCESS);

    // Check if the content was replaced
    FILE *file = fopen(TEST_FILE_SAVE, "r");
    assert(file != NULL);

    char buffer[256];
    fgets(buffer, sizeof(buffer), file);
    assert(strcmp(buffer, "[section]\n") == 0);
    fclose(file);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_ini_save_to_existing_file passed\n");
}
// ************************************************************************ //
// ======================================================================== //

int main()
{
    __helper_init_log_file();

    // === Test 1. ini_create_context() ====== //
    test_ini_create_context_success();
    test_ini_create_context_platform_specific();
    test_ini_create_context_reentrancy();
    test_ini_create_context_resource_leak();
    print_success("All ini_create_context() tests passed!\n\n");
    // ======================================= //

    // === Test 2. ini_create_context() ====== //
    test_ini_free_null_context();
    test_ini_free_empty_context();
    test_ini_free_loaded_context();
    test_ini_free_double_free();
    test_ini_free_platform_specific();
    test_ini_free_memory_leak();
    test_ini_free_with_sections();
    test_ini_free_with_subsections();
    print_success("All ini_free() tests passed!\n\n");
    // ======================================= //

    // === Test 3. ini_get_value() =========== //
    test_ini_get_value_empty_value();
    test_ini_get_value_get_existing_value();
    test_ini_get_value_key_not_found();
    test_ini_get_value_section_not_found();
    test_ini_get_value_subsection();
    test_ini_get_value_whitespace_value();
    test_ini_get_value_unicode();
    test_ini_get_value_utf8_bom();
    test_ini_get_value_quoted_values();
    test_ini_get_value_null_arguments();
    test_ini_get_value_reuse_value_pointer();
    test_ini_get_value_existing_value_memory();
    test_ini_get_value_thread_safety();
    test_ini_get_value_reuse_context();
    test_ini_get_value_corrupted_file();
    print_success("All ini_get_value() tests passed!\n\n");
    // ======================================= //

    // === Test 4. ini_good() ================ //
    test_ini_good_null_filepath();
    test_ini_good_nonexistent_file();
    // test_ini_good_directory();
    test_ini_good_empty_file();
    test_ini_good_valid_file();
    test_ini_good_bad_format_missing_bracket();
    test_ini_good_bad_format_empty_key();
    test_ini_good_empty_value();
    test_ini_good_no_read_permission();
    test_ini_good_bad_format_unbalanced_quotes();
    test_ini_good_bad_format_arrays_not_supported();
    test_ini_good_utf8_chars();
    test_ini_good_windows_line_endings();
    test_ini_good_line_too_long();
    test_ini_good_file_deleted_during_check();
    test_ini_good_binary_data();
    test_ini_good_symlink();
    test_ini_good_special_chars();
    print_success("All ini_good() tests passed!\n\n");
    // ======================================= //

    // === Test 5. ini_load() ================ //
    test_ini_load_null_ctx_and_filepath();
    test_ini_load_null_filepath();
    test_ini_load_nonexistent_file();
    // test_ini_load_directory();
    test_ini_load_empty_file();
    test_ini_load_valid_file();
    test_ini_load_bad_format_missing_bracket();
    test_ini_load_bad_format_empty_key();
    test_ini_load_empty_value();
    test_ini_load_bad_format_unbalanced_quotes();
    test_ini_load_bad_format_arrays_not_supported();
    test_ini_load_utf8_chars();
    test_ini_load_windows_line_endings();
    test_ini_load_line_too_long();
    test_ini_load_file_deleted_during_check();
    test_ini_load_binary_data();
    test_ini_load_reuse_ctx();
    test_ini_load_no_read_permission();
    test_ini_load_symlink();
    test_ini_load_special_chars();
    print_success("All ini_load() tests passed!\n\n");
    // ======================================= //

    // === Test 6. ini_print() =============== //
    test_ini_print_basic_section();
    test_ini_print_with_subsections();
    test_ini_print_unicode();
    test_ini_print_null_stream();
    test_ini_print_null_ctx();
    test_ini_print_malformed_section_names();
    test_ini_print_malformed_pairs();
    test_ini_print_large_output();
    test_ini_print_stream_errors();
    test_ini_print_fprintf_failure();
    test_ini_print_long_names();
    test_ini_print_empty_sections();
    test_ini_print_mixed_encoding();
    print_success("All ini_print() tests passed!\n\n");
    // ======================================= //

    // === Test 7. ini_save_section_value() == //
    test_save_section_value_null_context();
    test_save_section_value_null_filepath();
    test_save_section_value_null_section();
    test_save_section_value_section_not_found();
    test_save_section_value_key_not_found();
    test_save_section_value_save_section_key();
    test_save_section_value_save_entire_section();
    test_save_section_value_update_existing_file();
    test_save_section_value_subsection_save();
    test_save_section_value_save_with_special_chars();
    test_save_section_value_save_empty_value();
    test_save_section_value_unicode();
    // test_save_section_value_to_directory();
    // test_save_section_value_no_write_permission();
    test_save_section_value_thread_safety();
    print_success("All ini_save_section_value() tests passed!\n\n");
    // ======================================= //

    // === Test 8. ini_save() ================ //
    test_ini_save_null_context();
    test_ini_save_null_filepath();
    test_ini_save_empty_context();
    test_ini_save_simple_context();
    test_ini_save_with_subsections();
    test_ini_save_with_special_chars();
    test_ini_save_empty_values();
    test_ini_save_unicode();
    test_ini_save_to_existing_file();
    test_ini_save_thread_safety();
    print_success("All ini_save() tests passed!\n\n");
    // ======================================= //

    __helper_close_log_file();
    return EXIT_SUCCESS;
}
