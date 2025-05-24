#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ini_os_check.h"

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

#define TEST_FILE "test_integration.ini"
#define OUTPUT_FILE "test_integration_output.ini"

// ==================== Test Cases ====================

// --- Test 1: Basic Load and Save ---
void test_basic_load_save()
{
    // Clean test: Normal usage
    create_test_file(TEST_FILE, "[section]\nkey=value\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_save(ctx, OUTPUT_FILE);
    assert(err == INI_STATUS_SUCCESS);

    // Verify the saved file
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, OUTPUT_FILE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx2, "section", "key", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value, "value") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    remove_test_file(OUTPUT_FILE);

    print_success("test_basic_load_save passed\n");
}

// --- Test 2: Load Nonexistent File ---
void test_load_nonexistent_file()
{
    // Dirty test: File doesn't exist
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, "nonexistent.ini");
    assert(err == INI_STATUS_FILE_NOT_FOUND);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    print_success("test_load_nonexistent_file passed\n");
}

// --- Test 3: Save to Read-Only Directory ---
void test_save_readonly_dir()
{
    // Dirty test: No write permissions
#if INI_OS_LINUX
    create_test_dir("readonly_dir");
    chmod("readonly_dir", 0555); // r-xr-xr-x

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    chmod("readonly_dir", 0777); // Restore permissions
    remove_test_dir("readonly_dir");
    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_save_readonly_dir passed\n");
#endif
}

// --- Test 4: Corrupt INI File ---
void test_corrupt_ini_file()
{
    // Dirty test: Malformed INI
    create_test_file(TEST_FILE, "[section\nkey=value\n"); // Missing closing bracket

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_corrupt_ini_file passed\n");
}

// --- Test 5: Save Section with Subsection ---
void test_save_subsection()
{
    // Clean test: Subsections
    create_test_file(TEST_FILE, "[parent]\nkey1=value1\n[parent.child]\nkey2=value2\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    err = ini_save_section_value(ctx, OUTPUT_FILE, "parent.child", "key2");
    assert(err == INI_STATUS_SUCCESS);

    // Verify the saved file
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, OUTPUT_FILE);
    assert(err == INI_STATUS_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx2, "parent.child", "key2", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value, "value2") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    remove_test_file(OUTPUT_FILE);
    print_success("test_save_subsection passed\n");
}

// --- Test 6: Thread Safety ---
#if INI_OS_LINUX || INI_OS_WINDOWS
void *thread_read(void *arg)
{
    ini_context_t *ctx = (ini_context_t *)arg;
    char *value = NULL;
    ini_status_t err = ini_get_value(ctx, "section", "key", &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(strcmp(value, "value") == 0);
    free(value);
    return NULL;
}

void test_thread_safety()
{
    // Dirty test: Concurrent access
    create_test_file(TEST_FILE, "[section]\nkey=value\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

#if INI_OS_LINUX
    pthread_t threads[5];
    for (int i = 0; i < 5; i++)
    {
        pthread_create(&threads[i], NULL, thread_read, ctx);
    }
    for (int i = 0; i < 5; i++)
    {
        pthread_join(threads[i], NULL);
    }
#elif INI_OS_WINDOWS
    HANDLE threads[5];
    for (int i = 0; i < 5; i++)
    {
        threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_read, ctx, 0, NULL);
        assert(threads[i] != NULL);
    }
    WaitForMultipleObjects(5, threads, TRUE, INFINITE);
    for (int i = 0; i < 5; i++)
    {
        CloseHandle(threads[i]);
    }
#endif

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_thread_safety passed\n");
}
#endif

// ==================== Main ====================
int main()
{
    __helper_init_log_file();

    test_basic_load_save();
    test_load_nonexistent_file();
    test_save_readonly_dir();
    test_corrupt_ini_file();
    test_save_subsection();
#if INI_OS_LINUX || INI_OS_WINDOWS
    test_thread_safety();
#endif

    print_success("All integration tests passed!\n\n");
    __helper_close_log_file();
    return EXIT_SUCCESS;
}
