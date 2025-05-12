#include <assert.h>
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

#if INI_OS_WINDOWS && !defined(PATH_MAX)
#define PATH_MAX 260
#endif

#include "helper.h"
#include "iniparser.h"

#define TEST_FILE "test_print.ini"
#define OUTPUT_FILE "test_print_output.txt"

// Helper function to capture printed output
static char *capture_output(void (*test_func)(FILE *))
{
#if INI_OS_WINDOWS
#define TEMP_FILENAME_LEN 256
    char temp_filename[TEMP_FILENAME_LEN];
    snprintf(temp_filename, TEMP_FILENAME_LEN, "%s\\print_test_%lld.txt",
             getenv("TEMP"), (long long)GetCurrentProcessId());
#else
    char temp_filename[PATH_MAX];
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
        fprintf(stderr, "Failed to open temp file: %s\n", temp_filename);
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
static void test_print_basic_section_func(FILE *output)
{
    ini_context_t *ctx = ini_create_context();
    if (ctx == NULL)
    {
        fprintf(output, "Failed to create context\n");
        return;
    }

    // Initialize section
    ctx->sections = (ini_section_t *)malloc(sizeof(ini_section_t));
    if (ctx->sections == NULL)
    {
        ini_free(ctx);
        fprintf(output, "Failed to allocate sections\n");
        return;
    }

    ctx->section_count = 1;
    ctx->sections[0].name = strdup("section1");
    ctx->sections[0].pair_count = 2;
    ctx->sections[0].pairs = (ini_key_value_t *)malloc(2 * sizeof(ini_key_value_t));
    if (ctx->sections[0].pairs == NULL)
    {
        if (ctx->sections[0].name)
            free(ctx->sections[0].name);

        if (ctx->sections)
            free(ctx->sections);

        ini_free(ctx);
        fprintf(output, "Failed to allocate pairs\n");
        return;
    }

    // Initialize key-value pairs
    ctx->sections[0].pairs[0].key = strdup("key1");
    ctx->sections[0].pairs[0].value = strdup("value1");
    ctx->sections[0].pairs[1].key = strdup("key2");
    ctx->sections[0].pairs[1].value = strdup("value2");

    if (ctx->sections[0].pairs[0].key == NULL ||
        ctx->sections[0].pairs[0].value == NULL ||
        ctx->sections[0].pairs[1].key == NULL ||
        ctx->sections[0].pairs[1].value == NULL)
    {
        // Cleanup partial allocations
        if (ctx->sections[0].pairs[0].key)
            free(ctx->sections[0].pairs[0].key);
        if (ctx->sections[0].pairs[0].value)
            free(ctx->sections[0].pairs[0].value);
        if (ctx->sections[0].pairs[1].key)
            free(ctx->sections[0].pairs[1].key);
        if (ctx->sections[0].pairs[1].value)
            free(ctx->sections[0].pairs[1].value);
        if (ctx->sections[0].pairs)
            free(ctx->sections[0].pairs);
        if (ctx->sections[0].name)
            free(ctx->sections[0].name);
        if (ctx->sections)
            free(ctx->sections);
        ini_free(ctx);
        fprintf(output, "Failed to duplicate strings\n");
        return;
    }

    ini_error_details_t err = ini_print(output, ctx);
    if (err.error != INI_SUCCESS)
    {
        fprintf(output, "Print failed with error: %d\n", err.error);
    }

    // Cleanup
    if (ctx->sections[0].pairs[0].key)
        free(ctx->sections[0].pairs[0].key);
    if (ctx->sections[0].pairs[0].value)
        free(ctx->sections[0].pairs[0].value);
    if (ctx->sections[0].pairs[1].key)
        free(ctx->sections[0].pairs[1].key);
    if (ctx->sections[0].pairs[1].value)
        free(ctx->sections[0].pairs[1].value);
    if (ctx->sections[0].pairs)
        free(ctx->sections[0].pairs);
    if (ctx->sections[0].name)
        free(ctx->sections[0].name);
    if (ctx->sections)
        free(ctx->sections);
    ini_free(ctx);
}

void test_print_basic_section()
{
    char *content = capture_output(test_print_basic_section_func);

    // Verify output format
    assert(strstr(content, "[section1]") != NULL);
    assert(strstr(content, "key1 = value1") != NULL);
    assert(strstr(content, "key2 = value2") != NULL);

    free(content);
    print_success("test_print_basic_section passed\n");
}

// Test function for printing with subsections
static void test_print_with_subsections_func(FILE *output)
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Add parent section
    ctx->sections = (ini_section_t *)malloc(sizeof(ini_section_t));
    assert(ctx->sections != NULL);
    ctx->section_count = 1;

    ctx->sections[0].name = strdup("parent");
    ctx->sections[0].pair_count = 1;
    ctx->sections[0].pairs = (ini_key_value_t *)malloc(sizeof(ini_key_value_t));
    assert(ctx->sections[0].pairs != NULL);
    ctx->sections[0].pairs[0].key = strdup("key1");
    ctx->sections[0].pairs[0].value = strdup("value1");

    // Add subsection
    ctx->sections[0].subsection_count = 1;
    ctx->sections[0].subsections = (ini_section_t *)malloc(sizeof(ini_section_t));
    assert(ctx->sections[0].subsections != NULL);

    ctx->sections[0].subsections[0].name = strdup("child");
    ctx->sections[0].subsections[0].pair_count = 1;
    ctx->sections[0].subsections[0].pairs = (ini_key_value_t *)malloc(sizeof(ini_key_value_t));
    assert(ctx->sections[0].subsections[0].pairs != NULL);
    ctx->sections[0].subsections[0].pairs[0].key = strdup("key2");
    ctx->sections[0].subsections[0].pairs[0].value = strdup("value2");

    ini_error_details_t err = ini_print(output, ctx);
    assert(err.error == INI_SUCCESS);

    ini_free(ctx);
}

void test_print_with_subsections()
{
    char *content = capture_output(test_print_with_subsections_func);

    // Verify output format
    assert(strstr(content, "[parent]") != NULL);
    assert(strstr(content, "key1 = value1") != NULL);
    assert(strstr(content, "[parent.child]") != NULL);
    assert(strstr(content, "key2 = value2") != NULL);

    free(content);
    print_success("test_print_with_subsections passed\n");
}

// Test function for printing Unicode characters
static void test_print_unicode_func(FILE *output)
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Add section with Unicode chars
    ctx->sections = (ini_section_t *)malloc(sizeof(ini_section_t));
    assert(ctx->sections != NULL);
    ctx->section_count = 1;

    ctx->sections[0].name = strdup("секция");
    ctx->sections[0].pair_count = 1;
    ctx->sections[0].pairs = (ini_key_value_t *)malloc(sizeof(ini_key_value_t));
    assert(ctx->sections[0].pairs != NULL);
    ctx->sections[0].pairs[0].key = strdup("ключ");
    ctx->sections[0].pairs[0].value = strdup("значение");

    ini_error_details_t err = ini_print(output, ctx);
    assert(err.error == INI_SUCCESS);

    ini_free(ctx);
}

void test_print_unicode()
{
    char *content = capture_output(test_print_unicode_func);

    // Verify Unicode characters are printed correctly
    assert(strstr(content, "[секция]") != NULL);
    assert(strstr(content, "ключ = значение") != NULL);

    free(content);
    print_success("test_print_unicode passed\n");
}

void test_print_null_stream()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_print(NULL, ctx);
    assert(err.error == INI_PRINT_ERROR);
    assert(__ini_has_in_errstack(INI_PRINT_ERROR));

    ini_free(ctx);
    print_success("test_print_null_stream passed\n");
}

void test_print_malformed_section_names()
{
    FILE *output = tmpfile();
    assert(output != NULL);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Add a section with a malformed name
    ctx->sections = (ini_section_t *)malloc(sizeof(ini_section_t));
    assert(ctx->sections != NULL);
    ctx->section_count = 1;
    ctx->sections[0].name = strdup("bad\nsection");
    ctx->sections[0].pair_count = 0;
    ctx->sections[0].pairs = NULL;

    ini_error_details_t err = ini_print(output, ctx);
    assert(err.error == INI_SUCCESS); // Should skip malformed sections

    fclose(output);
    ini_free(ctx);
    print_success("test_print_malformed_section_names passed\n");
}

void test_print_malformed_pairs()
{
    FILE *output = tmpfile();
    assert(output != NULL);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Add a section with malformed pairs
    ctx->sections = (ini_section_t *)malloc(sizeof(ini_section_t));
    assert(ctx->sections != NULL);
    ctx->section_count = 1;
    ctx->sections[0].name = strdup("section");
    ctx->sections[0].pair_count = 2;
    ctx->sections[0].pairs = (ini_key_value_t *)malloc(2 * sizeof(ini_key_value_t));
    assert(ctx->sections[0].pairs != NULL);

    // First pair is valid
    ctx->sections[0].pairs[0].key = strdup("key1");
    ctx->sections[0].pairs[0].value = strdup("value1");

    // Second pair is invalid (NULL key)
    ctx->sections[0].pairs[1].key = NULL;
    ctx->sections[0].pairs[1].value = strdup("value2");

    ini_error_details_t err = ini_print(output, ctx);
    assert(err.error == INI_SUCCESS); // Should skip invalid pairs

    fclose(output);
    ini_free(ctx);
    print_success("test_print_malformed_pairs passed\n");
}

void test_print_large_output()
{
    FILE *output = tmpfile();
    assert(output != NULL);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    // Add 100 sections with 10 key-value pairs each
    ctx->sections = (ini_section_t *)malloc(100 * sizeof(ini_section_t));
    assert(ctx->sections != NULL);
    ctx->section_count = 100;

    for (int i = 0; i < 100; i++)
    {
        ctx->sections[i].name = (char *)malloc(16);
        sprintf(ctx->sections[i].name, "section%d", i);
        ctx->sections[i].pair_count = 10;
        ctx->sections[i].pairs = (ini_key_value_t *)malloc(10 * sizeof(ini_key_value_t));
        assert(ctx->sections[i].pairs != NULL);

        for (int j = 0; j < 10; j++)
        {
            ctx->sections[i].pairs[j].key = (char *)malloc(16);
            sprintf(ctx->sections[i].pairs[j].key, "key%d", j);
            ctx->sections[i].pairs[j].value = (char *)malloc(16);
            sprintf(ctx->sections[i].pairs[j].value, "value%d", j);
        }
    }

    ini_error_details_t err = ini_print(output, ctx);
    assert(err.error == INI_SUCCESS);

    fclose(output);
    ini_free(ctx);
    print_success("test_print_large_output passed\n");
}

void test_print_stream_errors()
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

    // Add a section to ensure the function tries to print
    ctx->sections = (ini_section_t *)malloc(sizeof(ini_section_t));
    assert(ctx->sections != NULL);
    ctx->section_count = 1;
    ctx->sections[0].name = strdup("section");
    ctx->sections[0].pair_count = 1;
    ctx->sections[0].pairs = (ini_key_value_t *)malloc(sizeof(ini_key_value_t));
    assert(ctx->sections[0].pairs != NULL);
    ctx->sections[0].pairs[0].key = strdup("key");
    ctx->sections[0].pairs[0].value = strdup("value");

    ini_error_details_t err = ini_print(read_only_stream, ctx);
    assert(err.error == INI_PRINT_ERROR);
    assert(__ini_has_in_errstack(INI_PRINT_ERROR));

    fclose(read_only_stream);
    ini_free(ctx);
    print_success("test_print_stream_errors passed\n");
}

void test_print_fprintf_failure()
{
    FILE *output = tmpfile();
    assert(output != NULL);
    fclose(output); // Close stream to force fprintf failure

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ctx->sections = (ini_section_t *)malloc(sizeof(ini_section_t));
    ctx->sections[0].name = strdup("section");
    ctx->section_count = 1;

    ini_error_details_t err = ini_print(output, ctx);
    assert(err.error == INI_PRINT_ERROR);
    ini_free(ctx);
    print_success("test_print_fprintf_failure passed\n");
}

void test_print_long_names()
{
    FILE *output = tmpfile();
    assert(output != NULL);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ctx->sections = (ini_section_t *)malloc(sizeof(ini_section_t));
    ctx->sections[0].name = (char *)malloc(INI_LINE_MAX + 2);
    memset(ctx->sections[0].name, 'a', INI_LINE_MAX + 1);
    ctx->sections[0].name[INI_LINE_MAX + 1] = '\0';
    ctx->section_count = 1;

    ini_error_details_t err = ini_print(output, ctx);
    assert(err.error == INI_SUCCESS); // Should truncate or handle gracefully
    fclose(output);
    ini_free(ctx);
    print_success("test_print_long_names passed\n");
}

void test_print_empty_sections()
{
    FILE *output = tmpfile();
    assert(output != NULL);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ctx->sections = (ini_section_t *)malloc(sizeof(ini_section_t));
    ctx->sections[0].name = strdup("empty_section");
    ctx->sections[0].pair_count = 0;
    ctx->sections[0].pairs = NULL;
    ctx->section_count = 1;

    ini_error_details_t err = ini_print(output, ctx);
    assert(err.error == INI_SUCCESS);
    fclose(output);
    ini_free(ctx);
    print_success("test_print_empty_sections passed\n");
}

void test_print_mixed_encoding()
{
    FILE *output = tmpfile();
    assert(output != NULL);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ctx->sections = (ini_section_t *)malloc(sizeof(ini_section_t));
    ctx->sections[0].name = strdup("mixed_section");
    ctx->sections[0].pair_count = 2;
    ctx->sections[0].pairs = (ini_key_value_t *)malloc(2 * sizeof(ini_key_value_t));
    ctx->sections[0].pairs[0].key = strdup("ascii_key");
    ctx->sections[0].pairs[0].value = strdup("ascii_value");
    ctx->sections[0].pairs[1].key = strdup("嗨。");
    ctx->sections[0].pairs[1].value = strdup("Cześć");
    ctx->section_count = 1;

    ini_error_details_t err = ini_print(output, ctx);
    assert(err.error == INI_SUCCESS);
    fclose(output);
    ini_free(ctx);
    print_success("test_print_mixed_encoding passed\n");
}

int main()
{
#ifdef __SANITIZE_ADDRESS__
    printf("AddressSanitizer is ACTIVE\n");
#else
    printf("AddressSanitizer is INACTIVE (check VS components)\n");
#endif

    __helper_init_log_file();
    ini_initialize();

    test_print_basic_section();
    test_print_with_subsections();
    test_print_unicode();
    test_print_null_stream();
    test_print_malformed_section_names();
    test_print_malformed_pairs();
    test_print_large_output();
    test_print_stream_errors();
    test_print_fprintf_failure();
    test_print_long_names();
    test_print_empty_sections();
    test_print_mixed_encoding();

    ini_finalize();
    print_success("All ini_print() tests passed!\n\n");
    __helper_close_log_file();
    return ini_has_error();
}
