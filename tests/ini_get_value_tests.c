#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if INI_OS_UNIX
#include <pthread.h>
#include <unistd.h>
#elif INI_OS_WINDOWS
#include <windows.h>
#endif

#include "helper.h"
#include "iniparser.h"

#define TEST_FILE "test.ini"

void test_empty_value()
{
    fprintf(stderr, "STARTING TEST: test_empty_value\n");
    create_test_file(TEST_FILE, "[section]\nkey=\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_print(stderr, ctx);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err.error == INI_SUCCESS);
    assert(value == NULL);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_empty_value passed\n");
}

void test_get_existing_value()
{
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_context_t *ctx = NULL;
    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);
    err = ini_print(stderr, ctx);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err.error == INI_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_get_existing_value passed\n");
}

void test_key_not_found()
{
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "nonexistent", &value);
    assert(err.error == INI_KEY_NOT_FOUND);
    assert(value == NULL);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_key_not_found passed\n");
}

void test_section_not_found()
{
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "nonexistent", "key", &value);
    assert(err.error == INI_SECTION_NOT_FOUND);
    assert(value == NULL);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_section_not_found passed\n");
}

void test_subsection()
{
    create_test_file(TEST_FILE, "[parent]\nkey=value\n[parent.child]\nkey=child_value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "parent.child", "key", &value);
    assert(err.error == INI_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "child_value") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_subsection passed\n");
}

void test_whitespace_value()
{
    create_test_file(TEST_FILE, "[section]\nkey=   value with spaces   \n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err.error == INI_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value with spaces") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_whitespace_value passed\n");
}

void test_unicode()
{
    create_test_file(TEST_FILE, "[секция]\nключ=значение\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "секция", "ключ", &value);
    assert(err.error == INI_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "значение") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_unicode passed\n");
}

void test_utf8_bom()
{
    FILE *f = fopen(TEST_FILE, "wb");
    const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    fwrite(bom, 1, 3, f);
    fputs("[section]\nkey=значение\n", f);
    fclose(f);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err.error == INI_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "значение") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_utf8_bom passed\n");
}

void test_null_arguments()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    char *value = NULL;
    ini_error_details_t err = ini_get_value(NULL, "section", "key", &value);
    assert(err.error == INI_INVALID_ARGUMENT);

    err = ini_get_value(ctx, NULL, "key", &value);
    assert(err.error == INI_INVALID_ARGUMENT);

    err = ini_get_value(ctx, "section", NULL, &value);
    assert(err.error == INI_INVALID_ARGUMENT);

    err = ini_get_value(ctx, "section", "key", NULL);
    assert(err.error == INI_INVALID_ARGUMENT);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_null_arguments passed\n");
}

void test_reuse_value_pointer()
{
    create_test_file(TEST_FILE, "[section]\nkey1=value1\nkey2=value2\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key1", &value);
    assert(err.error == INI_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value1") == 0);

    err = ini_get_value(ctx, "section", "key2", &value);
    assert(err.error == INI_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value2") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_reuse_value_pointer passed\n");
}

#if INI_OS_UNIX || INI_OS_WINDOWS
void *__thread_get_value(void *arg)
{
    ini_context_t *ctx = (ini_context_t *)arg;
    char *value = NULL;
    ini_error_details_t err = ini_get_value(ctx, "section", "key", &value);
    assert(err.error == INI_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value") == 0);
    free(value);
    return NULL;
}

void test_thread_safety()
{
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

#if INI_OS_UNIX
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
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_thread_safety passed\n");
}
#endif

void test_reuse_context()
{
    create_test_file("test1.ini", "[section1]\nkey1=value1\n");
    create_test_file("test2.ini", "[section2]\nkey2=value2\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, "test1.ini");
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section1", "key1", &value);
    assert(err.error == INI_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value1") == 0);
    free(value);
    value = NULL;

    err = ini_load(ctx, "test2.ini");
    assert(err.error == INI_SUCCESS);

    err = ini_get_value(ctx, "section2", "key2", &value);
    assert(err.error == INI_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value2") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file("test1.ini");
    remove_test_file("test2.ini");
    print_success("test_reuse_context passed\n");
}

void test_corrupted_file()
{
    create_test_file(TEST_FILE, "[section\nkey=value\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_corrupted_file passed\n");
}

void test_quoted_values()
{
    create_test_file(TEST_FILE, "[section]\nkey=\"quoted value\"\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err.error == INI_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "quoted value") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_quoted_values passed\n");
}

void test_existing_value_memory()
{
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = strdup("existing memory");
    assert(value != NULL);

    err = ini_get_value(ctx, "section", "key", &value);
    assert(err.error == INI_SUCCESS);
    assert(value != NULL);
    assert(strcmp(value, "value") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_existing_value_memory passed\n");
}

int main()
{
    __helper_init_log_file();
    ini_initialize();

    test_empty_value();
    test_get_existing_value();
    test_key_not_found();
    test_section_not_found();
    test_subsection();
    test_whitespace_value();
    test_unicode();
    test_utf8_bom();
    test_quoted_values();
    test_null_arguments();
    test_reuse_value_pointer();
    test_existing_value_memory();

#if INI_OS_UNIX || INI_OS_WINDOWS
    test_thread_safety();
#endif

    test_reuse_context();
    test_corrupted_file();

    ini_finalize();
    print_success("All ini_get_value() tests passed!\n\n");
    __helper_close_log_file();
    return ini_has_error();
}
