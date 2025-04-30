#ifndef INI_PARSER_H
#define INI_PARSER_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef INI_PARSER_VERSION
#define INI_PARSER_VERSION "0.1"
#endif

#define INI_LINE_MAX 1024
#define INI_BUFFER_SIZE 2048

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

// Apple-specific optimizations
#ifdef __APPLE__
#include <dispatch/dispatch.h> // For GCD (Grand Central Dispatch)
#endif

    typedef enum
    {
        INI_SUCCESS = 0,
        INI_FILE_NOT_FOUND,
        INI_FILE_EMPTY,
        INI_FILE_IS_DIR,
        INI_FILE_OPEN_FAILED,
        INI_FILE_BAD_FORMAT,
        INI_MEMORY_ERROR,
        INI_SECTION_NOT_FOUND,
        INI_KEY_NOT_FOUND,
        INI_INVALID_ARGUMENT,
        INI_PLATFORM_ERROR,
        INI_CLOSE_FAILED,
        INI_PRINT_ERROR,
        INI_FILE_BAD_FORMAT_LINE,
        INI_FILE_BAD_FORMAT_COLUMN
    } ini_error_t;

    typedef struct
    {
        ini_error_t error;
        char const *inipath;
        char const *srcpath; // Path to the C source file
        int ini_line_number;
        int src_line_number;
        char const *custommsg; // Message that can be provided by the developer
    } ini_error_details_t;

    char const *ini_error_to_string(ini_error_t error);
    char const *ini_error_details_to_string(ini_error_details_t error_detailed);

    typedef struct
    {
        char *key;
        char *value;
    } ini_key_value_t;

    typedef struct ini_section_t
    {
        char *name;
        ini_key_value_t *pairs;
        int pair_count;
        struct ini_section_t *subsections;
        int subsection_count;
    } ini_section_t;

    typedef struct
    {
        ini_section_t *sections;
        int section_count;
#ifdef _WIN32
        CRITICAL_SECTION mutex;
#elif defined(__APPLE__)
    dispatch_semaphore_t semaphore; // GCD semaphore (optional)
#else
    pthread_mutex_t mutex;
#endif
    } ini_context_t;

    ini_error_details_t ini_good(char const *filepath);
    ini_error_details_t ini_load(ini_context_t *ctx, char const *filepath);
    ini_context_t *ini_create_context();
    ini_error_details_t ini_free(ini_context_t *ctx);
    ini_error_details_t ini_get_value(ini_context_t const *ctx, char const *section, char const *key, char **value);
    ini_error_details_t ini_print(ini_context_t const *ctx);

#ifdef __cplusplus
}
#endif

#endif // !INI_PARSER_H
