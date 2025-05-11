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

#include "helper.h"
#include "iniparser.h"

void test_null_ctx_and_filepath()
{
    ini_error_details_t err = ini_load(NULL, NULL);
    assert(__ini_has_in_errstack(INI_INVALID_ARGUMENT) == 1);
    print_success("test_null_ctx_and_filepath passed\n");
}

void test_null_filepath()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, NULL);
    assert(__ini_has_in_errstack(INI_INVALID_ARGUMENT) == 1);
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_null_filepath passed\n");
}

void test_nonexistent_file()
{
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "nonexistent.ini");
    assert(__ini_has_in_errstack(INI_FILE_NOT_FOUND) == 1);
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_nonexistent_file passed\n");
}

void test_directory()
{
    create_test_dir("test_dir");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "test_dir");
    assert(__ini_has_in_errstack(INI_FILE_IS_DIR) == 1);
    remove_test_dir("test_dir");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_directory passed\n");
}

void test_empty_file()
{
    create_test_file("empty.ini", "");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "empty.ini");
    assert(__ini_has_in_errstack(INI_FILE_EMPTY) == 1);
    remove_test_file("empty.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_empty_file passed\n");
}

void test_valid_file()
{
    create_test_file("valid.ini", "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "valid.ini");
    assert(__ini_has_in_errstack(INI_SUCCESS) == 1);
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    remove_test_file("valid.ini");
    print_success("test_valid_file passed\n");
}

void test_bad_format_missing_bracket()
{
    create_test_file("bad_missing_bracket.ini", "[section\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "bad_missing_bracket.ini");
    assert(__ini_has_in_errstack(INI_FILE_BAD_FORMAT) == 1);
    remove_test_file("bad_missing_bracket.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_bad_format_missing_bracket passed\n");
}

void test_bad_format_empty_key()
{
    create_test_file("bad_empty_key.ini", "[section]\n=value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "bad_empty_key.ini");
    assert(__ini_has_in_errstack(INI_FILE_BAD_FORMAT) == 1);
    remove_test_file("bad_empty_key.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_bad_format_empty_key passed\n");
}

void test_empty_value()
{
    create_test_file("empty_value.ini", "[section]\nkey=\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "empty_value.ini");
    assert(err.error == INI_SUCCESS);
    remove_test_file("empty_value.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_empty_value passed\n");
}

void test_bad_format_unbalanced_quotes()
{
    create_test_file("bad_unbalanced_quotes.ini", "[section]\nkey=\"value\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "bad_unbalanced_quotes.ini");
    assert(__ini_has_in_errstack(INI_FILE_BAD_FORMAT) == 1);
    remove_test_file("bad_unbalanced_quotes.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_bad_format_unbalanced_quotes passed\n");
}

void test_bad_format_arrays_not_supported()
{
    create_test_file("bad_arrays.ini", "[section]\nkey=1,2,3\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "bad_arrays.ini");
    assert(__ini_has_in_errstack(INI_FILE_BAD_FORMAT) == 1);
    remove_test_file("bad_arrays.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_bad_format_arrays_not_supported passed\n");
}

void test_utf8_chars()
{
    create_test_file("utf8.ini", "[секция]\nключ=значение\n[节]\n键=值\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "utf8.ini");
    assert(__ini_has_in_errstack(INI_SUCCESS) == 1);
    remove_test_file("utf8.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_utf8_chars passed\n");
}

void test_windows_line_endings()
{
    create_test_file("windows.ini", "[section]\r\nkey=value\r\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "windows.ini");
    assert(__ini_has_in_errstack(INI_SUCCESS) == 1);
    remove_test_file("windows.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_windows_line_endings passed\n");
}

void test_line_too_long()
{
    char long_line[INI_LINE_MAX + 2];
    memset(long_line, 'a', INI_LINE_MAX + 1);
    long_line[INI_LINE_MAX + 1] = '\0';

    create_test_file("long_line.ini", long_line);
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "long_line.ini");
    assert(__ini_has_in_errstack(INI_FILE_BAD_FORMAT) == 1);
    remove_test_file("long_line.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_line_too_long passed\n");
}

void test_file_deleted_during_check()
{
    char const *filepath = "deleted_during_check.ini";
    create_test_file(filepath, "[section]\nkey=value\n");
    remove_test_file(filepath);
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, filepath);
    assert(__ini_has_in_errstack(INI_FILE_NOT_FOUND) == 1);
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_file_deleted_during_check passed\n");
}

void test_binary_data()
{
    char const *filepath = "binary.ini";
    FILE *fp = fopen(filepath, "wb");
    assert(fp != NULL);

    unsigned char binary_data[] = {0x01, 0x02, 0x03, 0x00, 0xFF, 0xFE, 0xFD};
    assert(fwrite(binary_data, sizeof(binary_data), 1, fp) == 1);
    assert(fclose(fp) == 0);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, filepath);
    assert(__ini_has_in_errstack(INI_FILE_BAD_FORMAT) == 1);
    remove_test_file(filepath);
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_binary_data passed\n");
}

#if INI_OS_LINUX
void test_no_read_permission()
{
    create_test_file("no_read.ini", "[section]\nkey=value\n");
    assert(chmod("no_read.ini", 0333) == 0);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "no_read.ini");
    assert(__ini_has_in_errstack(INI_FILE_OPEN_FAILED) == 1);
    remove_test_file("no_read.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_no_read_permission passed\n");
}

void test_symlink()
{
    create_test_file("target.ini", "[section]\nkey=value\n");
    assert(symlink("target.ini", "symlink.ini") == 0);

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "symlink.ini");
    assert(err.error == INI_SUCCESS);
    remove_test_file("symlink.ini");
    remove_test_file("target.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_symlink passed\n");
}

void test_special_chars()
{
    create_test_file("special.ini", "[sec#tion]\nke=y=val\\;ue\n");
    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "special.ini");
    assert(err.error == INI_SUCCESS);
    remove_test_file("special.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
    print_success("test_special_chars passed\n");
}
#endif

void test_reuse_ctx()
{
    create_test_file("valid1.ini", "[section1]\nkey1=value1\n");
    create_test_file("valid2.ini", "[section2]\nkey2=value2\n");

    ini_context_t *ctx = ini_create_context();
    assert(ctx != NULL);
    ini_error_details_t err = ini_load(ctx, "valid1.ini");
    assert(err.error == INI_SUCCESS);

    err = ini_load(ctx, "valid2.ini");
    assert(err.error == INI_SUCCESS);

    print_success("test_reuse_ctx passed\n");
    remove_test_file("valid1.ini");
    remove_test_file("valid2.ini");
    err = ini_free(ctx);
    assert(err.error == INI_SUCCESS);
}

int main()
{
    __helper_init_log_file();
    ini_initialize();

    test_null_ctx_and_filepath();
    test_null_filepath();
    test_nonexistent_file();
    test_directory();
    test_empty_file();
    test_valid_file();
    test_bad_format_missing_bracket();
    test_bad_format_empty_key();
    test_empty_value();
    test_bad_format_unbalanced_quotes();
    test_bad_format_arrays_not_supported();
    test_utf8_chars();
    test_windows_line_endings();
    test_line_too_long();
    test_file_deleted_during_check();
    test_binary_data();
    test_reuse_ctx();

#if INI_OS_LINUX
    test_no_read_permission();
    test_symlink();
    test_special_chars();
#endif

    ini_finalize();
    print_success("All ini_load() tests passed!\n\n");
    __helper_close_log_file();
    return ini_has_error();
}
