#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIPARSER_EXPORTS
#include "iniparser.h"

#if INI_OS_WINDOWS
#include <io.h> // For _access()
#include <windows.h>

#define F_OK 0
#define R_OK 4 // Read permission for _access()
#define PATH_SEPARATOR '\\'

#else // not Windows
#include <sys/stat.h>
#include <unistd.h> // For access()

#define PATH_SEPARATOR '/'
#endif // OS check

// To avoid: warning C4996: 'strdup': The POSIX name for this item is deprecated.
//                                    Instead, use the ISO C and C++ conformant name: _strdup
char *ini_strdup(char const *str)
{
    if (!str)
        return NULL;

    size_t len = strlen(str) + 1; // +1 for null terminator
    char *dup = malloc(len);
    if (!dup)
        return NULL;

    return memcpy(dup, str, len);
}

// Helper function for error reporting
static ini_error_details_t create_error(ini_error_t error, char const *inipath,
                                        int ini_line_number, char const *srcpath,
                                        int src_line_number, char const *custommsg)
{
    return (ini_error_details_t){
        .error = error,
        .inipath = inipath,
        .ini_line_number = ini_line_number,
        .srcpath = srcpath,
        .src_line_number = src_line_number,
        .custommsg = custommsg};
}

ini_error_t __ini_errstack[INI_ERRSTACK_SIZE];

#if INI_OS_WINDOWS
CRITICAL_SECTION __ini_errstack_mutex;
#elif INI_OS_APPLE
dispatch_semaphore_t __ini_errstack_semaphore;
#else
pthread_mutex_t __ini_errstack_mutex;
#endif

INIPARSER_API void __ini_init_errstack()
{
#if INI_OS_WINDOWS
    if (!InitializeCriticalSectionAndSpinCount(&__ini_errstack_mutex, 0x400)) // 0x400 is the default spin count
    {
        // Failed to initialize critical section
        fprintf(stderr, "Failed to initialize error stack mutex\n");
        exit(EXIT_FAILURE);
    }
#elif INI_OS_APPLE
    __ini_errstack_semaphore = dispatch_semaphore_create(1);
    if (!__ini_errstack_semaphore)
    {
        fprintf(stderr, "Failed to create error stack semaphore\n");
        exit(EXIT_FAILURE);
    }
#else
    if (pthread_mutex_init(&__ini_errstack_mutex, NULL) != 0)
    {
        fprintf(stderr, "Failed to initialize error stack mutex\n");
        exit(EXIT_FAILURE);
    }
#endif
    memset(__ini_errstack, INI_SUCCESS, INI_ERRSTACK_SIZE * sizeof(ini_error_t));
}

INIPARSER_API void __ini_finalize_errstack()
{
    __ini_clear_errstack();
#if INI_OS_WINDOWS
    DeleteCriticalSection(&__ini_errstack_mutex);
#elif INI_OS_APPLE
    if (__ini_errstack_semaphore)
    {
        dispatch_release(__ini_errstack_semaphore);
        __ini_errstack_semaphore = NULL;
    }
#else
    pthread_mutex_destroy(&__ini_errstack_mutex);
#endif
}

INIPARSER_API void __ini_clear_errstack()
{
    // Lock the mutex
#if INI_OS_WINDOWS
    EnterCriticalSection(&__ini_errstack_mutex);
#elif INI_OS_APPLE
    dispatch_semaphore_wait(__ini_errstack_semaphore, DISPATCH_TIME_FOREVER);
#else
    pthread_mutex_lock(&__ini_errstack_mutex);
#endif

    // ================================== //
    // ----> Critical section ----------- //
    // ================================== //
    for (int i = 0; i < INI_ERRSTACK_SIZE; i++)
        __ini_errstack[i] = INI_SUCCESS;
    // ================================== //
    // ---- End of critical section <---- //
    // ================================== //

    // Unlock the mutex
#if INI_OS_WINDOWS
    LeaveCriticalSection(&__ini_errstack_mutex);
#elif INI_OS_APPLE
    dispatch_semaphore_signal(__ini_errstack_semaphore);
#else
    pthread_mutex_unlock(&__ini_errstack_mutex);
#endif
}

INIPARSER_API void __ini_add_in_errstack(ini_error_t error)
{
// Lock the mutex
#if INI_OS_WINDOWS
    EnterCriticalSection(&__ini_errstack_mutex);
#elif INI_OS_APPLE
    dispatch_semaphore_wait(__ini_errstack_semaphore, DISPATCH_TIME_FOREVER);
#else
    pthread_mutex_lock(&__ini_errstack_mutex);
#endif

    // ================================== //
    // ----> Critical section ----------- //
    // ================================== //
    for (int i = 0; i < INI_ERRSTACK_SIZE; i++)
    {
        if (__ini_errstack[i] == INI_SUCCESS)
        {
            __ini_errstack[i] = error;
            break;
        }
    }
    // ================================== //
    // ---- End of critical section <---- //
    // ================================== //

// Unlock the mutex
#if INI_OS_WINDOWS
    LeaveCriticalSection(&__ini_errstack_mutex);
#elif INI_OS_APPLE
    dispatch_semaphore_signal(__ini_errstack_semaphore);
#else
    pthread_mutex_unlock(&__ini_errstack_mutex);
#endif
}

INIPARSER_API int __ini_has_in_errstack(ini_error_t error)
{
    for (int i = 0; i < INI_ERRSTACK_SIZE; i++)
        if (__ini_errstack[i] == error)
            return 1;
    return 0;
}

INIPARSER_API int ini_has_error()
{
    for (int i = 0; i < INI_ERRSTACK_SIZE; i++)
        if (__ini_errstack[i] != INI_SUCCESS)
            return 1;
    return 0;
}

INIPARSER_API char const *ini_error_to_string(ini_error_t error)
{
    switch (error)
    {
    case INI_SUCCESS:
        return "Success";
    case INI_FILE_NOT_FOUND:
        return "Ini file not found";
    case INI_FILE_EMPTY:
        return "Ini file is empty";
    case INI_FILE_IS_DIR:
        return "Ini file is a directory";
    case INI_FILE_OPEN_FAILED:
        return "Ini file open failed";
    case INI_FILE_BAD_FORMAT:
        return "Ini file bad format";
    case INI_MEMORY_ERROR:
        return "Ini memory error";
    case INI_SECTION_NOT_FOUND:
        return "Ini section not found";
    case INI_KEY_NOT_FOUND:
        return "Ini key not found";
    case INI_INVALID_ARGUMENT:
        return "Ini invalid argument";
    case INI_PLATFORM_ERROR:
        return "Ini platform error";
    case INI_CLOSE_FAILED:
        return "Ini close failed";
    default:
        return "Unknown error";
    }
}

INIPARSER_API char const *ini_error_details_to_string(ini_error_details_t error_detailed)
{
    static char buffer[INI_BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "code: %d - (%s)\nINI:\n\tFile: %s\n\tLine: %d\nSRC:\n\tFile: %s\n\tLine: %d\nMessage: %s",
             (int)error_detailed.error,
             ini_error_to_string(error_detailed.error),
             error_detailed.inipath ? error_detailed.inipath : "NULL",
             error_detailed.ini_line_number,
             error_detailed.srcpath ? error_detailed.srcpath : "NULL",
             error_detailed.src_line_number,
             error_detailed.custommsg);
    return buffer;
}

INIPARSER_API void ini_initialize()
{
    // Already initialized
    if (__ini_is_initialized == 1)
        return;
    __ini_init_errstack();
    __ini_is_initialized = 1;
}

INIPARSER_API void ini_finalize()
{
    // Already finalized
    if (__ini_is_initialized == 0)
        return;
    __ini_finalize_errstack();
    __ini_is_initialized = 0;
}

INIPARSER_API int __ini_is_initialized = 0;

INIPARSER_API int ini_is_initialized() { return __ini_is_initialized; }

INIPARSER_API ini_error_details_t ini_good(char const *filepath)
{
    if (!filepath)
    {
        __ini_add_in_errstack(INI_INVALID_ARGUMENT);
        return create_error(
            INI_INVALID_ARGUMENT,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Filepath is emtpy");
    }

#if INI_OS_WINDOWS
    // Windows-specific implementation
    DWORD attrs = GetFileAttributes(filepath);
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        __ini_add_in_errstack(INI_FILE_NOT_FOUND);
        return create_error(
            INI_FILE_NOT_FOUND,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "File does not exist or is not accessible");
    }

    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    {
        __ini_add_in_errstack(INI_FILE_IS_DIR);
        return create_error(
            INI_FILE_IS_DIR,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Path is a directory");
    }
#else
    // POSIX implementation (Linux, macOS)
    struct stat statbuf;
    if (stat(filepath, &statbuf) != 0)
    {
        __ini_add_in_errstack(INI_FILE_NOT_FOUND);
        return create_error(
            INI_FILE_NOT_FOUND,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "File does not exist or is not accessible");
    }

    // Check if it's a regular file
    if (!S_ISREG(statbuf.st_mode))
    {
        if (S_ISDIR(statbuf.st_mode))
        {
            __ini_add_in_errstack(INI_FILE_IS_DIR);
            return create_error(
                INI_FILE_IS_DIR,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Path is a directory");
        }
        else
        {
            __ini_add_in_errstack(INI_FILE_BAD_FORMAT);
            return create_error(
                INI_FILE_BAD_FORMAT,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Not a regular file");
        }
    }

    // Check if the file is empty
    if (statbuf.st_size == 0)
    {
        __ini_add_in_errstack(INI_FILE_EMPTY);
        return create_error(
            INI_FILE_EMPTY,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "File is empty");
    }
#endif

    // Open file
    FILE *file;
#if INI_OS_WINDOWS
    if (fopen_s(&file, filepath, "r") != 0)
    {
        __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to open file");
    }

    // Check if file is empty (Windows-specific)
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    if (file_size == 0)
    {
        if (fclose(file) != 0)
        {
            __ini_add_in_errstack(INI_CLOSE_FAILED);
            return create_error(
                INI_CLOSE_FAILED,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to close file");
        }

        __ini_add_in_errstack(INI_FILE_EMPTY);
        return create_error(
            INI_FILE_EMPTY,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "File is empty");
    }
#else
    file = fopen(filepath, "r");
    if (!file)
    {
        __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to open file");
    }
#endif

    // Check for UTF-8 BOM (Byte Order Mark: EF BB BF)
    unsigned char bom[3];
    size_t bytes_read = fread(bom, 1, 3, file);

    // If we don't have a BOM, rewind to the beginning
    if (bytes_read < 3 ||
        bom[0] != 0xEF ||
        bom[1] != 0xBB ||
        bom[2] != 0xBF)
    {
        rewind(file);
    }

    // Now validate file format
    char line[INI_LINE_MAX];
    int line_num = 0;
    int in_section = 0;

    while (fgets(line, sizeof(line), file))
    {
        line_num++;

        // Check for line too long
        size_t len = strlen(line);
        if (len >= INI_LINE_MAX - 1 && line[len - 1] != '\n')
        {
            if (fclose(file) != 0)
            {
                __ini_add_in_errstack(INI_CLOSE_FAILED);
                return create_error(
                    INI_CLOSE_FAILED,
                    filepath,
                    line_num,
                    __FILE__,
                    __LINE__,
                    "Failed to close file");
            }

            __ini_add_in_errstack(INI_FILE_BAD_FORMAT);
            return create_error(
                INI_FILE_BAD_FORMAT,
                filepath,
                line_num,
                __FILE__,
                __LINE__,
                "Line too long");
        }

        // Trim leading whitespace
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t')
            trimmed++;

        // Skip empty lines and comments
        if (*trimmed == '\0' || *trimmed == '\n' || *trimmed == '\r' || *trimmed == ';' || *trimmed == '#')
            continue;

        // Check for section
        if (*trimmed == '[')
        {
            char *end = strchr(trimmed, ']');
            if (!end)
            {
                if (fclose(file) != 0)
                {
                    __ini_add_in_errstack(INI_CLOSE_FAILED);
                    return create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        line_num,
                        __FILE__,
                        __LINE__,
                        "Failed to close file");
                }

                __ini_add_in_errstack(INI_FILE_BAD_FORMAT);
                return create_error(
                    INI_FILE_BAD_FORMAT,
                    filepath,
                    line_num,
                    __FILE__,
                    __LINE__,
                    "Missing closing bracket");
            }

            in_section = 1;
        }
        // Check for key-value pair
        else if (in_section && strchr(trimmed, '=') != NULL)
        {
            char *eq = strchr(trimmed, '=');
            if (eq == trimmed)
            {
                // Only reject empty keys, but accept empty values
                if (fclose(file) != 0)
                {
                    __ini_add_in_errstack(INI_CLOSE_FAILED);
                    return create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        line_num,
                        __FILE__,
                        __LINE__,
                        "Failed to close file after validation");
                }

                __ini_add_in_errstack(INI_FILE_BAD_FORMAT);
                return create_error(
                    INI_FILE_BAD_FORMAT,
                    filepath,
                    line_num,
                    __FILE__,
                    __LINE__,
                    "Empty key");
            }

            // Check for unbalanced quotes
            char *value = eq + 1;
            // Skip leading whitespace
            while (*value == ' ' || *value == '\t')
                value++;

            if (*value == '"')
            {
                // Find closing quote - but it must be at the end of the value or followed by whitespace/comment
                char *closing_quote = strchr(value + 1, '"');
                if (!closing_quote ||
                    (*(closing_quote + 1) != '\0' &&
                     *(closing_quote + 1) != '\n' &&
                     *(closing_quote + 1) != '\r' &&
                     *(closing_quote + 1) != ' ' &&
                     *(closing_quote + 1) != '\t' &&
                     *(closing_quote + 1) != ';' &&
                     *(closing_quote + 1) != '#'))
                {
                    if (fclose(file) != 0)
                    {
                        __ini_add_in_errstack(INI_CLOSE_FAILED);
                        return create_error(
                            INI_CLOSE_FAILED,
                            filepath,
                            line_num,
                            __FILE__,
                            __LINE__,
                            "Failed to close file after validation");
                    }

                    __ini_add_in_errstack(INI_FILE_BAD_FORMAT);
                    return create_error(
                        INI_FILE_BAD_FORMAT,
                        filepath,
                        line_num,
                        __FILE__,
                        __LINE__,
                        "Unbalanced quotes");
                }
            }

            // Check for arrays (not supported)
            if (strchr(value, ',') != NULL)
            {
                if (fclose(file) != 0)
                {
                    __ini_add_in_errstack(INI_CLOSE_FAILED);
                    return create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        line_num,
                        __FILE__,
                        __LINE__,
                        "Failed to close file after validation");
                }

                __ini_add_in_errstack(INI_FILE_BAD_FORMAT);
                return create_error(
                    INI_FILE_BAD_FORMAT,
                    filepath,
                    line_num,
                    __FILE__,
                    __LINE__,
                    "Arrays not supported");
            }
        }
        else
        {
            if (fclose(file) != 0)
            {
                __ini_add_in_errstack(INI_CLOSE_FAILED);
                return create_error(
                    INI_CLOSE_FAILED,
                    filepath,
                    line_num,
                    __FILE__,
                    __LINE__,
                    "Failed to close file after validation");
            }

            __ini_add_in_errstack(INI_FILE_BAD_FORMAT);
            return create_error(
                INI_FILE_BAD_FORMAT,
                filepath,
                line_num,
                __FILE__,
                __LINE__,
                "Invalid line format");
        }
    }

    if (fclose(file) != 0)
    {
        __ini_add_in_errstack(INI_CLOSE_FAILED);
        return create_error(
            INI_CLOSE_FAILED,
            filepath,
            line_num,
            __FILE__,
            __LINE__,
            "Failed to close file after validation");
    }

    __ini_clear_errstack();
    return create_error(
        INI_SUCCESS,
        filepath,
        line_num,
        __FILE__,
        __LINE__,
        "File format is valid");
}

INIPARSER_API ini_error_details_t ini_load(ini_context_t *ctx, char const *filepath)
{
    if (!filepath)
    {
        __ini_add_in_errstack(INI_INVALID_ARGUMENT);
        return create_error(
            INI_INVALID_ARGUMENT,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Filepath is empty");
    }

    ini_context_t *ctx_to_use = NULL;
    int need_to_free_ctx_on_error = 0; // Flag to track if we need to free the context on error

    // If no context was provided, create a new one
    if (!ctx)
    {
        ctx_to_use = ini_create_context();
        if (!ctx_to_use)
        {
            __ini_add_in_errstack(INI_MEMORY_ERROR);
            return create_error(
                INI_MEMORY_ERROR,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to create context");
        }
        need_to_free_ctx_on_error = 1;
    }
    else
    {
        // Use the provided context but clear/reset it first
        ctx_to_use = ctx;
        // Instead of freeing and potentially causing use-after-free,
        // we'll manually reset the context by freeing its internals
        if (ctx_to_use->sections)
        {
            for (int i = 0; i < ctx_to_use->section_count; i++)
            {
                ini_section_t *section = &ctx_to_use->sections[i];

                // Free section name
                if (section->name)
                {
                    free(section->name);
                    section->name = NULL;
                }

                // Free key-value pairs
                if (section->pairs)
                {
                    for (int j = 0; j < section->pair_count; j++)
                    {
                        if (section->pairs[j].key)
                        {
                            free(section->pairs[j].key);
                            section->pairs[j].key = NULL;
                        }
                        if (section->pairs[j].value)
                        {
                            free(section->pairs[j].value);
                            section->pairs[j].value = NULL;
                        }
                    }
                    free(section->pairs);
                    section->pairs = NULL;
                }
                section->pair_count = 0;

                // Free subsections
                if (section->subsections)
                {
                    for (int j = 0; j < section->subsection_count; j++)
                    {
                        ini_section_t *subsection = &section->subsections[j];

                        if (subsection->name)
                        {
                            free(subsection->name);
                            subsection->name = NULL;
                        }

                        if (subsection->pairs)
                        {
                            for (int k = 0; k < subsection->pair_count; k++)
                            {
                                if (subsection->pairs[k].key)
                                {
                                    free(subsection->pairs[k].key);
                                    subsection->pairs[k].key = NULL;
                                }
                                if (subsection->pairs[k].value)
                                {
                                    free(subsection->pairs[k].value);
                                    subsection->pairs[k].value = NULL;
                                }
                            }
                            free(subsection->pairs);
                            subsection->pairs = NULL;
                        }
                        subsection->pair_count = 0;
                        // Subsections don't have more subsections
                    }
                    free(section->subsections);
                    section->subsections = NULL;
                }
                section->subsection_count = 0;
            }
            free(ctx_to_use->sections);
            ctx_to_use->sections = NULL;
        }
        ctx_to_use->section_count = 0;
    }

    // Setup for error handling
    ini_error_details_t err = create_error(INI_SUCCESS, filepath, 0, __FILE__, __LINE__, "");

    // Validate the file first
    err = ini_good(filepath);
    if (err.error != INI_SUCCESS)
    {
        if (need_to_free_ctx_on_error)
        {
            ini_free(ctx_to_use);
        }
        return err;
    }

    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        if (need_to_free_ctx_on_error)
        {
            ini_free(ctx_to_use);
        }

        __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to open file");
    }

    // Check for UTF-8 BOM (Byte Order Mark: EF BB BF)
    unsigned char bom[3];
    size_t bytes_read = fread(bom, 1, 3, file);

    // If we don't have a BOM, rewind to the beginning
    if (bytes_read < 3 ||
        bom[0] != 0xEF ||
        bom[1] != 0xBB ||
        bom[2] != 0xBF)
    {
        rewind(file);
    }

    // Parsing ini file
    char line[INI_LINE_MAX];
    ini_section_t *current_section = NULL;

    // Lock the context for thread safety
    err = __INI_MUTEX_LOCK(ctx_to_use);
    if (err.error != INI_SUCCESS)
    {
        if (fclose(file) != 0)
        {
            __ini_add_in_errstack(INI_CLOSE_FAILED);
        }
        if (need_to_free_ctx_on_error)
        {
            ini_free(ctx_to_use);
        }
        return err;
    }

    // Parsing ini file
    while (fgets(line, sizeof(line), file))
    {
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t')
            trimmed++;

        // Skip empty lines and comments
        if (*trimmed == '\0' || *trimmed == '\n' || *trimmed == ';' || *trimmed == '#')
            continue;

        // Handle section
        if (*trimmed == '[')
        {
            char *end = strchr(trimmed, ']');
            if (!end)
            {
                if (fclose(file) != 0)
                {
                    __ini_add_in_errstack(INI_CLOSE_FAILED);
                    err = create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to close file");
                }
                else
                {
                    __ini_add_in_errstack(INI_FILE_BAD_FORMAT);
                    err = create_error(
                        INI_FILE_BAD_FORMAT,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Missing closing bracket");
                }
                __INI_MUTEX_UNLOCK(ctx_to_use);
                if (need_to_free_ctx_on_error)
                    ini_free(ctx_to_use);
                return err;
            }
            *end = '\0';
            char *name = trimmed + 1;

            // Check for subsection (e.g., "database.backup")
            char *dot = strchr(name, '.');
            if (dot)
            {
                *dot = '\0';
                char *subsection_name = dot + 1;

                // Find parent section
                ini_section_t *parent = NULL;
                for (int i = 0; i < ctx_to_use->section_count; i++)
                {
                    if (strcmp(ctx_to_use->sections[i].name, name) == 0)
                    {
                        parent = &ctx_to_use->sections[i];
                        break;
                    }
                }
                if (!parent)
                {
                    if (fclose(file) != 0)
                    {
                        __ini_add_in_errstack(INI_CLOSE_FAILED);
                        err = create_error(
                            INI_CLOSE_FAILED,
                            filepath,
                            0,
                            __FILE__,
                            __LINE__,
                            "Failed to close file");
                    }
                    else
                    {
                        __ini_add_in_errstack(INI_SECTION_NOT_FOUND);
                        err = create_error(
                            INI_SECTION_NOT_FOUND,
                            filepath,
                            0,
                            __FILE__,
                            __LINE__,
                            "Parent section not found");
                    }
                    __INI_MUTEX_UNLOCK(ctx_to_use);
                    if (need_to_free_ctx_on_error)
                        ini_free(ctx_to_use);
                    return err;
                }

                // Add subsection
                parent->subsections = realloc(parent->subsections, (parent->subsection_count + 1) * sizeof(ini_section_t));
                if (!parent->subsections)
                {
                    if (fclose(file) != 0)
                    {
                        __ini_add_in_errstack(INI_CLOSE_FAILED);
                        err = create_error(
                            INI_CLOSE_FAILED,
                            filepath,
                            0,
                            __FILE__,
                            __LINE__,
                            "Failed to close file");
                    }
                    else
                    {
                        __ini_add_in_errstack(INI_MEMORY_ERROR);
                        err = create_error(
                            INI_MEMORY_ERROR,
                            filepath,
                            0,
                            __FILE__,
                            __LINE__,
                            "Failed to allocate memory");
                    }
                    __INI_MUTEX_UNLOCK(ctx_to_use);
                    if (need_to_free_ctx_on_error)
                        ini_free(ctx_to_use);
                    return err;
                }
                current_section = &parent->subsections[parent->subsection_count++];
                current_section->name = ini_strdup(subsection_name);
                current_section->pairs = NULL;
                current_section->pair_count = 0;
                current_section->subsections = NULL;
                current_section->subsection_count = 0;
            }
            else
            {
                // Add top-level section
                ctx_to_use->sections = realloc(ctx_to_use->sections, (ctx_to_use->section_count + 1) * sizeof(ini_section_t));
                if (!ctx_to_use->sections)
                {
                    if (fclose(file) != 0)
                    {
                        __ini_add_in_errstack(INI_CLOSE_FAILED);
                        err = create_error(
                            INI_CLOSE_FAILED,
                            filepath,
                            0,
                            __FILE__,
                            __LINE__,
                            "Failed to close file");
                    }
                    else
                    {
                        __ini_add_in_errstack(INI_MEMORY_ERROR);
                        err = create_error(
                            INI_MEMORY_ERROR,
                            filepath,
                            0,
                            __FILE__,
                            __LINE__,
                            "Failed to allocate memory");
                    }
                    __INI_MUTEX_UNLOCK(ctx_to_use);
                    if (need_to_free_ctx_on_error)
                        ini_free(ctx_to_use);
                    return err;
                }
                current_section = &ctx_to_use->sections[ctx_to_use->section_count++];
                current_section->name = ini_strdup(name);
                current_section->pairs = NULL;
                current_section->pair_count = 0;
                current_section->subsections = NULL;
                current_section->subsection_count = 0;
            }
        }
        // Handle key-value pair
        else if (current_section && strchr(trimmed, '='))
        {
            char *eq = strchr(trimmed, '=');
            if (!eq)
            {
                if (fclose(file) != 0)
                {
                    __ini_add_in_errstack(INI_CLOSE_FAILED);
                }
                else
                {
                    __ini_add_in_errstack(INI_FILE_BAD_FORMAT);
                }
                __INI_MUTEX_UNLOCK(ctx_to_use);
                if (need_to_free_ctx_on_error)
                    ini_free(ctx_to_use);
                return create_error(
                    INI_FILE_BAD_FORMAT,
                    filepath,
                    0,
                    __FILE__,
                    __LINE__,
                    "Missing equals sign in key-value pair");
            }

            // Split into key and value
            *eq = '\0';
            char *key = trimmed;
            char *value = eq + 1;

            // Trim leading whitespace from key
            char *key_end = eq - 1;
            while (key_end >= key && (*key_end == ' ' || *key_end == '\t'))
                *key_end-- = '\0';

            // Handle quoted values ("value")
            if (value[0] == '"' && value[strlen(value) - 1] == '"')
            {
                // Remove the quotes
                value++;
                value[strlen(value) - 1] = '\0';
            }
            else
            {
                // Trim leading and trailing whitespace from value only if not quoted
                char *val_start = value;
                while (*val_start && (*val_start == ' ' || *val_start == '\t'))
                    val_start++;

                char *val_end = value + strlen(value) - 1;
                while (val_end > val_start && (*val_end == ' ' || *val_end == '\t' || *val_end == '\n' || *val_end == '\r'))
                    *val_end-- = '\0';

                // Move the value to start of string if needed
                if (val_start > value)
                    memmove(value, val_start, strlen(val_start) + 1);
            }

            // Trim trailing newlines and carriage returns from all values
            char *value_end = value + strlen(value) - 1;
            while (value_end > value && (*value_end == '\n' || *value_end == '\r'))
                *value_end-- = '\0';

            // Check if key is empty after trimming
            if (*key == '\0')
            {
                if (fclose(file) != 0)
                {
                    __ini_add_in_errstack(INI_CLOSE_FAILED);
                }
                else
                {
                    __ini_add_in_errstack(INI_FILE_BAD_FORMAT);
                }
                __INI_MUTEX_UNLOCK(ctx_to_use);
                if (need_to_free_ctx_on_error)
                    ini_free(ctx_to_use);
                return create_error(
                    INI_FILE_BAD_FORMAT,
                    filepath,
                    0,
                    __FILE__,
                    __LINE__,
                    "Empty key in key-value pair");
            }

            // Handle truly empty values after trimming
            if (*value == '\0' || *value == '\n' || *value == '\r')
            {
                value = "";
            }

            // Remove quotes if present
            if (*value == '"' && *value_end == '"')
            {
                *value_end = '\0';
                value++;
            }

            // Add key-value pair
            current_section->pairs = realloc(current_section->pairs, (current_section->pair_count + 1) * sizeof(ini_key_value_t));
            if (!current_section->pairs)
            {
                if (fclose(file) != 0)
                {
                    __ini_add_in_errstack(INI_CLOSE_FAILED);
                    err = create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to close file");
                }
                else
                {
                    __ini_add_in_errstack(INI_MEMORY_ERROR);
                    err = create_error(
                        INI_MEMORY_ERROR,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to allocate memory");
                }
                __INI_MUTEX_UNLOCK(ctx_to_use);
                if (need_to_free_ctx_on_error)
                    ini_free(ctx_to_use);
                return err;
            }
            current_section->pairs[current_section->pair_count].key = ini_strdup(key);

            // For empty values, store an empty string
            if (value && *value == '\0')
            {
                current_section->pairs[current_section->pair_count].value = ini_strdup("");
            }
            else
            {
                current_section->pairs[current_section->pair_count].value = ini_strdup(value);
            }

            current_section->pair_count++;
        }
    }

    // Close file
    if (fclose(file) != 0)
    {
        __ini_add_in_errstack(INI_CLOSE_FAILED);
        err = create_error(
            INI_CLOSE_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to close file after validation");
        __INI_MUTEX_UNLOCK(ctx_to_use);
        if (need_to_free_ctx_on_error)
            ini_free(ctx_to_use);
        return err;
    }

    // Unlock context
    err = __INI_MUTEX_UNLOCK(ctx_to_use);
    if (err.error != INI_SUCCESS)
    {
        if (need_to_free_ctx_on_error)
            ini_free(ctx_to_use);
        return err;
    }

    // If we needed to create a new context and we're returning it via pointer
    if (need_to_free_ctx_on_error && ctx)
    {
        // Copy newly created context's contents to caller's ctx pointer
        *ctx = *ctx_to_use;

        // Free only the container, not the contents
        free(ctx_to_use);
    }

    __ini_clear_errstack();
    return create_error(
        INI_SUCCESS,
        filepath,
        0,
        __FILE__,
        __LINE__,
        "File loaded successfully");
}

INIPARSER_API ini_context_t *ini_create_context()
{
    ini_context_t *ctx = (ini_context_t *)malloc(sizeof(ini_context_t));
    if (!ctx)
    {
        __ini_add_in_errstack(INI_MEMORY_ERROR);
        return NULL;
    }

    // Initialize all members to ensure a clean state
    ctx->sections = NULL;
    ctx->section_count = 0;

    // Platform-specific initialization of the mutex/semaphore
#if INI_OS_WINDOWS
    InitializeCriticalSection(&ctx->mutex);
#elif INI_OS_APPLE
    ctx->semaphore = dispatch_semaphore_create(1); // Binary semaphore for mutual exclusion
    if (!ctx->semaphore)
    {
        free(ctx);
        __ini_add_in_errstack(INI_PLATFORM_ERROR);
        return NULL;
    }
#else
    if (pthread_mutex_init(&ctx->mutex, NULL) != 0)
    {
        free(ctx);
        __ini_add_in_errstack(INI_PLATFORM_ERROR);
        return NULL;
    }
#endif

    return ctx;
}

INIPARSER_API ini_error_details_t ini_free(ini_context_t *ctx)
{
    if (!ctx)
    {
        __ini_add_in_errstack(INI_INVALID_ARGUMENT);
        return create_error(
            INI_INVALID_ARGUMENT,
            NULL,
            0,
            __FILE__,
            __LINE__,
            "Invalid arguments: NULL context, nothing to free");
    }

    // Free all sections and their resources
    if (ctx->sections)
    {
        for (int i = 0; i < ctx->section_count; i++)
        {
            ini_section_t *section = &ctx->sections[i];

            // Free all key-value pairs in this section
            if (section->pairs)
            {
                for (int j = 0; j < section->pair_count; j++)
                {
                    if (section->pairs[j].key)
                    {
                        free(section->pairs[j].key);
                        section->pairs[j].key = NULL;
                    }

                    if (section->pairs[j].value)
                    {
                        free(section->pairs[j].value);
                        section->pairs[j].value = NULL;
                    }
                }
                free(section->pairs);
                section->pairs = NULL;
            }

            // Free all subsections and their resources
            if (section->subsections)
            {
                for (int k = 0; k < section->subsection_count; k++)
                {
                    ini_section_t *subsection = &section->subsections[k];

                    // Free all key-value pairs in this subsection
                    if (subsection->pairs)
                    {
                        for (int l = 0; l < subsection->pair_count; l++)
                        {
                            if (subsection->pairs[l].key)
                            {
                                free(subsection->pairs[l].key);
                                subsection->pairs[l].key = NULL;
                            }

                            if (subsection->pairs[l].value)
                            {
                                free(subsection->pairs[l].value);
                                subsection->pairs[l].value = NULL;
                            }
                        }
                        free(subsection->pairs);
                        subsection->pairs = NULL;
                    }

                    // Free subsection name
                    if (subsection->name)
                    {
                        free(subsection->name);
                        subsection->name = NULL;
                    }

                    // We shouldn't need to free further nested subsections as the structure doesn't support them
                    if (subsection->subsections)
                    {
                        free(subsection->subsections);
                        subsection->subsections = NULL;
                    }
                }
                free(section->subsections);
                section->subsections = NULL;
            }

            // Free section name
            if (section->name)
            {
                free(section->name);
                section->name = NULL;
            }
        }
        free(ctx->sections);
        ctx->sections = NULL;
        ctx->section_count = 0;
    }

    // Destroy the mutex/semaphore
#if INI_OS_WINDOWS
    DeleteCriticalSection(&ctx->mutex);
#elif INI_OS_APPLE
    if (ctx->semaphore)
    {
        dispatch_release(ctx->semaphore);
        ctx->semaphore = NULL;
    }
#else
    pthread_mutex_destroy(&ctx->mutex);
#endif

    // Free the context itself
    free(ctx);

    return create_error(
        INI_SUCCESS,
        NULL,
        0,
        __FILE__,
        __LINE__,
        "Context freed successfully");
}

INIPARSER_API ini_error_details_t ini_get_value(const ini_context_t *ctx, const char *section, const char *key, char **value)
{
    if (!ctx || !section || !key || !value)
    {
        __ini_add_in_errstack(INI_INVALID_ARGUMENT);
        return create_error(
            INI_INVALID_ARGUMENT,
            NULL,
            0,
            __FILE__,
            __LINE__,
            "One or more arguments are NULL");
    }

    // Lock the context
    __INI_MUTEX_LOCK((ini_context_t *)ctx);

    // Free previous value if it exists
    if (*value)
    {
        free(*value);
        *value = NULL;
    }

    // Find the section
    const ini_section_t *found_section = __ini_find_section(ctx, section);
    if (!found_section)
    {
        __ini_add_in_errstack(INI_SECTION_NOT_FOUND);
        __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
        return create_error(
            INI_SECTION_NOT_FOUND,
            NULL,
            0,
            __FILE__,
            __LINE__,
            "Section not found");
    }

    // Find the key
    const ini_key_value_t *found_pair = __ini_find_pair(found_section, key);
    if (!found_pair)
    {
        __ini_add_in_errstack(INI_KEY_NOT_FOUND);
        __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
        return create_error(
            INI_KEY_NOT_FOUND,
            NULL,
            0,
            __FILE__,
            __LINE__,
            "Key not found in section");
    }

    // Copy the value
    if (found_pair->value)
    {
        *value = strdup(found_pair->value);
        if (!*value)
        {
            __ini_add_in_errstack(INI_MEMORY_ERROR);
            __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
            return create_error(
                INI_MEMORY_ERROR,
                NULL,
                0,
                __FILE__,
                __LINE__,
                "Failed to allocate memory for value");
        }
    }
    else
    {
        *value = strdup("");
        if (!*value)
        {
            __ini_add_in_errstack(INI_MEMORY_ERROR);
            __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
            return create_error(
                INI_MEMORY_ERROR,
                NULL,
                0,
                __FILE__,
                __LINE__,
                "Failed to allocate memory for empty value");
        }
    }

    // Unlock the context
    __INI_MUTEX_UNLOCK((ini_context_t *)ctx);

    return create_error(
        INI_SUCCESS,
        NULL,
        0,
        __FILE__,
        __LINE__,
        "Value retrieved successfully");
}

INIPARSER_API ini_error_details_t ini_print(FILE *stream, ini_context_t const *ctx)
{
    if (!ctx)
    {
        if (fprintf(stream, "Error: NULL context\n") < 0)
        {
            __ini_add_in_errstack(INI_PRINT_ERROR);
            return create_error(
                INI_PRINT_ERROR,
                NULL,
                0,
                __FILE__,
                __LINE__,
                "Failed to print error");
        }

        __ini_add_in_errstack(INI_INVALID_ARGUMENT);
        return create_error(
            INI_INVALID_ARGUMENT,
            NULL,
            0,
            __FILE__,
            __LINE__,
            "Failed to print error");
    }

    for (int i = 0; i < ctx->section_count; i++)
    {
        ini_section_t const *section = &ctx->sections[i];
        if (!section || !section->name)
            continue; // Skip invalid sections

        if (fprintf(stream, "[%s]\n", section->name) < 0)
        {
            __ini_add_in_errstack(INI_PRINT_ERROR);
            return create_error(
                INI_PRINT_ERROR,
                NULL,
                0,
                __FILE__,
                __LINE__,
                "Failed to print error");
        }

        // Print key-value pairs
        for (int j = 0; j < section->pair_count; j++)
        {
            if (!section->pairs || !section->pairs[j].key || !section->pairs[j].value)
                continue; // Skip invalid pairs

            if (fprintf(stream, "%s = %s\n", section->pairs[j].key, section->pairs[j].value) < 0)
            {
                __ini_add_in_errstack(INI_PRINT_ERROR);
                return create_error(
                    INI_PRINT_ERROR,
                    NULL,
                    0,
                    __FILE__,
                    __LINE__,
                    "Failed to print error");
            }
        }

        // Print subsections
        for (int k = 0; k < section->subsection_count; k++)
        {
            ini_section_t const *subsection = &section->subsections[k];
            if (!subsection || !subsection->name)
                continue; // Skip invalid subsections

            if (fprintf(stream, "\n[%s.%s]\n", section->name, subsection->name) < 0)
            {
                __ini_add_in_errstack(INI_PRINT_ERROR);
                return create_error(
                    INI_PRINT_ERROR,
                    NULL,
                    0,
                    __FILE__,
                    __LINE__,
                    "Failed to print error");
            }

            // Print subsection key-value pairs
            for (int l = 0; l < subsection->pair_count; l++)
            {
                if (!subsection->pairs || !subsection->pairs[l].key || !subsection->pairs[l].value)
                    continue; // Skip invalid pairs

                if (fprintf(stream, "%s = %s\n", subsection->pairs[l].key, subsection->pairs[l].value) < 0)
                {
                    __ini_add_in_errstack(INI_PRINT_ERROR);
                    return create_error(
                        INI_PRINT_ERROR,
                        NULL,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to print error");
                }
            }
        }

        if (fprintf(stream, "\n") < 0)
        {
            __ini_add_in_errstack(INI_PRINT_ERROR);
            return create_error(
                INI_PRINT_ERROR,
                NULL,
                0,
                __FILE__,
                __LINE__,
                "Failed to print error");
        }
    }

    return create_error(
        INI_SUCCESS,
        NULL,
        0,
        __FILE__,
        __LINE__,
        "File loaded successfully");
}

INIPARSER_API ini_error_details_t __INI_MUTEX_LOCK(ini_context_t *ctx)
{
    if (!ctx)
    {
        __ini_add_in_errstack(INI_INVALID_ARGUMENT);
        return create_error(INI_INVALID_ARGUMENT, NULL, 0, __FILE__, __LINE__, "Context is NULL");
    }

#if INI_OS_WINDOWS
    EnterCriticalSection(&ctx->mutex);
#elif INI_OS_APPLE
    dispatch_semaphore_wait(ctx->semaphore, DISPATCH_TIME_FOREVER);
#else
    if (pthread_mutex_lock(&ctx->mutex) != 0)
    {
        __ini_add_in_errstack(INI_PLATFORM_ERROR);
        return create_error(INI_PLATFORM_ERROR, NULL, 0, __FILE__, __LINE__, "Failed to lock mutex");
    }
#endif

    __ini_clear_errstack();
    return create_error(INI_SUCCESS, NULL, 0, __FILE__, __LINE__, "Mutex locked successfully");
}

INIPARSER_API ini_error_details_t __INI_MUTEX_UNLOCK(ini_context_t *ctx)
{
    if (!ctx)
    {
        __ini_add_in_errstack(INI_INVALID_ARGUMENT);
        return create_error(INI_INVALID_ARGUMENT, NULL, 0, __FILE__, __LINE__, "Context is NULL");
    }

#if INI_OS_WINDOWS
    LeaveCriticalSection(&ctx->mutex);
#elif INI_OS_APPLE
    dispatch_semaphore_signal(ctx->semaphore);
#else
    if (pthread_mutex_unlock(&ctx->mutex) != 0)
    {
        __ini_add_in_errstack(INI_PLATFORM_ERROR);
        return create_error(INI_PLATFORM_ERROR, NULL, 0, __FILE__, __LINE__, "Failed to unlock mutex");
    }
#endif

    __ini_clear_errstack();
    return create_error(INI_SUCCESS, NULL, 0, __FILE__, __LINE__, "Mutex unlocked successfully");
}

INIPARSER_API ini_error_details_t ini_save(ini_context_t const *ctx, char const *filepath)
{
    // 1. Check arguments for NULL
    if (!ctx)
    {
        __ini_add_in_errstack(INI_INVALID_ARGUMENT);
        return create_error(
            INI_INVALID_ARGUMENT,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Context is NULL");
    }

    if (!filepath)
    {
        __ini_add_in_errstack(INI_INVALID_ARGUMENT);
        return create_error(
            INI_INVALID_ARGUMENT,
            NULL,
            0,
            __FILE__,
            __LINE__,
            "Filepath is NULL");
    }

    // Check if the directory is writable (Windows-specific)
#if INI_OS_WINDOWS
    {
        // Extract directory path from filepath
        char dirpath[MAX_PATH];
        strncpy(dirpath, filepath, sizeof(dirpath));
        dirpath[sizeof(dirpath) - 1] = '\0';

        char *last_slash = strrchr(dirpath, '\\');
        if (last_slash)
            *last_slash = '\0'; // Remove filename, keep directory
        else
            dirpath[0] = '\0'; // No directory component (current dir)

        DWORD attrs = GetFileAttributes(dirpath[0] ? dirpath : ".");
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_READONLY))
        {
            __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
            return create_error(
                INI_FILE_OPEN_FAILED,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Parent directory is read-only");
        }
    }
#endif

    // 2. Open file for writing
    FILE *file = NULL;
#if INI_OS_WINDOWS
    if (fopen_s(&file, filepath, "w") != 0 || !file)
    {
        __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to open file for writing");
    }
#else
    file = fopen(filepath, "w");
    if (!file)
    {
        __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to open file for writing");
    }
#endif

    // 3. Lock the context for thread safety
    ini_error_details_t err = __INI_MUTEX_LOCK((ini_context_t *)ctx); // Cast away const for locking (implementation detail)
    if (err.error != INI_SUCCESS)
    {
        if (fclose(file) != 0)
        {
            __ini_add_in_errstack(INI_CLOSE_FAILED);
            return create_error(
                INI_CLOSE_FAILED,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to close file after mutex lock error");
        }
        return err;
    }

    // 4. Write sections to file
    for (int i = 0; i < ctx->section_count; i++)
    {
        ini_section_t const *section = &ctx->sections[i];
        if (!section || !section->name)
            continue; // Skip invalid sections

        if (fprintf(file, "[%s]\n", section->name) < 0)
        {
            __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
            if (fclose(file) != 0)
            {
                __ini_add_in_errstack(INI_CLOSE_FAILED);
                return create_error(
                    INI_CLOSE_FAILED,
                    filepath,
                    0,
                    __FILE__,
                    __LINE__,
                    "Failed to close file after write error");
            }
            __ini_add_in_errstack(INI_PRINT_ERROR);
            return create_error(
                INI_PRINT_ERROR,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to write section header");
        }

        // Write key-value pairs
        for (int j = 0; j < section->pair_count; j++)
        {
            if (!section->pairs || !section->pairs[j].key)
                continue; // Skip invalid pairs

            // Handle the case when value is NULL or empty
            const char *value = section->pairs[j].value ? section->pairs[j].value : "";

            // Check if value contains spaces or special characters that need quoting
            int need_quotes = 0;
            if (value[0] == '\0') // Empty string needs quotes
                need_quotes = 0;  // Don't add quotes for empty values to match test expectations
            else
            {
                for (const char *p = value; *p; p++)
                {
                    if (*p == ' ' || *p == '\t' || *p == ';' || *p == '#' || *p == '=')
                    {
                        need_quotes = 1;
                        break;
                    }
                }
            }

            // Write key-value pair with or without quotes
            int result;
            if (need_quotes)
                result = fprintf(file, "%s=\"%s\"\n", section->pairs[j].key, value);
            else
                result = fprintf(file, "%s=%s\n", section->pairs[j].key, value);

            if (result < 0)
            {
                __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
                if (fclose(file) != 0)
                {
                    __ini_add_in_errstack(INI_CLOSE_FAILED);
                    return create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to close file after write error");
                }
                __ini_add_in_errstack(INI_PRINT_ERROR);
                return create_error(
                    INI_PRINT_ERROR,
                    filepath,
                    0,
                    __FILE__,
                    __LINE__,
                    "Failed to write key-value pair");
            }
        }

        // Print subsections
        if (section->subsection_count > 0)
        {
            // Add a blank line before subsections
            if (fprintf(file, "\n") < 0)
            {
                __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
                if (fclose(file) != 0)
                {
                    __ini_add_in_errstack(INI_CLOSE_FAILED);
                    return create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to close file after write error");
                }
                __ini_add_in_errstack(INI_PRINT_ERROR);
                return create_error(
                    INI_PRINT_ERROR,
                    filepath,
                    0,
                    __FILE__,
                    __LINE__,
                    "Failed to write newline");
            }
        }

        for (int k = 0; k < section->subsection_count; k++)
        {
            ini_section_t const *subsection = &section->subsections[k];
            if (!subsection || !subsection->name)
                continue; // Skip invalid subsections

            if (fprintf(file, "[%s.%s]\n", section->name, subsection->name) < 0)
            {
                __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
                if (fclose(file) != 0)
                {
                    __ini_add_in_errstack(INI_CLOSE_FAILED);
                    return create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to close file after write error");
                }
                __ini_add_in_errstack(INI_PRINT_ERROR);
                return create_error(
                    INI_PRINT_ERROR,
                    filepath,
                    0,
                    __FILE__,
                    __LINE__,
                    "Failed to write subsection header");
            }

            // Print subsection key-value pairs
            for (int l = 0; l < subsection->pair_count; l++)
            {
                if (!subsection->pairs || !subsection->pairs[l].key)
                    continue; // Skip invalid pairs

                // Handle the case when value is NULL or empty
                const char *value = subsection->pairs[l].value ? subsection->pairs[l].value : "";

                // Check if value contains spaces or special characters that need quoting
                int need_quotes = 0;
                if (value[0] == '\0') // Empty string needs quotes
                    need_quotes = 0;  // Don't add quotes for empty values to match test expectations
                else
                {
                    for (const char *p = value; *p; p++)
                    {
                        if (*p == ' ' || *p == '\t' || *p == ';' || *p == '#' || *p == '=')
                        {
                            need_quotes = 1;
                            break;
                        }
                    }
                }

                // Write key-value pair with or without quotes
                int result;
                if (need_quotes)
                    result = fprintf(file, "%s=\"%s\"\n", subsection->pairs[l].key, value);
                else
                    result = fprintf(file, "%s=%s\n", subsection->pairs[l].key, value);

                if (result < 0)
                {
                    __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
                    if (fclose(file) != 0)
                    {
                        __ini_add_in_errstack(INI_CLOSE_FAILED);
                        return create_error(
                            INI_CLOSE_FAILED,
                            filepath,
                            0,
                            __FILE__,
                            __LINE__,
                            "Failed to close file after write error");
                    }
                    __ini_add_in_errstack(INI_PRINT_ERROR);
                    return create_error(
                        INI_PRINT_ERROR,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to write subsection key-value pair");
                }
            }
        }

        // Add an empty line between sections for better readability
        if (i < ctx->section_count - 1 && fprintf(file, "\n") < 0)
        {
            __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
            if (fclose(file) != 0)
            {
                __ini_add_in_errstack(INI_CLOSE_FAILED);
                return create_error(
                    INI_CLOSE_FAILED,
                    filepath,
                    0,
                    __FILE__,
                    __LINE__,
                    "Failed to close file after write error");
            }
            __ini_add_in_errstack(INI_PRINT_ERROR);
            return create_error(
                INI_PRINT_ERROR,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to write newline");
        }
    }

    // 5. Unlock the context
    err = __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
    if (err.error != INI_SUCCESS)
    {
        if (fclose(file) != 0)
        {
            __ini_add_in_errstack(INI_CLOSE_FAILED);
            return create_error(
                INI_CLOSE_FAILED,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to close file after mutex unlock error");
        }
        return err;
    }

    // 6. Close the file
    if (fclose(file) != 0)
    {
        __ini_add_in_errstack(INI_CLOSE_FAILED);
        return create_error(
            INI_CLOSE_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to close file");
    }

    // 7. Return success
    __ini_clear_errstack();
    return create_error(
        INI_SUCCESS,
        filepath,
        0,
        __FILE__,
        __LINE__,
        "File saved successfully");
}

INIPARSER_API ini_error_details_t ini_save_section_value(ini_context_t const *ctx,
                                                         char const *filepath,
                                                         char const *section,
                                                         char const *key)
{
    // 1. Validate input parameters
    if (!ctx)
    {
        __ini_add_in_errstack(INI_INVALID_ARGUMENT);
        return create_error(
            INI_INVALID_ARGUMENT,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Context is NULL");
    }

    if (!filepath)
    {
        __ini_add_in_errstack(INI_INVALID_ARGUMENT);
        return create_error(
            INI_INVALID_ARGUMENT,
            NULL,
            0,
            __FILE__,
            __LINE__,
            "Filepath is NULL");
    }

    if (!section)
    {
        __ini_add_in_errstack(INI_INVALID_ARGUMENT);
        return create_error(
            INI_INVALID_ARGUMENT,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Section name is NULL");
    }

    // 2. Lock the context for thread safety
    ini_error_details_t err = __INI_MUTEX_LOCK((ini_context_t *)ctx);
    if (err.error != INI_SUCCESS)
    {
        return err;
    }

    // 3. Find the section and key in the context
    ini_section_t const *found_section = NULL;
    ini_section_t const *parent_section = NULL;
    ini_key_value_t const *found_pair = NULL;
    int is_subsection = 0;

    // Check if this is a subsection reference (contains a dot)
    const char *dot = strchr(section, '.');
    if (dot)
    {
        // This is a subsection reference
        size_t parent_name_len = dot - section;
        char *parent_name = malloc(parent_name_len + 1);
        if (!parent_name)
        {
            __ini_add_in_errstack(INI_MEMORY_ERROR);
            __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
            return create_error(
                INI_MEMORY_ERROR,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to allocate memory for parent section name");
        }

        // Copy parent section name (before the dot)
        strncpy(parent_name, section, parent_name_len);
        parent_name[parent_name_len] = '\0';

        // Get subsection name (after the dot)
        const char *subsection_name = dot + 1;

        // Find parent section
        for (int i = 0; i < ctx->section_count; i++)
        {
            if (ctx->sections[i].name && strcmp(ctx->sections[i].name, parent_name) == 0)
            {
                parent_section = &ctx->sections[i];
                break;
            }
        }

        if (parent_section)
        {
            // Find the subsection
            for (int i = 0; i < parent_section->subsection_count; i++)
            {
                if (parent_section->subsections[i].name &&
                    strcmp(parent_section->subsections[i].name, subsection_name) == 0)
                {
                    found_section = &parent_section->subsections[i];
                    is_subsection = 1;
                    break;
                }
            }
        }

        free(parent_name);

        if (!found_section)
        {
            __ini_add_in_errstack(INI_SECTION_NOT_FOUND);
            __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
            return create_error(
                INI_SECTION_NOT_FOUND,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Section not found");
        }
    }
    else
    {
        // Look for a top-level section
        for (int i = 0; i < ctx->section_count; i++)
        {
            if (ctx->sections[i].name && strcmp(ctx->sections[i].name, section) == 0)
            {
                found_section = &ctx->sections[i];
                break;
            }
        }

        if (!found_section)
        {
            __ini_add_in_errstack(INI_SECTION_NOT_FOUND);
            __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
            return create_error(
                INI_SECTION_NOT_FOUND,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Section not found");
        }
    }

    // If key is specified, find it in the section
    if (key)
    {
        int key_found = 0;
        for (int i = 0; i < found_section->pair_count; i++)
        {
            if (found_section->pairs[i].key && strcmp(found_section->pairs[i].key, key) == 0)
            {
                found_pair = &found_section->pairs[i];
                key_found = 1;
                break;
            }
        }

        if (!key_found)
        {
            __ini_add_in_errstack(INI_KEY_NOT_FOUND);
            __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
            return create_error(
                INI_KEY_NOT_FOUND,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Key not found in section");
        }
    }

    // 4. Check if the file exists
    int file_exists = 0;
    FILE *temp_file = NULL;

#if INI_OS_WINDOWS
    if (_access(filepath, F_OK) == 0)
    {
        file_exists = 1;
    }

    if (file_exists)
    {
        // Open the existing file for reading
        if (fopen_s(&temp_file, filepath, "r") != 0 || !temp_file)
        {
            __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
            __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
            return create_error(
                INI_FILE_OPEN_FAILED,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to open existing file for reading");
        }
    }
#else
    if (access(filepath, F_OK) == 0)
    {
        file_exists = 1;
    }

    if (file_exists)
    {
        // Open the existing file for reading
        temp_file = fopen(filepath, "r");
        if (!temp_file)
        {
            __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
            __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
            return create_error(
                INI_FILE_OPEN_FAILED,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to open existing file for reading");
        }
    }
#endif

    // 5. Create a temporary file for writing
    char temp_filepath[1024];
    FILE *out_file = NULL;

#if INI_OS_WINDOWS
    if (tmpnam_s(temp_filepath, sizeof(temp_filepath)) != 0)
    {
        if (temp_file)
            fclose(temp_file);
        __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
        __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to create temporary filename");
    }

    if (fopen_s(&out_file, temp_filepath, "w") != 0 || !out_file)
    {
        if (temp_file)
            fclose(temp_file);
        __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
        __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to create temporary file");
    }
#else
    FILE *tmpf = tmpfile();
    if (!tmpf)
    {
        if (temp_file)
            fclose(temp_file);
        __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
        __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to create temporary file");
    }
    out_file = tmpf;
#endif

    // 6. If file exists, copy its contents to the temporary file,
    // replacing or inserting the specified section/key
    if (file_exists && temp_file)
    {
        char line[INI_LINE_MAX];
        char current_section[INI_LINE_MAX] = "";
        int in_target_section = 0;
        int target_section_written = 0;
        int key_written = 0;

        while (fgets(line, sizeof(line), temp_file))
        {
            // Check if this is a section header
            if (line[0] == '[')
            {
                char *end = strchr(line, ']');
                if (end)
                {
                    *end = '\0';
                    strcpy(current_section, line + 1);
                    *end = ']'; // Restore the closing bracket

                    // Check if this is the target section
                    if (strcmp(current_section, section) == 0)
                    {
                        in_target_section = 1;
                        target_section_written = 1;

                        // Write the section header as is
                        fprintf(out_file, "[%s]", current_section);

                        // Add back any trailing comment
                        if (*(end + 1) != '\0')
                        {
                            fprintf(out_file, "%s", end + 1);
                        }
                        fprintf(out_file, "\n");

                        // If we're just updating the section with no specific key,
                        // write all key-value pairs from the context
                        if (!key)
                        {
                            for (int i = 0; i < found_section->pair_count; i++)
                            {
                                if (!found_section->pairs[i].key)
                                    continue;

                                const char *pair_key = found_section->pairs[i].key;
                                const char *value = found_section->pairs[i].value ? found_section->pairs[i].value : "";
                                int need_quotes = 0;

                                if (value[0] == '\0')
                                {
                                    need_quotes = 0; // Don't add quotes for empty values
                                }
                                else
                                {
                                    for (const char *p = value; *p; p++)
                                    {
                                        if (*p == ' ' || *p == '\t' || *p == ';' || *p == '#' || *p == '=')
                                        {
                                            need_quotes = 1;
                                            break;
                                        }
                                    }
                                }

                                if (need_quotes)
                                    fprintf(out_file, "%s=\"%s\"\n", pair_key, value);
                                else
                                    fprintf(out_file, "%s=%s\n", pair_key, value);
                            }

                            // Skip to the next section since we've written all keys
                            in_target_section = 0;
                        }
                        else
                        {
                            // Write all key-value pairs, replacing the specific key with the updated value
                            for (int i = 0; i < found_section->pair_count; i++)
                            {
                                if (!found_section->pairs[i].key)
                                    continue;

                                // If this is the key we're updating, use the found_pair
                                if (strcmp(found_section->pairs[i].key, key) == 0)
                                {
                                    const char *value = found_pair->value ? found_pair->value : "";
                                    int need_quotes = 0;

                                    if (value[0] == '\0')
                                    {
                                        need_quotes = 0; // Don't add quotes for empty values
                                    }
                                    else
                                    {
                                        for (const char *p = value; *p; p++)
                                        {
                                            if (*p == ' ' || *p == '\t' || *p == ';' || *p == '#' || *p == '=')
                                            {
                                                need_quotes = 1;
                                                break;
                                            }
                                        }
                                    }

                                    if (need_quotes)
                                        fprintf(out_file, "%s=\"%s\"\n", key, value);
                                    else
                                        fprintf(out_file, "%s=%s\n", key, value);

                                    key_written = 1;
                                    target_section_written = 1; // Mark that we've written to the target section
                                }
                                else
                                {
                                    // Write other keys as they are
                                    const char *pair_key = found_section->pairs[i].key;
                                    const char *value = found_section->pairs[i].value ? found_section->pairs[i].value : "";
                                    int need_quotes = 0;

                                    if (value[0] == '\0')
                                    {
                                        need_quotes = 0; // Don't add quotes for empty values
                                    }
                                    else
                                    {
                                        for (const char *p = value; *p; p++)
                                        {
                                            if (*p == ' ' || *p == '\t' || *p == ';' || *p == '#' || *p == '=')
                                            {
                                                need_quotes = 1;
                                                break;
                                            }
                                        }
                                    }

                                    if (need_quotes)
                                        fprintf(out_file, "%s=\"%s\"\n", pair_key, value);
                                    else
                                        fprintf(out_file, "%s=%s\n", pair_key, value);
                                }
                            }

                            // Skip to the next section since we've written all keys
                            in_target_section = 0;
                        }
                        continue;
                    }
                }
            }
            else if (in_target_section && key && strchr(line, '='))
            {
                // We're in the target section and this is a key-value pair
                char *eq = strchr(line, '=');
                if (eq)
                {
                    *eq = '\0';
                    char *line_key = line;

                    // Trim whitespace from the key
                    while (*line_key && (*line_key == ' ' || *line_key == '\t'))
                        line_key++;

                    char *end = eq - 1;
                    while (end > line_key && (*end == ' ' || *end == '\t'))
                        *end-- = '\0';

                    // Check if this is the key we want to replace
                    if (strcmp(line_key, key) == 0)
                    {
                        // We found the key, write the new value instead
                        const char *value = found_pair->value ? found_pair->value : "";
                        int need_quotes = 0;

                        if (value[0] == '\0')
                        {
                            need_quotes = 0; // Don't add quotes for empty values
                        }
                        else
                        {
                            for (const char *p = value; *p; p++)
                            {
                                if (*p == ' ' || *p == '\t' || *p == ';' || *p == '#' || *p == '=')
                                {
                                    need_quotes = 1;
                                    break;
                                }
                            }
                        }

                        if (need_quotes)
                            fprintf(out_file, "%s=\"%s\"\n", key, value);
                        else
                            fprintf(out_file, "%s=%s\n", key, value);

                        key_written = 1;
                        target_section_written = 1; // Mark that we've written to the target section
                        continue;
                    }

                    // Restore the equals sign
                    *eq = '=';
                }
            }

            // Write the line to the output file
            fprintf(out_file, "%s", line);
        }

        // Close the input file
        fclose(temp_file);

        // If the section or key wasn't found in the file, add it at the end
        if (!target_section_written)
        {
            // Add a newline if needed
            if (ftell(out_file) > 0)
            {
                fprintf(out_file, "\n");
            }

            if (is_subsection)
            {
                // Write the parent section if it doesn't exist in the file
                fprintf(out_file, "[%s]\n\n", parent_section->name);

                // Write the subsection
                fprintf(out_file, "[%s.%s]\n", parent_section->name, found_section->name);
            }
            else
            {
                // Write the section header
                fprintf(out_file, "[%s]\n", section);
            }

            // Write the key-value pair
            if (key && found_pair)
            {
                key_written = 1; // Mark key as written so we don't duplicate it
                const char *value = found_pair->value ? found_pair->value : "";
                int need_quotes = 0;

                if (value[0] == '\0')
                {
                    need_quotes = 0; // Don't add quotes for empty values
                }
                else
                {
                    for (const char *p = value; *p; p++)
                    {
                        if (*p == ' ' || *p == '\t' || *p == ';' || *p == '#' || *p == '=')
                        {
                            need_quotes = 1;
                            break;
                        }
                    }
                }

                if (need_quotes)
                    fprintf(out_file, "%s=\"%s\"\n", key, value);
                else
                    fprintf(out_file, "%s=%s\n", key, value);
            }
            else if (!key)
            {
                // Write all keys in the section
                for (int i = 0; i < found_section->pair_count; i++)
                {
                    if (!found_section->pairs[i].key)
                        continue;

                    const char *pair_key = found_section->pairs[i].key;
                    const char *value = found_section->pairs[i].value ? found_section->pairs[i].value : "";
                    int need_quotes = 0;

                    if (value[0] == '\0')
                    {
                        need_quotes = 0; // Don't add quotes for empty values
                    }
                    else
                    {
                        for (const char *p = value; *p; p++)
                        {
                            if (*p == ' ' || *p == '\t' || *p == ';' || *p == '#' || *p == '=')
                            {
                                need_quotes = 1;
                                break;
                            }
                        }
                    }

                    if (need_quotes)
                        fprintf(out_file, "%s=\"%s\"\n", pair_key, value);
                    else
                        fprintf(out_file, "%s=%s\n", pair_key, value);
                }
            }
        }
    }
    else // File doesn't exist or couldn't be opened, create a new one
    {
        if (is_subsection)
        {
            // Write the parent section
            fprintf(out_file, "[%s]\n\n", parent_section->name);

            // Write the subsection
            fprintf(out_file, "[%s.%s]\n", parent_section->name, found_section->name);
        }
        else
        {
            // Write the section header
            fprintf(out_file, "[%s]\n", section);
        }

        // Write the key-value pair
        if (key && found_pair)
        {
            const char *value = found_pair->value ? found_pair->value : "";
            int need_quotes = 0;

            if (value[0] == '\0')
            {
                need_quotes = 0; // Don't add quotes for empty values
            }
            else
            {
                for (const char *p = value; *p; p++)
                {
                    if (*p == ' ' || *p == '\t' || *p == ';' || *p == '#' || *p == '=')
                    {
                        need_quotes = 1;
                        break;
                    }
                }
            }

            if (need_quotes)
                fprintf(out_file, "%s=\"%s\"\n", key, value);
            else
                fprintf(out_file, "%s=%s\n", key, value);
        }
        else if (!key)
        {
            // Write all keys in the section
            for (int i = 0; i < found_section->pair_count; i++)
            {
                if (!found_section->pairs[i].key)
                    continue;

                const char *pair_key = found_section->pairs[i].key;
                const char *value = found_section->pairs[i].value ? found_section->pairs[i].value : "";
                int need_quotes = 0;

                if (value[0] == '\0')
                {
                    need_quotes = 0; // Don't add quotes for empty values
                }
                else
                {
                    for (const char *p = value; *p; p++)
                    {
                        if (*p == ' ' || *p == '\t' || *p == ';' || *p == '#' || *p == '=')
                        {
                            need_quotes = 1;
                            break;
                        }
                    }
                }

                if (need_quotes)
                    fprintf(out_file, "%s=\"%s\"\n", pair_key, value);
                else
                    fprintf(out_file, "%s=%s\n", pair_key, value);
            }
        }
    }

    // 7. Flush and close the temporary file
    if (fflush(out_file) != 0)
    {
        __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
        __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
#if !INI_OS_WINDOWS
        if (tmpf)
            fclose(tmpf);
#else
        fclose(out_file);
        remove(temp_filepath); // Clean up temp file
#endif
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to flush temporary file");
    }

#if INI_OS_WINDOWS
    fclose(out_file);

    // Remove the target file if it exists
    if (file_exists)
    {
        if (remove(filepath) != 0)
        {
            __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
            __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
            remove(temp_filepath); // Clean up temporary file
            return create_error(
                INI_FILE_OPEN_FAILED,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to remove existing file");
        }
    }

    // Rename the temporary file to the target file
    if (rename(temp_filepath, filepath) != 0)
    {
        __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
        __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
        remove(temp_filepath); // Clean up temporary file
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to rename temporary file");
    }
#else
    // For Unix-like systems, create or open the destination file
    FILE *dest_file = fopen(filepath, "w");
    if (!dest_file)
    {
        __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
        __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
        fclose(tmpf); // Close the temporary file
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to open destination file for writing");
    }

    // Rewind the temporary file
    rewind(tmpf);

    // Copy the contents of the temporary file to the destination file
    char buffer[INI_LINE_MAX];
    while (fgets(buffer, sizeof(buffer), tmpf))
    {
        if (fputs(buffer, dest_file) < 0)
        {
            __ini_add_in_errstack(INI_FILE_OPEN_FAILED);
            __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
            fclose(tmpf);
            fclose(dest_file);
            return create_error(
                INI_FILE_OPEN_FAILED,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to write to destination file");
        }
    }

    // Close both files
    fclose(tmpf);
    fclose(dest_file);
#endif

    // 8. Unlock the context
    err = __INI_MUTEX_UNLOCK((ini_context_t *)ctx);
    if (err.error != INI_SUCCESS)
    {
        return err;
    }

    // 9. Success
    __ini_clear_errstack();
    return create_error(
        INI_SUCCESS,
        filepath,
        0,
        __FILE__,
        __LINE__,
        "Section/key saved successfully");
}

// Helper function to find a section by name
INIPARSER_API const ini_section_t *__ini_find_section(const ini_context_t *ctx, const char *section_name)
{
    if (!ctx || !section_name || !ctx->sections)
        return NULL;

    // Check if this is a subsection reference (contains a dot)
    const char *dot = strchr(section_name, '.');
    if (dot)
    {
        // This is a subsection reference
        // Extract the parent section name (before the dot)
        size_t parent_name_len = dot - section_name;
        char parent_name[INI_LINE_MAX] = {0};
        strncpy(parent_name, section_name, parent_name_len);
        parent_name[parent_name_len] = '\0';

        // Find the parent section
        const ini_section_t *parent = NULL;
        for (int i = 0; i < ctx->section_count; i++)
        {
            if (ctx->sections[i].name && strcmp(ctx->sections[i].name, parent_name) == 0)
            {
                parent = &ctx->sections[i];
                break;
            }
        }

        if (!parent || !parent->subsections)
            return NULL;

        // Get subsection name (after the dot)
        const char *subsection_name = dot + 1;

        // Find the subsection
        for (int i = 0; i < parent->subsection_count; i++)
        {
            if (parent->subsections[i].name &&
                strcmp(parent->subsections[i].name, subsection_name) == 0)
            {
                return &parent->subsections[i];
            }
        }
    }
    else
    {
        // Look for a top-level section
        for (int i = 0; i < ctx->section_count; i++)
        {
            if (ctx->sections[i].name && strcmp(ctx->sections[i].name, section_name) == 0)
            {
                return &ctx->sections[i];
            }
        }
    }

    return NULL;
}

// Helper function to find a key-value pair by key in a section
INIPARSER_API const ini_key_value_t *__ini_find_pair(const ini_section_t *section, const char *key)
{
    if (!section || !key || !section->pairs)
        return NULL;

    for (int i = 0; i < section->pair_count; i++)
    {
        if (section->pairs[i].key && strcmp(section->pairs[i].key, key) == 0)
        {
            return &section->pairs[i];
        }
    }

    return NULL;
}
