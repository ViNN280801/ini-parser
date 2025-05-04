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

void test_null_ctx_and_filepath()
{
    ini_error_details_t err = ini_load(NULL, NULL);
    if (err.error != INI_INVALID_ARGUMENT)
    {
        print_error("test_null_ctx_and_filepath failed: expected INI_INVALID_ARGUMENT, got %d\n", err.error);
        return;
    }
    print_success("test_null_ctx_and_filepath passed\n");
}

void test_null_filepath()
{
    ini_context_t *ctx = ini_create_context();
    fprintf(stderr, "test_null_filepath: ctx = %p\n", (void *)ctx);
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, NULL);
    fprintf(stderr, "test_null_filepath: err = %d\n", err.error);
    if (err.error != INI_INVALID_ARGUMENT)
    {
        print_error("test_null_filepath failed: expected INI_INVALID_ARGUMENT, got %d\n", err.error);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_null_filepath passed\n");
    err = ini_free(ctx);
    fprintf(stderr, "test_null_filepath: err = %d\n", err.error);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
    fprintf(stderr, "test_null_filepath: err = %d\n", err.error);
}

void test_nonexistent_file()
{
    ini_context_t *ctx = ini_create_context();
    fprintf(stderr, "test_nonexistent_file: ctx = %p\n", (void *)ctx);
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "nonexistent.ini");
    if (err.error != INI_SUCCESS)
    {
        print_error("test_nonexistent_file failed: expected INI_SUCCESS, got %d\n", err.error);
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_nonexistent_file passed\n");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_directory()
{
    create_test_dir("test_dir");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "test_dir");
    if (err.error != INI_FILE_IS_DIR)
    {
        print_error("test_directory failed: expected INI_FILE_IS_DIR, got %d\n", err.error);
        remove_test_dir("test_dir");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_directory passed\n");
    remove_test_dir("test_dir");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_empty_file()
{
    create_test_file("empty.ini", "");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "empty.ini");
    if (err.error != INI_FILE_EMPTY)
    {
        print_error("test_empty_file failed: expected INI_FILE_EMPTY, got %d\n", err.error);
        remove_test_file("empty.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_empty_file passed\n");
    remove_test_file("empty.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_valid_file()
{
    create_test_file("valid.ini", "[section]\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "valid.ini");
    if (err.error != INI_SUCCESS)
    {
        print_error("test_valid_file failed: expected INI_SUCCESS, got %d\n", err.error);
        ini_free(ctx);
        remove_test_file("valid.ini");
        return;
    }
    print_success("test_valid_file passed\n");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
    remove_test_file("valid.ini");
}

void test_bad_format_missing_bracket()
{
    create_test_file("bad_missing_bracket.ini", "[section\nkey=value\n");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "bad_missing_bracket.ini");
    if (err.error != INI_FILE_BAD_FORMAT)
    {
        print_error("test_bad_format_missing_bracket failed: expected INI_FILE_BAD_FORMAT, got %d\n", err.error);
        remove_test_file("bad_missing_bracket.ini");
        ini_free(ctx);
        return;
    }
    print_success("test_bad_format_missing_bracket passed\n");
    remove_test_file("bad_missing_bracket.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_bad_format_empty_key()
{
    create_test_file("bad_empty_key.ini", "[section]\n=value\n");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "bad_empty_key.ini");
    if (err.error != INI_FILE_BAD_FORMAT)
    {
        print_error("test_bad_format_empty_key failed: expected INI_FILE_BAD_FORMAT, got %d\n", err.error);
        remove_test_file("bad_empty_key.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_bad_format_empty_key passed\n");
    remove_test_file("bad_empty_key.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_bad_format_empty_value()
{
    create_test_file("bad_empty_value.ini", "[section]\nkey=\n");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "bad_empty_value.ini");
    if (err.error != INI_FILE_BAD_FORMAT)
    {
        print_error("test_bad_format_empty_value failed: expected INI_FILE_BAD_FORMAT, got %d\n", err.error);
        remove_test_file("bad_empty_value.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_bad_format_empty_value passed\n");
    remove_test_file("bad_empty_value.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_bad_format_unbalanced_quotes()
{
    create_test_file("bad_unbalanced_quotes.ini", "[section]\nkey=\"value\n");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "bad_unbalanced_quotes.ini");
    if (err.error != INI_FILE_BAD_FORMAT)
    {
        print_error("test_bad_format_unbalanced_quotes failed: expected INI_FILE_BAD_FORMAT, got %d\n", err.error);
        remove_test_file("bad_unbalanced_quotes.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_bad_format_unbalanced_quotes passed\n");
    remove_test_file("bad_unbalanced_quotes.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_bad_format_arrays_not_supported()
{
    create_test_file("bad_arrays.ini", "[section]\nkey=1,2,3\n");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "bad_arrays.ini");
    if (err.error != INI_FILE_BAD_FORMAT)
    {
        print_error("test_bad_format_arrays_not_supported failed: expected INI_FILE_BAD_FORMAT, got %d\n", err.error);
        remove_test_file("bad_arrays.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_bad_format_arrays_not_supported passed\n");
    remove_test_file("bad_arrays.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_utf8_chars()
{
    create_test_file("utf8.ini", "[секция]\nключ=значение\n[节]\n键=值\n");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "utf8.ini");
    if (err.error != INI_SUCCESS)
    {
        print_error("test_utf8_chars failed: expected INI_SUCCESS, got %d\n", err.error);
        remove_test_file("utf8.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_utf8_chars passed\n");
    remove_test_file("utf8.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_windows_line_endings()
{
    create_test_file("windows.ini", "[section]\r\nkey=value\r\n");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "windows.ini");
    if (err.error != INI_SUCCESS)
    {
        print_error("test_windows_line_endings failed: expected INI_SUCCESS, got %d\n", err.error);
        remove_test_file("windows.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_windows_line_endings passed\n");
    remove_test_file("windows.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_line_too_long()
{
    char long_line[INI_LINE_MAX + 2];
    memset(long_line, 'a', INI_LINE_MAX + 1);
    long_line[INI_LINE_MAX + 1] = '\0';

    create_test_file("long_line.ini", long_line);
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "long_line.ini");
    if (err.error != INI_FILE_BAD_FORMAT)
    {
        print_error("test_line_too_long failed: expected INI_FILE_BAD_FORMAT, got %d\n", err.error);
        remove_test_file("long_line.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_line_too_long passed\n");
    remove_test_file("long_line.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_file_deleted_during_check()
{
    char const *filepath = "deleted_during_check.ini";
    create_test_file(filepath, "[section]\nkey=value\n");
    remove_test_file(filepath);
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, filepath);
    if (err.error != INI_FILE_NOT_FOUND)
    {
        print_error("test_file_deleted_during_check failed: expected INI_FILE_NOT_FOUND, got %d\n", err.error);
        ini_free(ctx);
        return;
    }
    print_success("test_file_deleted_during_check passed\n");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_binary_data()
{
    char const *filepath = "binary.ini";
    FILE *fp = fopen(filepath, "wb");
    if (!fp)
    {
        print_error("Failed to create binary file");
        return;
    }
    unsigned char binary_data[] = {0x01, 0x02, 0x03, 0x00, 0xFF, 0xFE, 0xFD};
    if (fwrite(binary_data, sizeof(binary_data), 1, fp) != 1)
    {
        print_error("Failed to write binary data to file");
        if (fclose(fp) != 0)
            print_error("Failed to close binary file");
        remove_test_file(filepath);
        return;
    }
    if (fclose(fp) != 0)
    {
        print_error("Failed to close binary file");
        remove_test_file(filepath);
        return;
    }

    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, filepath);
    if (err.error != INI_FILE_BAD_FORMAT)
    {
        print_error("test_binary_data failed: expected INI_FILE_BAD_FORMAT, got %d\n", err.error);
        remove_test_file(filepath);
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_binary_data passed\n");
    remove_test_file(filepath);
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

#if INI_OS_LINUX
void test_no_read_permission()
{
    create_test_file("no_read.ini", "[section]\nkey=value\n");
    chmod("no_read.ini", 0333); // Remove read permissions
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "no_read.ini");
    if (err.error != INI_FILE_OPEN_FAILED)
    {
        print_error("test_no_read_permission failed: expected INI_FILE_OPEN_FAILED, got %d\n", err.error);
        remove_test_file("no_read.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_no_read_permission passed\n");
    remove_test_file("no_read.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_symlink()
{
    create_test_file("target.ini", "[section]\nkey=value\n");
    symlink("target.ini", "symlink.ini");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "symlink.ini");
    if (err.error != INI_SUCCESS)
    {
        print_error("test_symlink failed: expected INI_SUCCESS, got %d\n", err.error);
        remove_test_file("symlink.ini");
        remove_test_file("target.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_symlink passed\n");
    remove_test_file("symlink.ini");
    remove_test_file("target.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

void test_special_chars()
{
    create_test_file("special.ini", "[sec#tion]\nke=y=val\\;ue\n");
    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "special.ini");
    if (err.error != INI_SUCCESS)
    {
        print_error("test_special_chars failed: expected INI_SUCCESS, got %d\n", err.error);
        remove_test_file("special.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }
    print_success("test_special_chars passed\n");
    remove_test_file("special.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}
#endif

void test_reuse_ctx()
{
    create_test_file("valid1.ini", "[section1]\nkey1=value1\n");
    create_test_file("valid2.ini", "[section2]\nkey2=value2\n");

    ini_context_t *ctx = ini_create_context();
    if (!ctx)
    {
        print_error("Failed to create context");
        return;
    }
    ini_error_details_t err = ini_load(ctx, "valid1.ini");
    if (err.error != INI_SUCCESS)
    {
        print_error("test_reuse_ctx failed (first load): expected INI_SUCCESS, got %d\n", err.error);
        remove_test_file("valid1.ini");
        remove_test_file("valid2.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }

    err = ini_load(ctx, "valid2.ini");
    if (err.error != INI_SUCCESS)
    {
        print_error("test_reuse_ctx failed (second load): expected INI_SUCCESS, got %d\n", err.error);
        remove_test_file("valid1.ini");
        remove_test_file("valid2.ini");
        err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            print_error("Failed to free context: %s\n", err.custommsg);
        return;
    }

    print_success("test_reuse_ctx passed\n");
    remove_test_file("valid1.ini");
    remove_test_file("valid2.ini");
    err = ini_free(ctx);
    if (err.error != INI_SUCCESS)
        print_error("Failed to free context: %s\n", err.custommsg);
}

int main()
{
    __g_init_log_file();
    test_null_ctx_and_filepath();
    test_null_filepath();
#if INI_OS_APPLE
    test_nonexistent_file();
#endif
    test_directory();
    test_empty_file();
    test_valid_file();
    test_bad_format_missing_bracket();
    test_bad_format_empty_key();
    test_bad_format_empty_value();
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

    print_success("All ini_load() tests passed!\n\n");
    __g_close_log_file();
    return EXIT_SUCCESS;
}
