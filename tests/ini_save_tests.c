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

#define TEST_FILE "test_save.ini"
#define TEST_FILE_LOAD "test_load.ini"
#define TEST_FILE_SAVE "test_save_output.ini"

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

    // For debugging
    printf("Comparing files %s and %s\n", file1, file2);

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
            printf("Line %d: One file ended before the other\n", line_num);
            match = 0;
            break;
        }

        // Print lines for debugging
        printf("Line %d: \nFile1: '%s'\nFile2: '%s'\n", line_num, line1, line2);

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
                printf("Line %d: Section headers don't match: '%s' vs '%s'\n",
                       line_num, line1, line2);
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
            printf("Line %d: Missing equals sign in one of the lines\n", line_num);
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
            printf("Line %d: Keys don't match: '%s' vs '%s'\n", line_num, key1, key2);
            match = 0;
            break;
        }

        if (strcmp(val1, val2) != 0)
        {
            printf("Line %d: Values don't match: '%s' vs '%s'\n", line_num, val1, val2);
            match = 0;
            break;
        }
    }

    fclose(f1);
    fclose(f2);
    return match;
}

void test_null_context()
{
    ini_error_details_t err = ini_save(NULL, TEST_FILE);
    assert(err.error == INI_INVALID_ARGUMENT);
    print_success("test_null_context passed\n");
}

void test_null_filepath()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_save(ctx, NULL);
    assert(err.error == INI_INVALID_ARGUMENT);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_null_filepath passed\n");
}

void test_save_empty_context()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_save(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    // Check if the file exists and is empty (no sections)
    FILE *file = fopen(TEST_FILE, "r");
    assert(file != NULL);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    assert(size == 0); // File should be empty

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_save_empty_context passed\n");
}

void test_save_simple_context()
{
    // Create test data
    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save to a new file
    err = ini_save(ctx, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Check if the files have the same content
    assert(compare_files(TEST_FILE_LOAD, TEST_FILE_SAVE) == 1);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_simple_context passed\n");
}

void test_save_with_subsections()
{
    // Create test data
    create_test_file(TEST_FILE_LOAD, "[parent]\nkey1=value1\n\n[parent.child]\nkey2=value2\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save to a new file
    err = ini_save(ctx, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Check if the saved file can be loaded correctly
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Verify the content by getting values
    char *value1 = NULL;
    err = ini_get_value(ctx2, "parent", "key1", &value1);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value1, "value1") == 0);
    free(value1);

    char *value2 = NULL;
    err = ini_get_value(ctx2, "parent.child", "key2", &value2);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value2, "value2") == 0);
    free(value2);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_with_subsections passed\n");
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

    // Save to a new file
    err = ini_save(ctx, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Check if the saved file can be loaded correctly
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Verify the content by getting values
    char *value1 = NULL;
    err = ini_get_value(ctx2, "section", "key1", &value1);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value1, "value with spaces") == 0);
    free(value1);

    char *value2 = NULL;
    err = ini_get_value(ctx2, "section", "key2", &value2);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value2, "quoted value") == 0);
    free(value2);

    char *value3 = NULL;
    err = ini_get_value(ctx2, "section", "key3", &value3);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value3, "value#with#hash") == 0);
    free(value3);

    char *value4 = NULL;
    err = ini_get_value(ctx2, "section", "key4", &value4);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value4, "value;with;semicolon") == 0);
    free(value4);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_with_special_chars passed\n");
}

void test_save_empty_values()
{
    // Create test data with empty values
    create_test_file(TEST_FILE_LOAD, "[section]\nkey1=\nkey2=\"\"\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save to a new file
    err = ini_save(ctx, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Check if the saved file can be loaded correctly
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Verify the content by getting values
    char *value1 = NULL;
    err = ini_get_value(ctx2, "section", "key1", &value1);
    assert(err.error == INI_SUCCESS);
    assert(strlen(value1) == 0); // Empty string
    free(value1);

    char *value2 = NULL;
    err = ini_get_value(ctx2, "section", "key2", &value2);
    assert(err.error == INI_SUCCESS);
    assert(strlen(value2) == 0); // Empty string
    free(value2);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_empty_values passed\n");
}

void test_save_unicode()
{
    // Create test data with Unicode characters
    create_test_file(TEST_FILE_LOAD, "[секция]\nключ=значение\n[节]\n键=值\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save to a new file
    err = ini_save(ctx, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Check if the saved file can be loaded correctly
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Verify the content by getting values
    char *value1 = NULL;
    err = ini_get_value(ctx2, "секция", "ключ", &value1);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value1, "значение") == 0);
    free(value1);

    char *value2 = NULL;
    err = ini_get_value(ctx2, "节", "键", &value2);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value2, "值") == 0);
    free(value2);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_unicode passed\n");
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
    err = ini_save(ctx, "test_dir/test_no_perm.ini");
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
void *__thread_save(void *arg)
{
    ini_context_t *ctx = (ini_context_t *)arg;
    char filename[64];
#if INI_OS_UNIX
    sprintf(filename, "thread_save_%ld.ini", (long)pthread_self());
#elif INI_OS_WINDOWS
    sprintf(filename, "thread_save_%lu.ini", (unsigned long)GetCurrentThreadId());
#endif

    ini_error_details_t err = ini_save(ctx, filename);
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
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    print_success("test_thread_safety passed\n");
}
#endif

void test_save_to_existing_file()
{
    // Create a file with some content
    create_test_file(TEST_FILE_SAVE, "This is some existing content\n");

    // Create test data
    create_test_file(TEST_FILE_LOAD, "[section]\nkey=value\n");

    // Load the INI file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE_LOAD);
    assert(err.error == INI_SUCCESS);

    // Save to the existing file
    err = ini_save(ctx, TEST_FILE_SAVE);
    assert(err.error == INI_SUCCESS);

    // Check if the content was replaced
    FILE *file = fopen(TEST_FILE_SAVE, "r");
    assert(file != NULL);

    char buffer[256];
    fgets(buffer, sizeof(buffer), file);
    assert(strcmp(buffer, "[section]\n") == 0);
    fclose(file);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_file(TEST_FILE_SAVE);
    print_success("test_save_to_existing_file passed\n");
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
    err = ini_save(ctx, "test_dir");
    assert(err.error == INI_FILE_OPEN_FAILED);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE_LOAD);
    remove_test_dir("test_dir");
    print_success("test_save_to_directory passed\n");
}

int main()
{
    __helper_init_log_file();
    ini_initialize();

    test_null_context();
    test_null_filepath();
    test_save_empty_context();
    test_save_simple_context();
    test_save_with_subsections();
    test_save_with_special_chars();
    test_save_empty_values();
    test_save_unicode();
    test_save_to_existing_file();
    test_save_to_directory();

#if INI_OS_UNIX
    test_save_no_write_permission();
#endif

#if INI_OS_UNIX || INI_OS_WINDOWS
    test_thread_safety();
#endif

    ini_finalize();
    print_success("All ini_save() tests passed!\n\n");
    __helper_close_log_file();
    return ini_has_error();
}
