#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ini_os_check.h"

#if INI_OS_LINUX
#include <sys/stat.h>
#include <unistd.h>
#elif INI_OS_WINDOWS
#include <windows.h>
#endif

#include "helper.h"
#include "ini_filesystem.h"
#include "ini_parser.h"

#define OUTPUT_FILE "test_functional_output.ini"

// ==================== Test Cases ====================

// --- Clean Test 1: Basic Load and Save ---
void test_basic_load_save()
{
    char TEST_FILE[] = "test_basic_load_save.ini";
    create_test_file(TEST_FILE, "[section]\nkey=value\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_SUCCESS);

    err = ini_save(ctx, OUTPUT_FILE);
    assert(err == INI_SUCCESS);

    // Verify the saved file
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, OUTPUT_FILE);
    assert(err == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx2, "section", "key", &value);
    assert(err == INI_SUCCESS);
    assert(strcmp(value, "value") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    remove_test_file(OUTPUT_FILE);

    print_success("test_basic_load_save passed\n");
}

// --- Dirty Test 1: Load Nonexistent File ---
void test_load_nonexistent_file()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, "nonexistent.ini");
    assert(err == INI_FILE_NOT_FOUND);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    print_success("test_load_nonexistent_file passed\n");
}

// --- Dirty Test 2: Save to Read-Only Directory (Unix) ---
void test_save_readonly_dir_unix()
{
#if INI_OS_LINUX
    char TEST_FILE[] = "test_save_readonly_dir_unix.ini";
    mkdir("readonly_dir", 0777);
    // chmod("readonly_dir", 0444); // r--r--r--

    // ini_context_t *ctx = ini_create_context();
    // assert(ctx != NULL);

    // create_test_file(TEST_FILE, "[section]\nkey=value\n");
    // ini_error_t err = ini_load(ctx, TEST_FILE);
    // assert(err == INI_SUCCESS);

    // err = ini_save(ctx, "readonly_dir/test.ini");
    // assert(err == INI_FILE_PERMISSION_DENIED);

    // chmod("readonly_dir", 0777); // Restore permissions
    // rmdir("readonly_dir");
    // err = ini_free(ctx);
    // assert(err == INI_SUCCESS);
    // remove_test_file(TEST_FILE);
    // print_success("test_save_readonly_dir_unix passed\n");
#endif
}

// --- Dirty Test 3: Save to Read-Only Directory (Windows) ---
void test_save_readonly_dir_windows()
{
#if INI_OS_WINDOWS
    char TEST_FILE[] = "test_save_readonly_dir_windows.ini";
    CreateDirectory("readonly_dir", NULL);
    SetFileAttributes("readonly_dir", FILE_ATTRIBUTE_READONLY);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_SUCCESS);

    err = ini_save(ctx, "readonly_dir\\test.ini");
    assert(err == INI_FILE_PERMISSION_DENIED);

    SetFileAttributes("readonly_dir", FILE_ATTRIBUTE_NORMAL);
    RemoveDirectory("readonly_dir");
    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_save_readonly_dir_windows passed\n");
#endif
}

// --- Dirty Test 4: Corrupt INI File ---
void test_corrupt_ini_file()
{
    char TEST_FILE[] = "test_corrupt_ini_file.ini";
    create_test_file(TEST_FILE, "[section\nkey=value\n"); // Missing closing bracket

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_corrupt_ini_file passed\n");
}

// --- Dirty Test 5: Empty File ---
void test_empty_file()
{
    char TEST_FILE[] = "test_empty_file.ini";
    create_test_file(TEST_FILE, ""); // Empty file

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    size_t file_size = 0;
    ini_fs_error_t fs_err = ini_get_file_size(TEST_FILE, &file_size);
    assert(fs_err == INI_FS_SUCCESS);
    assert(file_size == 0);
    assert(err == INI_FILE_EMPTY);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_empty_file passed\n");
}

// --- Clean Test 2: Unicode Support ---
void test_unicode_support()
{
    char TEST_FILE[] = "test_unicode_support.ini";
    create_test_file(TEST_FILE, "[секция]\nключ=значение\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_SUCCESS);

    err = ini_save(ctx, OUTPUT_FILE);
    assert(err == INI_SUCCESS);

    // Verify the saved file
    ini_context_t *ctx2 = ini_create_context();
    assert(ctx2 != NULL);

    err = ini_load(ctx2, OUTPUT_FILE);
    assert(err == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx2, "секция", "ключ", &value);
    assert(err == INI_SUCCESS);
    assert(strcmp(value, "значение") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    err = ini_free(ctx2);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    remove_test_file(OUTPUT_FILE);
    print_success("test_unicode_support passed\n");
}

// --- Dirty Test 6: Invalid UTF-8 ---
void test_invalid_utf8()
{
    char TEST_FILE[] = "test_invalid_utf8.ini";
    FILE *file = fopen(TEST_FILE, "wb");
    assert(file != NULL);
    // Write invalid UTF-8 sequence
    unsigned char invalid_utf8[] = {0xFF, 0xFE, 0x00};
    fwrite(invalid_utf8, sizeof(invalid_utf8), 1, file);
    fclose(file);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_invalid_utf8 passed\n");
}

// --- Dirty Test 7: Binary Data ---
void test_binary_data()
{
    char TEST_FILE[] = "test_binary_data.ini";
    FILE *file = fopen(TEST_FILE, "wb");
    assert(file != NULL);
    unsigned char binary_data[] = {0x01, 0x02, 0x03, 0x00, 0xFF, 0xFE, 0xFD};
    fwrite(binary_data, sizeof(binary_data), 1, file);
    fclose(file);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_binary_data passed\n");
}

// --- Dirty Test 8: Line Too Long ---
void test_line_too_long()
{
    char TEST_FILE[] = "test_line_too_long.ini";
    FILE *file = fopen(TEST_FILE, "w");
    assert(file != NULL);
    for (int i = 0; i < INI_LINE_MAX + 2; i++)
    {
        fputc('a', file);
    }
    fclose(file);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_line_too_long passed\n");
}

// --- Dirty Test 9: Missing Section ---
void test_missing_section()
{
    char TEST_FILE[] = "test_missing_section.ini";
    create_test_file(TEST_FILE, "key=value\n"); // No section

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_missing_section passed\n");
}

// --- Dirty Test 10: Escaped Characters ---
void test_escaped_characters()
{
    char TEST_FILE[] = "test_escaped_characters.ini";
    // Write the test file with properly escaped quotes
    create_test_file(TEST_FILE, "[section]\nkey=\"value\\\"with\\\"quotes\"\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_escaped_characters passed\n");
}

// --- Clean Test 3: Nested Sections ---
void test_nested_sections()
{
    char TEST_FILE[] = "test_nested_sections.ini";
    create_test_file(TEST_FILE, "[parent]\nkey1=value1\n[parent.child]\nkey2=value2\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "parent", "key1", &value);
    assert(err == INI_SUCCESS);
    assert(strcmp(value, "value1") == 0);
    free(value);

    err = ini_get_value(ctx, "parent.child", "key2", &value);
    assert(err == INI_SUCCESS);
    assert(strcmp(value, "value2") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_nested_sections passed\n");
}

// --- Dirty Test 11: Malformed Key ---
void test_malformed_key()
{
    char TEST_FILE[] = "test_malformed_key.ini";
    create_test_file(TEST_FILE, "[section]\n=value\n"); // Empty key

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_FILE_BAD_FORMAT);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_malformed_key passed\n");
}

// --- Dirty Test 12: Malformed Value ---
void test_malformed_value()
{
    char TEST_FILE[] = "test_malformed_value.ini";
    create_test_file(TEST_FILE, "[section]\nkey=\n"); // Empty value

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_SUCCESS); // Empty values are allowed

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err == INI_SUCCESS);
    assert(strcmp(value, "") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_malformed_value passed\n");
}

// --- Clean Test 4: Comments ---
void test_comments()
{
    char TEST_FILE[] = "test_comments.ini";
    create_test_file(TEST_FILE, "; Comment\n[section]\nkey=value\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_SUCCESS);

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err == INI_SUCCESS);
    assert(strcmp(value, "value") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_comments passed\n");
}

// --- Dirty Test 13: Invalid Comment ---
void test_invalid_comment()
{
    char TEST_FILE[] = "test_invalid_comment.ini";
    create_test_file(TEST_FILE, "[section]\n# Invalid comment\nkey=value\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);

    ini_error_t err = ini_load(ctx, TEST_FILE);
    assert(err == INI_SUCCESS); // Non-standard comments are ignored

    char *value = NULL;
    err = ini_get_value(ctx, "section", "key", &value);
    assert(err == INI_SUCCESS);
    assert(strcmp(value, "value") == 0);
    free(value);

    err = ini_free(ctx);
    assert(err == INI_SUCCESS);
    remove_test_file(TEST_FILE);
    print_success("test_invalid_comment passed\n");
}

// ==================== Main ====================
int main()
{
    __helper_init_log_file();

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

    print_success("All functional tests passed!\n\n");
    __helper_close_log_file();
    return EXIT_SUCCESS;
}
