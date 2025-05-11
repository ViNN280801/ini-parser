#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if INI_OS_UNIX
#include <sys/stat.h>
#include <unistd.h>
#elif INI_OS_WINDOWS
#include <windows.h>
#endif

#include "helper.h"
#include "iniparser.h"

#define TEST_FILE "test_functional.ini"
#define OUTPUT_FILE "test_functional_output.ini"

// ==================== Test Cases ====================

// --- Clean Test 1: Basic Load and Save ---
void test_basic_load_save()
{
    create_test_file(TEST_FILE, "[section]\nkey=value\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    err = ini_save(ctx, OUTPUT_FILE);
    assert(err.error == INI_SUCCESS);

    // Verify the saved file
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, OUTPUT_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx2, "section", "key", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value") == 0);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    remove_test_file(OUTPUT_FILE);

    print_success("test_basic_load_save passed\n");
}

// --- Dirty Test 1: Load Nonexistent File ---
void test_load_nonexistent_file()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, "nonexistent.ini");
    assert(err.error == INI_FILE_NOT_FOUND);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_load_nonexistent_file passed\n");
}

// --- Dirty Test 2: Save to Read-Only Directory (Unix) ---
void test_save_readonly_dir_unix()
{
#if INI_OS_LINUX
    mkdir("readonly_dir", 0777);
    chmod("readonly_dir", 0555); // r-xr-xr-x

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    err = ini_save(ctx, "readonly_dir/test.ini");
    assert(err.error == INI_FILE_OPEN_FAILED);

    chmod("readonly_dir", 0777); // Restore permissions
    rmdir("readonly_dir");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_save_readonly_dir_unix passed\n");
#endif
}

// --- Dirty Test 3: Save to Read-Only Directory (Windows) ---
void test_save_readonly_dir_windows()
{
#if INI_OS_WINDOWS
    CreateDirectory("readonly_dir", NULL);
    SetFileAttributes("readonly_dir", FILE_ATTRIBUTE_READONLY);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    err = ini_save(ctx, "readonly_dir\\test.ini");
    assert(err.error == INI_FILE_OPEN_FAILED);

    SetFileAttributes("readonly_dir", FILE_ATTRIBUTE_NORMAL);
    RemoveDirectory("readonly_dir");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_save_readonly_dir_windows passed\n");
#endif
}

// --- Dirty Test 4: Corrupt INI File ---
void test_corrupt_ini_file()
{
    create_test_file(TEST_FILE, "[section\nkey=value\n"); // Missing closing bracket

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_corrupt_ini_file passed\n");
}

// --- Dirty Test 5: Empty File ---
void test_empty_file()
{
    create_test_file(TEST_FILE, ""); // Empty file

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_FILE_EMPTY);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_empty_file passed\n");
}

// --- Clean Test 2: Unicode Support ---
void test_unicode_support()
{
    create_test_file(TEST_FILE, "[секция]\nключ=значение\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    err = ini_save(ctx, OUTPUT_FILE);
    assert(err.error == INI_SUCCESS);

    // Verify the saved file
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, OUTPUT_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx2, "секция", "ключ", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "значение") == 0);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    remove_test_file(OUTPUT_FILE);
    print_success("test_unicode_support passed\n");
}

// --- Dirty Test 6: Invalid UTF-8 ---
void test_invalid_utf8()
{
    FILE *file = fopen(TEST_FILE, "wb");
    assert(file != NULL);
    // Write invalid UTF-8 sequence
    unsigned char invalid_utf8[] = {0xFF, 0xFE, 0x00};
    fwrite(invalid_utf8, sizeof(invalid_utf8), 1, file);
    fclose(file);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_invalid_utf8 passed\n");
}

// --- Dirty Test 7: Binary Data ---
void test_binary_data()
{
    FILE *file = fopen(TEST_FILE, "wb");
    assert(file != NULL);
    unsigned char binary_data[] = {0x01, 0x02, 0x03, 0x00, 0xFF, 0xFE, 0xFD};
    fwrite(binary_data, sizeof(binary_data), 1, file);
    fclose(file);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_binary_data passed\n");
}

// --- Dirty Test 8: Line Too Long ---
void test_line_too_long()
{
    FILE *file = fopen(TEST_FILE, "w");
    assert(file != NULL);
    for (int i = 0; i < INI_LINE_MAX + 2; i++)
    {
        fputc('a', file);
    }
    fclose(file);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_line_too_long passed\n");
}

// --- Dirty Test 9: Missing Section ---
void test_missing_section()
{
    create_test_file(TEST_FILE, "key=value\n"); // No section

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_missing_section passed\n");
}

// --- Dirty Test 10: Escaped Characters ---
void test_escaped_characters()
{
    // Write the test file with properly escaped quotes
    create_test_file(TEST_FILE, "[section]\nkey=\"value\\\"with\\\"quotes\"\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);

    remove_test_file(TEST_FILE);
    print_success("test_escaped_characters passed\n");
}

// --- Clean Test 3: Nested Sections ---
void test_nested_sections()
{
    create_test_file(TEST_FILE, "[parent]\nkey1=value1\n[parent.child]\nkey2=value2\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "parent", "key1", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value1") == 0);

    err = ini_get_value(ctx, "parent.child", "key2", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value2") == 0);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_nested_sections passed\n");
}

// --- Dirty Test 11: Malformed Key ---
void test_malformed_key()
{
    create_test_file(TEST_FILE, "[section]\n=value\n"); // Empty key

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_malformed_key passed\n");
}

// --- Dirty Test 12: Malformed Value ---
void test_malformed_value()
{
    create_test_file(TEST_FILE, "[section]\nkey=\n"); // Empty value

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS); // Empty values are allowed

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "") == 0);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_malformed_value passed\n");
}

// --- Clean Test 4: Comments ---
void test_comments()
{
    create_test_file(TEST_FILE, "; Comment\n[section]\nkey=value\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value") == 0);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_comments passed\n");
}

// --- Dirty Test 13: Invalid Comment ---
void test_invalid_comment()
{
    create_test_file(TEST_FILE, "[section]\n# Invalid comment\nkey=value\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_details_t err = ini_load(ctx, TEST_FILE);
    assert(err.error == INI_SUCCESS); // Non-standard comments are ignored

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err.error == INI_SUCCESS);
    assert(strcmp(value, "value") == 0);

    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_invalid_comment passed\n");
}

// ==================== Main ====================
int main()
{
    __helper_init_log_file();
    ini_initialize();

    // Clean Tests
    test_basic_load_save();
    test_unicode_support();
    test_nested_sections();
    test_comments();

    // Dirty Tests
    test_load_nonexistent_file();
    test_save_readonly_dir_unix();
    test_save_readonly_dir_windows();
    test_corrupt_ini_file();
    test_empty_file();
    test_invalid_utf8();
    test_binary_data();
    test_line_too_long();
    test_missing_section();
    test_malformed_key();
    test_malformed_value();
    test_invalid_comment();
    test_escaped_characters();

    ini_finalize();
    print_success("All functional tests passed!\n\n");
    __helper_close_log_file();
    return ini_has_error();
}
