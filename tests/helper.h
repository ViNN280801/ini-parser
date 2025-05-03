#ifndef HELPER_H
#define HELPER_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
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

#include <unistd.h>
#endif

static void print_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    vfprintf_s(stdout, format, args);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    fprintf(stdout, "%s[INFO]%s ", COLOR_INFO, COLOR_RESET);
    vfprintf(stdout, format, args);
#endif
    va_end(args);
}

static void print_success(const char *format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    vfprintf_s(stdout, format, args);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    fprintf(stdout, "%s[SUCCESS]%s ", COLOR_SUCCESS, COLOR_RESET);
    vfprintf(stdout, format, args);
#endif
    va_end(args);
}

static void print_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
    vfprintf_s(stderr, format, args);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    fprintf(stderr, "%s[ERROR]%s ", COLOR_ERROR, COLOR_RESET);
    vfprintf(stderr, format, args);
#endif
    va_end(args);
}

static void print_warning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    vfprintf_s(stderr, format, args);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    fprintf(stderr, "%s[WARNING]%s ", COLOR_WARNING, COLOR_RESET);
    vfprintf(stderr, format, args);
#endif
    va_end(args);
}

static void create_test_file(const char *filename, const char *content)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Failed to create test file, do it manually after the test!");
        exit(EXIT_FAILURE);
    }
    if (fprintf(file, "%s", content) < 0)
    {
        perror("Failed to write to test file");
        exit(EXIT_FAILURE);
    }
    if (fclose(file) != 0)
    {
        perror("Failed to close test file, do it manually after the test!");
        exit(EXIT_FAILURE);
    }
}

static void remove_test_file(const char *filename)
{
#ifdef _WIN32
    if (remove(filename) != 0)
    {
        perror("Failed to remove test file, do it manually after the test!");
    }
#else
    if (unlink(filename) != 0)
    {
        perror("Failed to remove test file, do it manually after the test!");
    }
#endif
}

static void create_test_dir(const char *dirname)
{
#ifdef _WIN32
    if (system("mkdir test_dir 2> nul") != 0)
    {
        perror("Failed to create test directory");
        exit(EXIT_FAILURE);
    }
#else
    if (system("mkdir -p test_dir") != 0)
    {
        perror("Failed to create test directory");
        exit(EXIT_FAILURE);
    }
#endif
}

static void remove_test_dir(const char *dirname)
{
#ifdef _WIN32
    if (system("rmdir /Q /S test_dir 2> nul") != 0)
    {
        perror("Failed to remove test directory");
    }
#else
    if (system("rm -rf test_dir") != 0)
    {
        perror("Failed to remove test directory");
    }
#endif
}

#endif // HELPER_H
