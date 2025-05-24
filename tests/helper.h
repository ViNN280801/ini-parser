#ifndef HELPER_H
#define HELPER_H

#include "ini_os_check.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if INI_OS_WINDOWS
#include <shlobj.h>
#include <windows.h>
#define COLOR_RESET ""
#define COLOR_INFO ""
#define COLOR_SUCCESS ""
#define COLOR_ERROR ""
#define COLOR_WARNING ""
#else
#define COLOR_RESET "\033[0m"
#define COLOR_INFO "\033[34m"
#define COLOR_SUCCESS "\033[32m"
#define COLOR_ERROR "\033[31m"
#define COLOR_WARNING "\033[33m"

#include <sys/stat.h>
#include <unistd.h>
#endif

static int __helper_exit_code = EXIT_SUCCESS;

inline static void __helper_set_exit_code(int code) { __helper_exit_code = code; }
inline static void __helper_reset_exit_code() { __helper_exit_code = EXIT_SUCCESS; }
inline static int __helper_get_exit_code() { return __helper_exit_code; }

static FILE *__helper_lfd = NULL; // log file descriptor
char const *__helper_log_file_name = "test.log";

static void __helper_init_log_file()
{
    __helper_lfd = fopen(__helper_log_file_name, "a");
    if (!__helper_lfd)
    {
        perror("Failed to create log file");
        exit(EXIT_FAILURE);
    }
}

static void __helper_close_log_file()
{
    if (__helper_lfd)
    {
        if (fclose(__helper_lfd) != 0)
            perror("Failed to close log file");
        __helper_lfd = NULL;
    }
}

static void print_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
#if INI_OS_WINDOWS
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    vfprintf_s(__helper_lfd, format, args);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    fprintf(__helper_lfd, "%s[INFO]%s ", COLOR_INFO, COLOR_RESET);
    vfprintf(__helper_lfd, format, args);
#endif
    va_end(args);
}

static void print_success(const char *format, ...)
{
    va_list args;
    va_start(args, format);
#if INI_OS_WINDOWS
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    vfprintf_s(__helper_lfd, format, args);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    fprintf(__helper_lfd, "%s[SUCCESS]%s ", COLOR_SUCCESS, COLOR_RESET);
    vfprintf(__helper_lfd, format, args);
#endif
    va_end(args);
}

static void print_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
#if INI_OS_WINDOWS
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
    vfprintf_s(__helper_lfd, format, args);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    fprintf(__helper_lfd, "%s[ERROR]%s ", COLOR_ERROR, COLOR_RESET);
    vfprintf(__helper_lfd, format, args);
#endif
    va_end(args);
}

static void print_warning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
#if INI_OS_WINDOWS
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    vfprintf_s(__helper_lfd, format, args);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    fprintf(__helper_lfd, "%s[WARNING]%s ", COLOR_WARNING, COLOR_RESET);
    vfprintf(__helper_lfd, format, args);
#endif
    va_end(args);
}

static void create_test_file(const char *filename, const char *content)
{
    if (!filename || !content)
    {
        print_error("NULL filename or content\n");
        return;
    }
    FILE *file = fopen(filename, "w");
    assert(file);
    assert(fprintf(file, "%s", content) >= 0);
    assert(fclose(file) == 0);
}

static void remove_test_file(const char *filename)
{
    if (!filename)
    {
        print_error("NULL filename\n");
        return;
    }
#if INI_OS_WINDOWS
    assert(DeleteFile(filename));
#else
    assert(remove(filename) == 0);
#endif
}

static void create_test_dir(const char *dirname)
{
#if INI_OS_WINDOWS
    assert(CreateDirectory(dirname, NULL));
#else
    assert(mkdir(dirname, 0755) == 0);
#endif
}

static void remove_test_dir(const char *dirname)
{
#if INI_OS_WINDOWS
    SHFILEOPSTRUCTA file_op = {
        NULL,
        FO_DELETE,
        dirname,
        NULL,
        FOF_NOCONFIRMATION | FOF_SILENT,
        FALSE,
        NULL,
        0};
    assert(SHFileOperationA(&file_op) == 0);
#else
    assert(rmdir(dirname) == 0);
#endif
}

#endif // HELPER_H
