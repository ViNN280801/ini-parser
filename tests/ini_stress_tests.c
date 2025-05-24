#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ini_os_check.h"

#if INI_OS_LINUX
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#endif

#if INI_OS_WINDOWS
#include <windows.h>
#elif INI_OS_LINUX || INI_OS_APPLE
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "helper.h"
#include "ini_parser.h"

#define TEST_FILE "test_stress.ini"
#define LARGE_FILE "test_large.ini"
#define NUM_THREADS 10
#define NUM_SECTIONS 100
#define NUM_KEYS_PER_SECTION 50
#define LARGE_KEY_SIZE 2048

// Generate a large INI file with many sections and keys
void generate_large_ini_file(const char *filename, int num_sections, int keys_per_section)
{
    FILE *fp = fopen(filename, "w");
    if (!fp)
    {
        return;
    }

    for (int i = 0; i < num_sections; i++)
    {
        fprintf(fp, "[section%d]\n", i);
        for (int j = 0; j < keys_per_section; j++)
        {
            fprintf(fp, "key%d=value%d_%d\n", j, i, j);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

// Test loading a large INI file
void test_large_file_load()
{
    clock_t start, end;
    double cpu_time_used;

    // Generate a large INI file
    generate_large_ini_file(LARGE_FILE, NUM_SECTIONS, NUM_KEYS_PER_SECTION);

    // Load the file and measure time
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    start = clock();
    ini_status_t err = ini_load(ctx, LARGE_FILE);
    end = clock();

    assert(err == INI_STATUS_SUCCESS);

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Verify random access to values
    for (int i = 0; i < 10; i++)
    {
        int section_idx = rand() % NUM_SECTIONS;
        int key_idx = rand() % NUM_KEYS_PER_SECTION;

        char section[32];
        char key[32];
        char expected[64];

        sprintf(section, "section%d", section_idx);
        sprintf(key, "key%d", key_idx);
        sprintf(expected, "value%d_%d", section_idx, key_idx);

        char *value = NULL;
        err = ini_get_value(ctx, section, key, &value);
        assert(err == INI_STATUS_SUCCESS);
        assert(value != NULL);
        assert(strcmp(value, expected) == 0);
        free(value);
    }

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(LARGE_FILE);

    print_success("test_large_file_load passed\n");
}

// Test saving a large INI file
void test_large_file_save()
{
    clock_t start, end;
    double cpu_time_used;

    // Create a context with many sections and keys
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Generate a large INI file and load it
    generate_large_ini_file(LARGE_FILE, NUM_SECTIONS, NUM_KEYS_PER_SECTION);
    ini_status_t err = ini_load(ctx, LARGE_FILE);
    assert(err == INI_STATUS_SUCCESS);

    // Save the file and measure time
    start = clock();
    err = ini_save(ctx, TEST_FILE);
    end = clock();

    assert(err == INI_STATUS_SUCCESS);

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Verify the saved file
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    // Verify random access to values
    for (int i = 0; i < 10; i++)
    {
        int section_idx = rand() % NUM_SECTIONS;
        int key_idx = rand() % NUM_KEYS_PER_SECTION;

        char section[32];
        char key[32];
        char expected[64];

        sprintf(section, "section%d", section_idx);
        sprintf(key, "key%d", key_idx);
        sprintf(expected, "value%d_%d", section_idx, key_idx);

        char *value = NULL;
        err = ini_get_value(ctx2, section, key, &value);
        assert(err == INI_STATUS_SUCCESS);
        assert(value != NULL);
        assert(strcmp(value, expected) == 0);
        free(value);
    }

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(LARGE_FILE);
    remove_test_file(TEST_FILE);

    print_success("test_large_file_save passed\n");
}

// Test with very large keys and values
void test_large_keys_values()
{
    // Create a large key and value
    char *large_key = (char *)malloc(LARGE_KEY_SIZE + 1);
    char *large_value = (char *)malloc(LARGE_KEY_SIZE + 1);

    assert(large_key != NULL);
    assert(large_value != NULL);

    // Fill with a repeating pattern
    for (int i = 0; i < LARGE_KEY_SIZE; i++)
    {
        large_key[i] = 'a' + (i % 26);
        large_value[i] = 'A' + (i % 26);
    }
    large_key[LARGE_KEY_SIZE] = '\0';
    large_value[LARGE_KEY_SIZE] = '\0';

    // Create an INI file with the large key/value
    FILE *fp = fopen(TEST_FILE, "w");
    assert(fp != NULL);
    fprintf(fp, "[section]\n%s=%s\n", large_key, large_value);
    fclose(fp);

    // Load the file
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_STATUS_SUCCESS);

    // Get the value and verify
    char *value = NULL;
    err = ini_get_value(ctx, "section", large_key, &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, large_value) == 0);
    free(value);

    // Save to a new file
    err = ini_save(ctx, LARGE_FILE);
    assert(err == INI_STATUS_SUCCESS);

    // Load the saved file and verify
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, LARGE_FILE);
    assert(err == INI_STATUS_SUCCESS);

    value = NULL;
    err = ini_get_value(ctx2, "section", large_key, &value);
    assert(err == INI_STATUS_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, large_value) == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_STATUS_SUCCESS);

    free(large_key);
    free(large_value);
    remove_test_file(TEST_FILE);
    remove_test_file(LARGE_FILE);

    print_success("test_large_keys_values passed\n");
}

#if INI_OS_LINUX || INI_OS_WINDOWS
// Thread function for concurrent reading
void *thread_read_ini(void *arg)
{
    ini_context_t *ctx = (ini_context_t *)arg;

    for (int i = 0; i < 100; i++)
    {
        int section_idx = rand() % NUM_SECTIONS;
        int key_idx = rand() % NUM_KEYS_PER_SECTION;

        char section[32];
        char key[32];
        char expected[64];

        sprintf(section, "section%d", section_idx);
        sprintf(key, "key%d", key_idx);
        sprintf(expected, "value%d_%d", section_idx, key_idx);

        char *value = NULL;
        ini_status_t err = ini_get_value(ctx, section, key, &value);
        assert(err == INI_STATUS_SUCCESS);
        assert(value != NULL);
        assert(strcmp(value, expected) == 0);
        free(value);
    }

    return NULL;
}

// Thread function for concurrent writes to different files
void *thread_write_ini(void *arg)
{
    int thread_id = *((int *)arg);
    char filename[64];
    sprintf(filename, "thread_stress_%d.ini", thread_id);

    // Create a context with a few sections
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Add sections and keys
    char content[2048] = {0};
    char section[32];
    char buffer[128];

    for (int i = 0; i < 5; i++)
    {
        sprintf(section, "[thread%d_section%d]\n", thread_id, i);
        strcat(content, section);

        for (int j = 0; j < 10; j++)
        {
            sprintf(buffer, "key%d=thread%d_value%d_%d\n", j, thread_id, i, j);
            strcat(content, buffer);
        }
        strcat(content, "\n");
    }

    create_test_file(filename, content);

    // Load, modify and save repeatedly
    ini_status_t err = ini_load(ctx, filename);
    assert(err == INI_STATUS_SUCCESS);

    for (int i = 0; i < 10; i++)
    {
        // Modify a random value
        int section_idx = rand() % 5;
        int key_idx = rand() % 10;

        sprintf(section, "thread%d_section%d", thread_id, section_idx);
        sprintf(buffer, "key%d", key_idx);

        char new_value[64];
        sprintf(new_value, "thread%d_modified%d", thread_id, i);

        // Create a temporary context with the new value
        ini_context_t *temp_ctx = ini_create_context();
        assert(temp_ctx != NULL);

        char temp_content[256];
        snprintf(temp_content, sizeof(temp_content), "[%s]\n%s=%s\n", section, buffer, new_value);

        char temp_file[64];
        sprintf(temp_file, "thread_temp_%d.ini", thread_id);
        create_test_file(temp_file, temp_content);

        err = ini_load(temp_ctx, temp_file);
        assert(err == INI_STATUS_SUCCESS);

        // Save the modified value
        err = ini_save_section_value(temp_ctx, filename, section, buffer);
        assert(err == INI_STATUS_SUCCESS);

        // Verify the change
        ini_context_t *verify_ctx = ini_create_context();
        assert(verify_ctx != NULL);

        err = ini_load(verify_ctx, filename);
        assert(err == INI_STATUS_SUCCESS);

        char *value = NULL;
        err = ini_get_value(verify_ctx, section, buffer, &value);
        assert(err == INI_STATUS_SUCCESS);
        assert(value != NULL);
        assert(strcmp(value, new_value) == 0);
        free(value);

        err = ini_free(temp_ctx);
        assert(err == INI_STATUS_SUCCESS);
        err = ini_free(verify_ctx);
        assert(err == INI_STATUS_SUCCESS);
        remove_test_file(temp_file);
    }

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(filename);

    return NULL;
}

// Test concurrent reading from the same context
// void test_concurrent_reads()
// {
//     // Create a large INI file
//     generate_large_ini_file(LARGE_FILE, NUM_SECTIONS, NUM_KEYS_PER_SECTION);

//     // Load the file
//     ini_context_t *ctx = ini_create_context();
//     assert(ctx != NULL);

//     ini_status_t err = ini_load(ctx, LARGE_FILE);
//     assert(err == INI_STATUS_SUCCESS);

//     // Spawn multiple threads to read concurrently
// #if INI_OS_LINUX
//     pthread_t threads[NUM_THREADS];
//     for (int i = 0; i < NUM_THREADS; i++)
//     {
//         pthread_create(&threads[i], NULL, thread_read_ini, ctx);
//     }

//     // Wait for all threads to complete
//     for (int i = 0; i < NUM_THREADS; i++)
//     {
//         pthread_join(threads[i], NULL);
//     }
// #elif INI_OS_WINDOWS
//     HANDLE threads[NUM_THREADS];
//     for (int i = 0; i < NUM_THREADS; i++)
//     {
//         threads[i] = CreateThread(NULL, 0,
//                                   (LPTHREAD_START_ROUTINE)thread_read_ini,
//                                   ctx, 0, NULL);
//         assert(threads[i] != NULL);
//     }

//     WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

//     for (int i = 0; i < NUM_THREADS; i++)
//     {
//         CloseHandle(threads[i]);
//     }
// #endif

//     err = ini_free(ctx);
//     assert(err == INI_STATUS_SUCCESS);
//     remove_test_file(LARGE_FILE);

//     print_success("test_concurrent_reads passed\n");
// }
#endif

// Test memory usage with very large files
void test_memory_usage()
{
    // Generate an extremely large INI file
    generate_large_ini_file(LARGE_FILE, NUM_SECTIONS * 2, NUM_KEYS_PER_SECTION * 2);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_status_t err = ini_load(ctx, LARGE_FILE);
    assert(err == INI_STATUS_SUCCESS);

    // Perform many random accesses
    for (int i = 0; i < 1000; i++)
    {
        int section_idx = rand() % (NUM_SECTIONS * 2);
        int key_idx = rand() % (NUM_KEYS_PER_SECTION * 2);

        char section[32];
        char key[32];

        sprintf(section, "section%d", section_idx);
        sprintf(key, "key%d", key_idx);

        char *value = NULL;
        err = ini_get_value(ctx, section, key, &value);
        if (err == INI_STATUS_SUCCESS)
        {
            free(value);
        }
    }

    err = ini_free(ctx);
    assert(err == INI_STATUS_SUCCESS);
    remove_test_file(LARGE_FILE);

    print_success("test_memory_usage passed\n");
}

// Test handling corrupt or invalid files
void test_corrupt_files()
{
    // Test 1: Truncated file
    FILE *fp = fopen(TEST_FILE, "w");
    assert(fp != NULL);
    fprintf(fp, "[section1]\nkey1=value1\nkey2=val"); // Truncated
    fclose(fp);

    ini_status_t err = ini_good(TEST_FILE);
    // This might return either success or bad format depending on implementation
    // Just make sure it doesn't crash

    // Test 2: File with very long lines
    fp = fopen(TEST_FILE, "w");
    assert(fp != NULL);
    fprintf(fp, "[section1]\nkey1=");

    // Write a very long value
    for (int i = 0; i < 10000; i++)
    {
        fprintf(fp, "a");
    }
    fprintf(fp, "\n");
    fclose(fp);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    err = ini_load(ctx, TEST_FILE);
    // This should either succeed or fail gracefully

    if (err == INI_STATUS_SUCCESS)
    {
        // Clean up if it succeeded
        err = ini_free(ctx);
        assert(err == INI_STATUS_SUCCESS);
    }
    else
    {
        // Just make sure it failed in a controlled way
        err = ini_free(ctx);
        assert(err == INI_STATUS_SUCCESS);
    }

    // Test 3: File with binary data
    fp = fopen(TEST_FILE, "wb");
    assert(fp != NULL);

    // Write some random binary data
    for (int i = 0; i < 1000; i++)
    {
        fprintf(fp, "%c", rand() % 256);
    }
    fclose(fp);

    err = ini_good(TEST_FILE);
    // This should fail, but not crash
    if (err != INI_STATUS_SUCCESS)
    {
    }

    remove_test_file(TEST_FILE);

    print_success("test_corrupt_files passed\n");
}

// Main stress test function
int main(int argc, char *argv[])
{
    __helper_init_log_file();

    // Seed random number generator
    srand((unsigned int)time(NULL));

    // Run stress tests
    test_large_file_load();
    test_large_file_save();
    test_large_keys_values();

    // #if INI_OS_WINDOWS || INI_OS_LINUX
    //     test_concurrent_reads();
    // #endif

    test_memory_usage();
    test_corrupt_files();

    print_success("All stress tests passed!\n\n");
    __helper_close_log_file();
    return EXIT_SUCCESS;
}
