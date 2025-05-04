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
    size_t len = strlen(str) + 1;
    char *dup = malloc(len);
    return dup ? memcpy(dup, str, len) : NULL;
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

INIPARSER_API ini_error_details_t ini_good(char const *filepath)
{
    if (!filepath)
    {
        return create_error(
            INI_INVALID_ARGUMENT,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Null filepath provided");
    }

    // --- Platform-agnostic checks ---
    // Check file existence and readability
#if INI_OS_WINDOWS
    if (_access(filepath, F_OK) != 0)
    {
        return create_error(
            INI_FILE_NOT_FOUND,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "File does not exist");
    }
    if (_access(filepath, R_OK) != 0)
    {
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "File is not readable");
    }
#else
    // On Unix/macOS, use stat() for more reliable existence check
    struct stat statbuf;
    if (stat(filepath, &statbuf) != 0)
    {
        return create_error(
            INI_FILE_NOT_FOUND,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "File does not exist");
    }
    if (access(filepath, R_OK) != 0)
    {
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "File is not readable");
    }
#endif

    // Check if it's a directory
#if INI_OS_WINDOWS
    DWORD attrs = GetFileAttributesA(filepath);
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        return create_error(
            INI_PLATFORM_ERROR,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to get file attributes");
    }
    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    {
        return create_error(
            INI_FILE_IS_DIR,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Path is a directory");
    }
#else
    if (stat(filepath, &statbuf) != 0)
    {
        return create_error(
            INI_PLATFORM_ERROR,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to get file stats");
    }
    if (S_ISDIR(statbuf.st_mode))
    {
        return create_error(
            INI_FILE_IS_DIR,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Path is a directory");
    }
#endif

    // Check file size
#if INI_OS_WINDOWS
    FILE *file;
    if (fopen_s(&file, filepath, "rb") != 0)
    {
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to open file for size check");
    }
#else
    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to open file for size check");
    }
#endif

    if (fseek(file, 0, SEEK_END) != 0)
    {
        if (fclose(file) != 0)
        {
            return create_error(
                INI_CLOSE_FAILED,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to close file after size check");
        }

        return create_error(
            INI_PLATFORM_ERROR,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to seek to end of file");
    }

    long size = ftell(file);
    if (fclose(file) != 0)
    {
        return create_error(
            INI_CLOSE_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to close file after size check");
    }

    if (size == 0)
    {
        return create_error(
            INI_FILE_EMPTY,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "File is empty");
    }

    // --- Syntax validation ---
#if INI_OS_WINDOWS
    if (fopen_s(&file, filepath, "r") != 0)
    {
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to open file for syntax validation");
    }
#else
    file = fopen(filepath, "r");
    if (!file)
    {
        return create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to open file for syntax validation");
    }
#endif

    char line[INI_LINE_MAX];
    int line_num = 0, in_section = 0;

    while (fgets(line, sizeof(line), file))
    {
        line_num++;
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t')
            trimmed++; // Trim leading whitespace

        // Skip empty lines and comments
        if (*trimmed == '\0' || *trimmed == '\n' || *trimmed == ';' || *trimmed == '#')
        {
            continue;
        }

        // Check for section
        if (*trimmed == '[')
        {
            if (strchr(trimmed, ']') == NULL)
            {
                if (fclose(file) != 0)
                {
                    return create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        line_num,
                        __FILE__,
                        __LINE__,
                        "Failed to close file after validation");
                }

                return create_error(
                    INI_FILE_BAD_FORMAT,
                    filepath,
                    line_num,
                    __FILE__,
                    __LINE__,
                    "Missing closing bracket for section");
            }
            in_section = 1;
        }
        // Check for key-value pair
        else if (in_section && strchr(trimmed, '=') != NULL)
        {
            char *eq = strchr(trimmed, '=');
            if (eq == trimmed || eq[1] == '\0' || eq[1] == '\n')
            {
                if (fclose(file) != 0)
                {
                    return create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        line_num,
                        __FILE__,
                        __LINE__,
                        "Failed to close file after validation");
                }

                return create_error(
                    INI_FILE_BAD_FORMAT,
                    filepath,
                    line_num,
                    __FILE__,
                    __LINE__,
                    "Empty key or value");
            }

            // Check for unbalanced quotes
            char *value_start = eq + 1;
            int quote_count = 0;
            for (char *p = value_start; *p && *p != '\n'; p++)
            {
                if (*p == '"')
                {
                    quote_count++;
                }
                // Array detection (.ini does not support arrays)
                else if (*p == ',')
                {
                    if (fclose(file) != 0)
                    {
                        return create_error(
                            INI_CLOSE_FAILED,
                            filepath,
                            line_num,
                            __FILE__,
                            __LINE__,
                            "Failed to close file after validation");
                    }
                    return create_error(
                        INI_FILE_BAD_FORMAT,
                        filepath,
                        line_num,
                        __FILE__,
                        __LINE__,
                        "Arrays are not supported in INI files");
                }
            }
            if (quote_count % 2 != 0)
            {
                if (fclose(file) != 0)
                {
                    return create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        line_num,
                        __FILE__,
                        __LINE__,
                        "Failed to close file after validation");
                }
                return create_error(
                    INI_FILE_BAD_FORMAT,
                    filepath,
                    line_num,
                    __FILE__,
                    __LINE__,
                    "Unbalanced quotes in value");
            }
        }
        else
        {
            if (fclose(file) != 0)
            {
                return create_error(
                    INI_CLOSE_FAILED,
                    filepath,
                    line_num,
                    __FILE__,
                    __LINE__,
                    "Failed to close file after validation");
            }
            return create_error(
                INI_FILE_BAD_FORMAT,
                filepath,
                line_num,
                __FILE__,
                __LINE__,
                "Invalid line (not a section or key-value pair)");
        }

        if (strlen(trimmed) >= INI_LINE_MAX - 1)
        {
            return create_error(INI_FILE_BAD_FORMAT,
                                filepath,
                                line_num,
                                __FILE__,
                                __LINE__,
                                "Line too long, max length is 1024 characters");
        }
    }

    if (fclose(file) != 0)
    {
        return create_error(
            INI_CLOSE_FAILED,
            filepath,
            line_num,
            __FILE__,
            __LINE__,
            "Failed to close file after validation");
    }

    return create_error(
        INI_SUCCESS,
        filepath,
        line_num,
        __FILE__,
        __LINE__,
        "File is valid");
}

INIPARSER_API ini_error_details_t ini_load(ini_context_t *ctx, char const *filepath)
{
    if (!filepath)
        return create_error(
            INI_INVALID_ARGUMENT,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Filepath is emtpy");

    if (!ctx)
    {
        ctx = ini_create_context();
        if (!ctx)
            return create_error(
                INI_MEMORY_ERROR,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to create context");
    }
    else
    {
        ini_error_details_t err = ini_free(ctx);
        if (err.error != INI_SUCCESS)
            return err;
        ctx = ini_create_context();
        if (!ctx)
            return create_error(
                INI_MEMORY_ERROR,
                filepath,
                0,
                __FILE__,
                __LINE__,
                "Failed to create context");
    }

    // Validate file
    ini_error_details_t err = ini_good(filepath);
    if (err.error != INI_SUCCESS)
        goto cleanup;

    // Lock mutex
    err = __INI_MUTEX_LOCK(ctx);
    if (err.error != INI_SUCCESS)
        goto cleanup;

    // Open file
    FILE *file;
#if INI_OS_WINDOWS
    if (fopen_s(&file, filepath, "r") != 0)
    {
        err = create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to open file");
        goto unlock;
    }
#else
    file = fopen(filepath, "r");
    if (!file)
    {
        err = create_error(
            INI_FILE_OPEN_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to open file");
        goto unlock;
    }
#endif

    char line[INI_LINE_MAX];
    ini_section_t *current_section = NULL;

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
                    err = create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to close file");
                    goto unlock;
                }
                err = create_error(
                    INI_FILE_BAD_FORMAT,
                    filepath,
                    0,
                    __FILE__,
                    __LINE__,
                    "Failed to close file");
                goto unlock;
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
                for (int i = 0; i < ctx->section_count; i++)
                {
                    if (strcmp(ctx->sections[i].name, name) == 0)
                    {
                        parent = &ctx->sections[i];
                        break;
                    }
                }
                if (!parent)
                {
                    if (fclose(file) != 0)
                    {
                        err = create_error(
                            INI_CLOSE_FAILED,
                            filepath,
                            0,
                            __FILE__,
                            __LINE__,
                            "Failed to close file");
                        goto unlock;
                    }
                    err = create_error(
                        INI_SECTION_NOT_FOUND,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to close file");
                    goto unlock;
                }

                // Add subsection
                parent->subsections = realloc(parent->subsections, (parent->subsection_count + 1) * sizeof(ini_section_t));
                if (!parent->subsections)
                {
                    if (fclose(file) != 0)
                    {
                        err = create_error(
                            INI_CLOSE_FAILED,
                            filepath,
                            0,
                            __FILE__,
                            __LINE__,
                            "Failed to close file");
                        goto unlock;
                    }
                    err = create_error(
                        INI_MEMORY_ERROR,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to close file");
                    goto unlock;
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
                ctx->sections = realloc(ctx->sections, (ctx->section_count + 1) * sizeof(ini_section_t));
                if (!ctx->sections)
                {
                    if (fclose(file) != 0)
                    {
                        err = create_error(
                            INI_CLOSE_FAILED,
                            filepath,
                            0,
                            __FILE__,
                            __LINE__,
                            "Failed to close file");
                        goto unlock;
                    }
                    err = create_error(
                        INI_MEMORY_ERROR,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to close file");
                    goto unlock;
                }
                current_section = &ctx->sections[ctx->section_count++];
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
            *eq = '\0';
            char *key = trimmed;
            char *value = eq + 1;

            // Trim whitespace
            while (*key == ' ' || *key == '\t')
                key++;
            char *key_end = key + strlen(key) - 1;
            while (key_end > key && (*key_end == ' ' || *key_end == '\t'))
                key_end--;
            *(key_end + 1) = '\0';

            while (*value == ' ' || *value == '\t')
                value++;
            char *value_end = value + strlen(value) - 1;
            while (value_end > value && (*value_end == ' ' || *value_end == '\t' || *value_end == '\n'))
                value_end--;
            *(value_end + 1) = '\0';

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
                    err = create_error(
                        INI_CLOSE_FAILED,
                        filepath,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to close file");
                    goto unlock;
                }
                err = create_error(
                    INI_MEMORY_ERROR,
                    filepath,
                    0,
                    __FILE__,
                    __LINE__,
                    "Failed to close file");
                goto unlock;
            }
            current_section->pairs[current_section->pair_count].key = ini_strdup(key);
            current_section->pairs[current_section->pair_count].value = ini_strdup(value);
            current_section->pair_count++;
        }
    }

    if (fclose(file) != 0)
    {
        err = create_error(
            INI_CLOSE_FAILED,
            filepath,
            0,
            __FILE__,
            __LINE__,
            "Failed to close file");
        goto unlock;
    }

    goto unlock;
    return create_error(
        INI_SUCCESS,
        filepath,
        0,
        __FILE__,
        __LINE__,
        "File loaded successfully");

unlock:
    __INI_MUTEX_UNLOCK(ctx);
    if (err.error != INI_SUCCESS)
        goto cleanup;

cleanup:
    if (err.error != INI_SUCCESS && ctx)
        err = ini_free(ctx);
    return err;
}

INIPARSER_API ini_context_t *ini_create_context()
{
    ini_context_t *ctx = (ini_context_t *)malloc(sizeof(ini_context_t));
    if (!ctx)
        return NULL;

    ctx->sections = NULL;
    ctx->section_count = 0;

    // Platform-specific initialization of the mutex/semaphore
#if INI_OS_WINDOWS
    InitializeCriticalSection(&ctx->mutex);
#elif INI_OS_APPLE
    ctx->semaphore = dispatch_semaphore_create(1); // Binary semaphore for mutual exclusion
    if (!ctx->semaphore)
        return NULL;
#else
    if (pthread_mutex_init(&ctx->mutex, NULL) != 0)
        return NULL;
#endif

    return ctx;
}

INIPARSER_API ini_error_details_t ini_free(ini_context_t *ctx)
{
    if (!ctx)
        return create_error(
            INI_INVALID_ARGUMENT,
            NULL,
            0,
            __FILE__,
            __LINE__,
            "Invalid arguments: NULL context, nothing to free");

    for (int i = 0; i < ctx->section_count; i++)
    {
        ini_section_t *section = &ctx->sections[i];
        for (int j = 0; j < section->pair_count; j++)
        {
            free(section->pairs[j].key);
            free(section->pairs[j].value);
        }
        free(section->pairs);

        for (int k = 0; k < section->subsection_count; k++)
        {
            ini_section_t *subsection = &section->subsections[k];
            for (int l = 0; l < subsection->pair_count; l++)
            {
                free(subsection->pairs[l].key);
                free(subsection->pairs[l].value);
            }
            free(subsection->pairs);
            free(subsection->name);
        }
        free(section->subsections);
        free(section->name);
    }
    free(ctx->sections);

#if INI_OS_WINDOWS
    DeleteCriticalSection(&ctx->mutex);
#elif INI_OS_APPLE
    dispatch_release(ctx->semaphore);
#else
    pthread_mutex_destroy(&ctx->mutex);
#endif

    return create_error(
        INI_SUCCESS,
        NULL,
        0,
        __FILE__,
        __LINE__,
        "File loaded successfully");
}

INIPARSER_API ini_error_details_t ini_get_value(ini_context_t const *ctx, char const *section, char const *key, char **value)
{
    if (!ctx || !section || !key || !value)
    {
        return create_error(
            INI_INVALID_ARGUMENT,
            NULL,
            0,
            __FILE__,
            __LINE__,
            "Invalid arguments: NULL context, section, key, or value pointer");
    }

    // Thread safety: Lock the mutex/semaphore
#if INI_OS_WINDOWS
    EnterCriticalSection(&((ini_context_t *)ctx)->mutex); // Cast away const for mutex access
#elif INI_OS_APPLE
    dispatch_semaphore_wait(((ini_context_t *)ctx)->semaphore, DISPATCH_TIME_FOREVER);
#else
    pthread_mutex_lock(&((ini_context_t *)ctx)->mutex);
#endif

    // Search for the section
    ini_section_t const *found_section = NULL;
    for (int i = 0; i < ctx->section_count; i++)
    {
        if (strcmp(ctx->sections[i].name, section) == 0)
        {
            found_section = &ctx->sections[i];
            break;
        }
    }

    if (!found_section)
    {
        // Unlock before returning
#if INI_OS_WINDOWS
        LeaveCriticalSection(&((ini_context_t *)ctx)->mutex);
#elif INI_OS_APPLE
        dispatch_semaphore_signal(((ini_context_t *)ctx)->semaphore);
#else
        pthread_mutex_unlock(&((ini_context_t *)ctx)->mutex);
#endif
        return create_error(
            INI_SECTION_NOT_FOUND,
            NULL,
            0,
            __FILE__,
            __LINE__,
            "Section not found");
    }

    // Search for the key in the section
    for (int j = 0; j < found_section->pair_count; j++)
    {
        if (strcmp(found_section->pairs[j].key, key) == 0)
        {
            *value = ini_strdup(found_section->pairs[j].value);
            if (!*value)
            {
                // Unlock before returning
#if INI_OS_WINDOWS
                LeaveCriticalSection(&((ini_context_t *)ctx)->mutex);
#elif INI_OS_APPLE
                dispatch_semaphore_signal(((ini_context_t *)ctx)->semaphore);
#else
                pthread_mutex_unlock(&((ini_context_t *)ctx)->mutex);
#endif
                return create_error(
                    INI_MEMORY_ERROR,
                    NULL,
                    0,
                    __FILE__,
                    __LINE__,
                    "Failed to duplicate value string");
            }
            // Unlock before returning
#if INI_OS_WINDOWS
            LeaveCriticalSection(&((ini_context_t *)ctx)->mutex);
#elif INI_OS_APPLE
            dispatch_semaphore_signal(((ini_context_t *)ctx)->semaphore);
#else
            pthread_mutex_unlock(&((ini_context_t *)ctx)->mutex);
#endif
            return create_error(
                INI_SUCCESS,
                NULL,
                0,
                __FILE__,
                __LINE__,
                "Value retrieved successfully");
        }
    }

    // Key not found in the section
    // Unlock before returning
#if INI_OS_WINDOWS
    LeaveCriticalSection(&((ini_context_t *)ctx)->mutex);
#elif INI_OS_APPLE
    dispatch_semaphore_signal(((ini_context_t *)ctx)->semaphore);
#else
    pthread_mutex_unlock(&((ini_context_t *)ctx)->mutex);
#endif
    return create_error(
        INI_KEY_NOT_FOUND,
        NULL,
        0,
        __FILE__,
        __LINE__,
        "Key not found in section");
}

INIPARSER_API ini_error_details_t ini_print(ini_context_t const *ctx)
{
    if (!ctx)
    {
        if (printf("Error: NULL context\n") < 0)
            return create_error(
                INI_PRINT_ERROR,
                NULL,
                0,
                __FILE__,
                __LINE__,
                "Failed to print error");

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
        if (printf("[%s]\n", section->name) < 0)
            return create_error(
                INI_PRINT_ERROR,
                NULL,
                0,
                __FILE__,
                __LINE__,
                "Failed to print error");

        // Print key-value pairs
        for (int j = 0; j < section->pair_count; j++)
        {
            if (printf("%s = %s\n", section->pairs[j].key, section->pairs[j].value) < 0)
                return create_error(
                    INI_PRINT_ERROR,
                    NULL,
                    0,
                    __FILE__,
                    __LINE__,
                    "Failed to print error");
        }

        // Print subsections
        for (int k = 0; k < section->subsection_count; k++)
        {
            ini_section_t const *subsection = &section->subsections[k];
            if (printf("\n[%s.%s]\n", section->name, subsection->name) < 0)
                return create_error(
                    INI_PRINT_ERROR,
                    NULL,
                    0,
                    __FILE__,
                    __LINE__,
                    "Failed to print error");

            // Print subsection key-value pairs
            for (int l = 0; l < subsection->pair_count; l++)
            {
                if (printf("%s = %s\n", subsection->pairs[l].key, subsection->pairs[l].value) < 0)
                    return create_error(
                        INI_PRINT_ERROR,
                        NULL,
                        0,
                        __FILE__,
                        __LINE__,
                        "Failed to print error");
            }
        }

        if (printf("\n") < 0)
            return create_error(
                INI_PRINT_ERROR,
                NULL,
                0,
                __FILE__,
                __LINE__,
                "Failed to print error");
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
        create_error(INI_INVALID_ARGUMENT, NULL, 0, __FILE__, __LINE__, "Context is NULL");

#if INI_OS_WINDOWS
    EnterCriticalSection(&ctx->mutex);
#elif INI_OS_APPLE
    dispatch_semaphore_wait(ctx->semaphore, DISPATCH_TIME_FOREVER);
#else
    pthread_mutex_lock(&ctx->mutex);
#endif

    return create_error(INI_SUCCESS, NULL, 0, __FILE__, __LINE__, "Mutex locked successfully");
}

INIPARSER_API ini_error_details_t __INI_MUTEX_UNLOCK(ini_context_t *ctx)
{
    if (!ctx)
        return create_error(INI_INVALID_ARGUMENT, NULL, 0, __FILE__, __LINE__, "Context is NULL");

#if INI_OS_WINDOWS
    LeaveCriticalSection(&ctx->mutex);
#elif INI_OS_APPLE
    dispatch_semaphore_signal(ctx->semaphore);
#else
    pthread_mutex_unlock(&ctx->mutex);
#endif

    return create_error(INI_SUCCESS, NULL, 0, __FILE__, __LINE__, "Mutex unlocked successfully");
}
