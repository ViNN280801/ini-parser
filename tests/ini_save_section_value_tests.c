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
#include <sys/stat.h> // For chmod() function
#include <unistd.h>
#endif

#include "helper.h"
#include "iniparser.h"

#define TEST_FILE "test_save_section.ini"
#define TEST_FILE_LOAD "test_load_section.ini"
#define TEST_FILE_SAVE "test_save_section_output.ini"

void test_null_context()
{
    ini_error_details_t err = ini_save_section_value(NULL, TEST_FILE, "section", "key");
    assert(err.error == INI_INVALID_ARGUMENT);
    print_success("test_null_context passed\n");
}

void test_null_filepath()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_save_section_value(ctx, NULL, "section", "key");
    assert(err.error == INI_INVALID_ARGUMENT);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_null_filepath passed\n");
}

void test_null_section()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_save_section_value(ctx, TEST_FILE, NULL, "key");
    assert(err.error == INI_INVALID_ARGUMENT);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_null_section passed\n");
}

void test_section_not_found()
{
    // Create test data
    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Try to save a non-existent section
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "nonexistent", "key");
    assert(err.error == INI_SECTION_NOT_FOUND);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    print_success("test_section_not_found passed\n");
}

void test_key_not_found()
{
    // Create test data
    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Try to save a non-existent key
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "section", "nonexistent");
    assert(err.error == INI_KEY_NOT_FOUND);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    print_success("test_key_not_found passed\n");
}

void test_save_section_key()
{
    // Create test data
    create_test_file(TEST_FILE_LOAD, "[section]\nkey1=value1\nkey2=value2\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save specific key to a new file
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "section", "key1");
    assert(err.error == INI_SUCCESS);

    // Verify the saved file contains only the specified section and key
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Check that the section and key exists
    char *value = NULL;
    err = ini_get_value(ctx2, "section", "key1", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value1") == 0);
    free(value);

    // Check that the other key doesn't exist in the new file
    value = NULL;
    err = ini_get_value(ctx2, "section", "key2", &value);
    assert(err.error == INI_KEY_NOT_FOUND);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_section_key passed\n");
}

void test_save_entire_section()
{
    // Create test data
    create_test_file(TEST_FILE_LOAD, "[section1]\nkey1=value1\nkey2=value2\n[section2]\nkey3=value3\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save entire section to a new file by passing NULL for key
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "section1", NULL);
    assert(err.error == INI_SUCCESS);

    // Verify the saved file contains the entire specified section
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Check that both keys from section1 exist
    char *value = NULL;
    err = ini_get_value(ctx2, "section1", "key1", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value1") == 0);
    free(value);

    value = NULL;
    err = ini_get_value(ctx2, "section1", "key2", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value2") == 0);
    free(value);

    // Check that section2 doesn't exist in the new file
    value = NULL;
    err = ini_get_value(ctx2, "section2", "key3", &value);
    assert(err.error == INI_SECTION_NOT_FOUND);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_entire_section passed\n");
}

void test_update_existing_file()
{
    // Create initial file with multiple sections
    create_test_file(TEST_FILE, "[section1]\nkey1=original\nkey2=value2\n\n[section2]\nkey3=value3\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Modify the context
    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    // Get key1's value, modify it, and update the context
    char *value = NULL;
    err = ini_get_value(ctx, "section1", "key1", &value);
    assert(err.error == INI_SUCCESS);
    free(value);

    // Create a new context with modified value
    ini_context_t *modified_ctx = ini_create_context();
    assert(modified_ctx != NULL);

    create_test_file(TEST_FILE_LOAD, "[section1]\nkey1=updated\n");
    err = ini_load(modified_ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Update only key1 in the original file
    err = ini_save_section_value(modified_ctx, TEST_FILE, "section1", "key1");
    assert(err.error == INI_SUCCESS);

    // Reload the file and verify only key1 was updated
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    // Check that key1 was updated
    value = NULL;
    err = ini_get_value(ctx2, "section1", "key1", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "updated") == 0);
    free(value);

    // Check that other keys are preserved
    value = NULL;
    err = ini_get_value(ctx2, "section1", "key2", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value2") == 0);
    free(value);

    value = NULL;
    err = ini_get_value(ctx2, "section2", "key3", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value3") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(modified_ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    remove_test_file(TEST_FILE_LOAD);
    print_success("test_update_existing_file passed\n");
}

void test_subsection_save()
{
    // Create test data with a subsection
    create_test_file(TEST_FILE_LOAD, "[parent]\nkey1=value1\n\n[parent.child]\nkey2=value2\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save the subsection to a new file
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "parent.child", "key2");
    assert(err.error == INI_SUCCESS);

    // Verify the file was saved correctly with parent section too
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Check that the parent section exists
    char *value = NULL;
    err = ini_get_value(ctx2, "parent.child", "key2", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value2") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_subsection_save passed\n");
}

void test_save_with_special_chars()
{
    // Create test data with special characters
    create_test_file(TEST_FILE_LOAD, "[section]\nkey1=value with spaces\nkey2=\"quoted value\"\nkey3=value#with#hash\nkey4=value;with;semicolon\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save key with special chars
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "section", "key1");
    assert(err.error == INI_SUCCESS);

    // Verify the content
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx2, "section", "key1", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value with spaces") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_with_special_chars passed\n");
}

void test_save_empty_value()
{
    // Create test data with empty value
    create_test_file(TEST_FILE_LOAD, "[section]\nkey1=\nkey2=value2\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save key with empty value
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "section", "key1");
    assert(err.error == INI_SUCCESS);

    // Verify the content
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx2, "section", "key1", &value);
    assert(err.error == INI_SUCCESS);
    assert(strlen(value) == 0); // Empty string
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_empty_value passed\n");
}

void test_save_unicode()
{
    // Create test data with Unicode characters
    create_test_file(TEST_FILE_LOAD, "[секция]\nключ=значение\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save section with Unicode
    err = ini_save_section_value(ctx, TEST_FILE_SAVE, "секция", "ключ");
    assert(err.error == INI_SUCCESS);

    // Verify the content
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx2, "секция", "ключ", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "значение") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_unicode passed\n");
}

void test_save_to_directory()
{
    // Create a directory
    create_test_dir("test_dir");

    // Create test data
    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Try to save to a directory
    err = ini_save_section_value(ctx, "test_dir", "section", "key");
    assert(err.error == INI_FILE_OPEN_FAILED);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_dir("test_dir");
    print_success("test_save_to_directory passed\n");
}

#if INI_OS_UNIX
void test_save_no_write_permission()
{
    // Create a directory
    create_test_dir("test_dir");

    // Create a file in that directory
    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Remove write permissions from the directory
    chmod("test_dir", 0555); // r-xr-xr-x

    // Try to save to a file in that directory
    err = ini_save_section_value(ctx, "test_dir/test_no_perm.ini", "section", "key");
    assert(err.error == INI_FILE_OPEN_FAILED);

    // Restore permissions and clean up
    chmod("test_dir", 0777); // rwxrwxrwx
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_dir("test_dir");
    print_success("test_save_no_write_permission passed\n");
}
#endif

#if INI_OS_UNIX || INI_OS_WINDOWS
void *__thread_save_section(void *arg)
{
    ini_context_t *ctx = (ini_context_t *)arg;
    char filename[64];
#if INI_OS_UNIX
    sprintf(filename, "thread_save_section_%ld.ini", (long)pthread_self());
#elif INI_OS_WINDOWS
    sprintf(filename, "thread_save_section_%lu.ini", (unsigned long)GetCurrentThreadId());
#endif

    ini_error_details_t err = ini_save_section_value(ctx, filename, "section", "key");
    assert(err.error == INI_SUCCESS);
    remove_test_file(filename);
    return NULL;
}

void test_thread_safety()
{
    // Create test data
    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save from multiple threads
#if INI_OS_UNIX
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
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    print_success("test_thread_safety passed\n");
}
#endif

void test_add_section_to_existing_file()
{
    // Create initial file with one section
    create_test_file(TEST_FILE, "[section1]\nkey1=value1\n");

    // Create a context with a new section
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    create_test_file(TEST_FILE_LOAD, "[section2]\nkey2=value2\n");
    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save the new section to the existing file
    err = ini_save_section_value(ctx, TEST_FILE, "section2", NULL);
    assert(err.error == INI_SUCCESS);

    // Reload the file and verify both sections exist
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    // Check section1 is preserved
    char *value = NULL;
    err = ini_get_value(ctx2, "section1", "key1", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value1") == 0);
    free(value);

    // Check section2 was added
    value = NULL;
    err = ini_get_value(ctx2, "section2", "key2", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value2") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    remove_test_file(TEST_FILE_LOAD);
    print_success("test_add_section_to_existing_file passed\n");
}

int main()
{
    __helper_init_log_file();
    ini_initialize();

    test_null_context();
    test_null_filepath();
    test_null_section();
    test_section_not_found();
    test_key_not_found();
    test_save_section_key();
    test_save_entire_section();
    test_update_existing_file();
    test_subsection_save();
    test_save_with_special_chars();
    test_save_empty_value();
    test_save_unicode();
    test_save_to_directory();
    test_add_section_to_existing_file();

#if INI_OS_UNIX
    test_save_no_write_permission();
#endif

#if INI_OS_UNIX || INI_OS_WINDOWS
    test_thread_safety();
#endif

    ini_finalize();
    print_success("All ini_save_section_value() tests passed!\n\n");
    __helper_close_log_file();
    return ini_has_error();
}
