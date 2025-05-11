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

#define TEST_FILE "test.ini"

void test_null_filepath()
{
    ini_error_details_t err = ini_good(NULL);
    assert(err.error == INI_INVALID_ARGUMENT);
    print_success("test_null_filepath passed\n");
}

void test_nonexistent_file()
{
    ini_error_details_t err = ini_good("nonexistent.ini");
    assert(err.error == INI_FILE_NOT_FOUND);
    print_success("test_nonexistent_file passed\n");
}

void test_directory()
{
    create_test_dir("test_dir");
    ini_error_details_t err = ini_good("test_dir");
    assert(err.error == INI_FILE_IS_DIR);
    remove_test_dir("test_dir");
    print_success("test_directory passed\n");
}

void test_empty_file()
{
    create_test_file(TEST_FILE, "");
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_FILE_EMPTY);
    print_success("test_empty_file passed\n");
    remove_test_file(TEST_FILE);
}

void test_valid_file()
{
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_SUCCESS);
    print_success("test_valid_file passed\n");
    remove_test_file(TEST_FILE);
}

void test_bad_format_missing_bracket()
{
    create_test_file(TEST_FILE, "[section\nkey=value\n");
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);
    print_success("test_bad_format_missing_bracket passed\n");
    remove_test_file(TEST_FILE);
}

void test_bad_format_empty_key()
{
    create_test_file(TEST_FILE, "[section]\n=value\n");
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);
    print_success("test_bad_format_empty_key passed\n");
    remove_test_file(TEST_FILE);
}

void test_empty_value()
{
    create_test_file(TEST_FILE, "[section]\nkey=\n");
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_SUCCESS);
    print_success("test_empty_value passed\n");
    remove_test_file(TEST_FILE);
}

void test_bad_format_unbalanced_quotes()
{
    create_test_file(TEST_FILE, "[section]\nkey=\"value\n");
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);
    print_success("test_bad_format_unbalanced_quotes passed\n");
    remove_test_file(TEST_FILE);
}

void test_bad_format_arrays_not_supported()
{
    create_test_file(TEST_FILE, "[section]\nkey=1,2,3\n");
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);
    print_success("test_bad_format_arrays_not_supported passed\n");
    remove_test_file(TEST_FILE);
}

void test_utf8_chars()
{
    create_test_file(TEST_FILE, "[секция]\nключ=значение\n[节]\n键=值\n");
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_SUCCESS);
    print_success("test_utf8_chars passed\n");
    remove_test_file(TEST_FILE);
}

void test_windows_line_endings()
{
    create_test_file(TEST_FILE, "[section]\r\nkey=value\r\n");
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_SUCCESS);
    print_success("test_windows_line_endings passed\n");
    remove_test_file(TEST_FILE);
}

void test_line_too_long()
{
    char long_line[INI_LINE_MAX + 2];
    memset(long_line, 'a', INI_LINE_MAX + 1);
    for (int i = 0; i < INI_LINE_MAX + 1; i++)
        assert(long_line[i] == 'a');

    create_test_file(TEST_FILE, long_line);
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);
    print_success("test_line_too_long passed\n");
    remove_test_file(TEST_FILE);
}

void test_file_deleted_during_check()
{
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    remove_test_file(TEST_FILE);
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_FILE_NOT_FOUND);
    print_success("test_file_deleted_during_check passed\n");
}

void test_binary_data()
{
    FILE *fp = fopen(TEST_FILE, "wb");
    assert(fp != NULL);
    unsigned char binary_data[] = {0x01, 0x02, 0x03, 0x00, 0xFF, 0xFE, 0xFD};
    assert(fwrite(binary_data, sizeof(binary_data), 1, fp) == 1);
    assert(fclose(fp) == 0);
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_FILE_BAD_FORMAT);
    print_success("test_binary_data passed\n");
    remove_test_file(TEST_FILE);
}

#if INI_OS_LINUX
void test_no_read_permission()
{
    create_test_file(TEST_FILE, "[section]\nkey=value\n");
    chmod(TEST_FILE, 0333);
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_FILE_OPEN_FAILED);
    print_success("test_no_read_permission passed\n");
    remove_test_file(TEST_FILE);
}

void test_symlink()
{
    create_test_file("target.ini", "[section]\nkey=value\n");
    symlink("target.ini", "symlink.ini");
    ini_error_details_t err = ini_good("symlink.ini");
    assert(err.error == INI_SUCCESS);
    print_success("test_symlink passed\n");
    remove_test_file("symlink.ini");
    remove_test_file("target.ini");
}

void test_special_chars()
{
    create_test_file(TEST_FILE, "[sec#tion]\nke=y=val\\;ue\n");
    ini_error_details_t err = ini_good(TEST_FILE);
    assert(err.error == INI_SUCCESS);
    print_success("test_special_chars passed\n");
    remove_test_file(TEST_FILE);
}
#endif

int main()
{
    __helper_init_log_file();
    ini_initialize();

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

#if INI_OS_LINUX
    test_no_read_permission();
    test_symlink();
    test_special_chars();
#endif

    ini_finalize();
    print_success("All ini_good() tests passed!\n\n");
    __helper_close_log_file();
    return ini_has_error();
}
