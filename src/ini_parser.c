#define INI_IMPLEMENTATION

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ini_filesystem.h"
#include "ini_parser.h"
#include "ini_string.h"

ini_error_t to_ini_error(ini_fs_error_t fs_err)
{
    switch (fs_err)
    {
    case INI_FS_SUCCESS:
        return INI_SUCCESS;
    case INI_FS_FILE_NOT_FOUND:
        return INI_FILE_NOT_FOUND;
    case INI_FS_FILE_IS_DIR:
        return INI_FILE_IS_DIR;
    case INI_FS_FILE_BAD_FORMAT:
        return INI_FILE_BAD_FORMAT;
    case INI_FS_FILE_EMPTY:
        return INI_FILE_EMPTY;
    case INI_FS_ACCESS_DENIED:
        return INI_FILE_PERMISSION_DENIED;
    case INI_FS_INVALID_PARAM:
        return INI_INVALID_ARGUMENT;
    case INI_FS_UNKNOWN_ERROR:
        return INI_UNKNOWN_ERROR;
    case INI_FS_STAT_ERROR:
        return INI_FILE_OPEN_FAILED;
    case INI_FS_WIN_GETFILEATTRIBUTES_ERROR:
        return INI_FILE_OPEN_FAILED;
    default:
        return INI_UNKNOWN_ERROR;
    }
}

// Helper function to convert a pointer to string
// This is used for storing pointers in the hash table
static char *ptr_to_str(void *ptr)
{
    char buf[32]; // Large enough for any pointer
    snprintf(buf, sizeof(buf), "%p", ptr);
    return ini_strdup(buf);
}

// Helper function to convert a string back to pointer
// This is used to retrieve pointers from the hash table
static void *str_to_ptr(const char *str)
{
    void *ptr;
    sscanf(str, "%p", &ptr);
    return ptr;
}

// Helper function to store section hash table
static void store_section_ht(ini_ht_t *sections, const char *section_name, ini_ht_t *section_ht)
{
    char *str_ptr = ptr_to_str(section_ht);
    if (str_ptr)
    {
        ini_ht_set(sections, section_name, str_ptr);
        free(str_ptr);
    }
}

// Helper function to get section hash table
static ini_ht_t *get_section_ht(ini_ht_t *sections, const char *section_name)
{
    const char *str_ptr = ini_ht_get(sections, section_name);
    if (!str_ptr)
        return NULL;
    return str_to_ptr(str_ptr);
}

INI_PUBLIC_API char const *ini_error_to_string(ini_error_t error)
{
    switch (error)
    {
    case INI_SUCCESS:
        return "Success";
    case INI_FILE_NOT_FOUND:
        return "Ini file not found";
    case INI_FILE_EMPTY:
        return "Ini file is empty";
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
    case INI_PRINT_ERROR:
        return "Ini print error";
    case INI_FILE_PERMISSION_DENIED:
        return "Ini file permission denied";
    default:
        return "Unknown error";
    }
}

INI_PUBLIC_API ini_context_t *ini_create_context()
{
    ini_context_t *ctx = (ini_context_t *)malloc(sizeof(ini_context_t));
    if (!ctx)
        return NULL;

    ctx->sections = ini_ht_create();
    if (!ctx->sections)
    {
        free(ctx);
        return NULL;
    }

    if (ini_mutex_init(&ctx->mutex) != INI_MUTEX_SUCCESS)
    {
        ini_ht_destroy(ctx->sections);
        free(ctx);
        return NULL;
    }

    return ctx;
}

INI_PUBLIC_API ini_error_t ini_free(ini_context_t *ctx)
{
    if (!ctx)
        return INI_INVALID_ARGUMENT;

    if (ini_mutex_lock(&ctx->mutex) != INI_MUTEX_SUCCESS)
        return INI_PLATFORM_ERROR;

    // Iterate over the sections
    if (ctx->sections)
    {
        ini_ht_iterator_t it = ini_ht_iterator(ctx->sections);
        char *section_name;
        char *section_ptr_str;

        while (ini_ht_next(&it, &section_name, &section_ptr_str) == INI_HT_SUCCESS)
        {
            ini_ht_t *section_ht = str_to_ptr(section_ptr_str);
            if (section_ht)
            {
                ini_ht_destroy(section_ht);
            }
        }
        ini_ht_destroy(ctx->sections);
    }

    ini_mutex_error_t unlock_err = ini_mutex_unlock(&ctx->mutex);
    ini_mutex_error_t destroy_err = ini_mutex_destroy(&ctx->mutex);

    if (ctx)
        free(ctx);

    if (unlock_err != INI_MUTEX_SUCCESS || destroy_err != INI_MUTEX_SUCCESS)
        return INI_PLATFORM_ERROR;

    return INI_SUCCESS;
}

INI_PUBLIC_API ini_error_t ini_good(char const *filepath)
{
    if (!filepath || !*filepath)
    {
        return INI_INVALID_ARGUMENT;
    }

    ini_fs_error_t fs_err = ini_check_file_status(filepath);
    if (fs_err != INI_FS_SUCCESS)
        return to_ini_error(fs_err);

    size_t file_size = 0;
    if (ini_get_file_size(filepath, &file_size) == INI_FS_SUCCESS && file_size == 0)
        return INI_FILE_EMPTY;

    FILE *file = ini_fopen(filepath, "r");
    if (!file)
        return INI_FILE_OPEN_FAILED;

    size_t fileSize;
    fs_err = ini_get_file_size(filepath, &fileSize);
    if (fs_err != INI_FS_SUCCESS)
        return to_ini_error(fs_err);
    if (fileSize == 0)
        return INI_FILE_EMPTY;

    // Validate INI syntax
    char line[INI_LINE_MAX];
    unsigned line_num = 0;
    int in_section = -1;
    ini_error_t error = INI_SUCCESS;

    while (fgets(line, sizeof(line), file))
    {
        line_num++;
        char *trimmed = line;

        // Trim leading whitespace
        while (*trimmed == ' ' || *trimmed == '\t')
        {
            trimmed++;
        }

        // Skip empty lines and comments
        if (*trimmed == '\0' || *trimmed == '\n' || *trimmed == ';' || *trimmed == '#')
        {
            continue;
        }

        // Check line length
        if (strlen(trimmed) >= INI_LINE_MAX - 1 && trimmed[INI_LINE_MAX - 2] != '\n')
        {
            error = INI_FILE_BAD_FORMAT;
            break;
        }

        // Check for section
        if (*trimmed == '[')
        {
            char *end = strchr(trimmed, ']');
            if (!end)
            {
                error = INI_FILE_BAD_FORMAT;
                break;
            }
            in_section = 0;
        }

        // Check for key-value pair
        else if (in_section == 0)
        {
            char *eq = strchr(trimmed, '=');
            if (!eq || eq == trimmed)
            {
                error = INI_FILE_BAD_FORMAT;
                break;
            }

            // Validate value (basic checks)
            char *value = eq + 1;
            while (*value == ' ' || *value == '\t')
            {
                value++;
            }

            // Check for quoted strings
            if (*value == '"')
            {
                char *end_quote = strchr(value + 1, '"');
                if (!end_quote || (end_quote[1] != '\0' && end_quote[1] != '\n' &&
                                   end_quote[1] != ';' && end_quote[1] != '#'))
                {
                    error = INI_FILE_BAD_FORMAT;
                    break;
                }
            }
        }
        else
        {
            error = INI_FILE_BAD_FORMAT;
            break;
        }
    }

    if (fclose(file) != 0 && error == INI_SUCCESS)
        error = INI_CLOSE_FAILED;

    return error;
}

INI_PUBLIC_API ini_error_t ini_load(ini_context_t *ctx, char const *filepath)
{
    if (!filepath)
    {
        return INI_INVALID_ARGUMENT;
    }

    // Validate file first
    ini_error_t err = ini_good(filepath);
    if (err != INI_SUCCESS)
    {
        return err;
    }

    // Create context if NULL
    ini_context_t *ctx_to_use = ctx;
    int need_to_free_on_error = 0;

    if (!ctx_to_use)
    {
        ctx_to_use = ini_create_context();
        if (!ctx_to_use)
        {
            return INI_MEMORY_ERROR;
        }
        need_to_free_on_error = 1;
    }
    else
    {
        // Clear existing context by freeing all section hash tables
        if (ini_mutex_lock(&ctx_to_use->mutex) != 0)
        {
            return INI_PLATFORM_ERROR;
        }

        ini_ht_iterator_t it = ini_ht_iterator(ctx_to_use->sections);
        char *section_name;
        char *section_ptr_str;

        while (ini_ht_next(&it, &section_name, &section_ptr_str) == INI_HT_SUCCESS)
        {
            ini_ht_t *section_ht = str_to_ptr(section_ptr_str);
            if (section_ht)
            {
                ini_ht_destroy(section_ht);
            }
        }

        // Reset the top-level hash table
        ini_ht_destroy(ctx_to_use->sections);
        ctx_to_use->sections = ini_ht_create();

        if (!ctx_to_use->sections)
        {
            ini_mutex_unlock(&ctx_to_use->mutex);
            return INI_MEMORY_ERROR;
        }

        ini_mutex_unlock(&ctx_to_use->mutex);
    }

    FILE *file = ini_fopen(filepath, "r");
    if (!file)
    {
        if (need_to_free_on_error)
        {
            ini_free(ctx_to_use);
        }
        return INI_FILE_OPEN_FAILED;
    }

    // Check for UTF-8 BOM
    unsigned char bom[3];
    size_t bytes_read = fread(bom, 1, 3, file);

    if (bytes_read < 3 || bom[0] != 0xEF || bom[1] != 0xBB || bom[2] != 0xBF)
    {
        rewind(file);
    }

    // Lock context for thread safety
    if (ini_mutex_lock(&ctx_to_use->mutex) != 0)
    {
        fclose(file);
        if (need_to_free_on_error)
        {
            ini_free(ctx_to_use);
        }
        return INI_PLATFORM_ERROR;
    }

    // Parse the INI file
    char line[INI_LINE_MAX];
    char current_section[INI_LINE_MAX] = ""; // Empty string means global section
    ini_ht_t *current_section_ht = NULL;
    int line_num = 0;

    while (fgets(line, sizeof(line), file))
    {
        line_num++;
        char *trimmed = line;

        // Trim leading whitespace
        while (*trimmed && (*trimmed == ' ' || *trimmed == '\t'))
        {
            trimmed++;
        }

        // Skip empty lines and comments
        if (*trimmed == '\0' || *trimmed == '\n' || *trimmed == '\r' ||
            *trimmed == ';' || *trimmed == '#')
        {
            continue;
        }

        // Handle section header
        if (*trimmed == '[')
        {
            char *end = strchr(trimmed, ']');
            if (!end)
            {
                fclose(file);
                ini_mutex_unlock(&ctx_to_use->mutex);
                if (need_to_free_on_error)
                {
                    ini_free(ctx_to_use);
                }
                return INI_FILE_BAD_FORMAT;
            }

            *end = '\0';
            strncpy(current_section, trimmed + 1, INI_LINE_MAX - 1);
            current_section[INI_LINE_MAX - 1] = '\0';

            // Get or create section hash table
            current_section_ht = (ini_ht_t *)ini_ht_get(ctx_to_use->sections, current_section);

            if (!current_section_ht)
            {
                // Create new section hash table
                current_section_ht = ini_ht_create();
                if (!current_section_ht)
                {
                    fclose(file);
                    ini_mutex_unlock(&ctx_to_use->mutex);
                    if (need_to_free_on_error)
                    {
                        ini_free(ctx_to_use);
                    }
                    return INI_MEMORY_ERROR;
                }

                // Add it to the sections hash table
                store_section_ht(ctx_to_use->sections, current_section, current_section_ht);
            }
        }
        // Handle key-value pair
        else if (strchr(trimmed, '='))
        {
            char *eq = strchr(trimmed, '=');
            if (!eq)
            {
                continue; // Shouldn't happen due to the check above
            }

            // Split key and value
            *eq = '\0';
            char *key = trimmed;
            char *value = eq + 1;

            // Trim key
            char *key_end = eq - 1;
            while (key_end >= key && (*key_end == ' ' || *key_end == '\t'))
            {
                *key_end-- = '\0';
            }

            // Trim value
            while (*value && (*value == ' ' || *value == '\t'))
            {
                value++;
            }

            // Remove trailing whitespace and newlines from value
            char *value_end = value + strlen(value) - 1;
            while (value_end >= value &&
                   (*value_end == ' ' || *value_end == '\t' ||
                    *value_end == '\n' || *value_end == '\r'))
            {
                *value_end-- = '\0';
            }

            // Handle quoted values
            if (*value == '"' && value_end >= value && *value_end == '"')
            {
                value++;
                *value_end = '\0';
            }

            // Skip empty keys
            if (*key == '\0')
            {
                continue;
            }

            // If we have no current section, create the default/global section
            if (!current_section_ht)
            {
                current_section_ht = ini_ht_create();
                if (!current_section_ht)
                {
                    fclose(file);
                    ini_mutex_unlock(&ctx_to_use->mutex);
                    if (need_to_free_on_error)
                    {
                        ini_free(ctx_to_use);
                    }
                    return INI_MEMORY_ERROR;
                }

                // Add it to the sections hash table with empty string key
                store_section_ht(ctx_to_use->sections, "", current_section_ht);
            }

            // Add/update key-value pair in current section
            ini_ht_set(current_section_ht, key, value);
        }
    }

    fclose(file);
    ini_mutex_unlock(&ctx_to_use->mutex);
    return INI_SUCCESS;
}

/**
 * @brief Gets a value from a section in the INI context.
 *
 * @param ctx Context to query
 * @param section Section name (empty string for global keys)
 * @param key Key name to retrieve
 * @param[out] value Output value (caller must free)
 * @return ini_error_t Error code
 */
INI_PUBLIC_API ini_error_t ini_get_value(ini_context_t const *ctx,
                                         char const *section,
                                         char const *key,
                                         char **value)
{
    if (!ctx || !key || !value)
    {
        return INI_INVALID_ARGUMENT;
    }

    // Default to empty string section if NULL
    if (!section)
    {
        section = "";
    }

    // Lock context for thread safety
    if (ini_mutex_lock((ini_mutex_t *)&ctx->mutex) != 0)
    {
        return INI_PLATFORM_ERROR;
    }

    // Get the section hash table
    ini_ht_t *section_ht = get_section_ht(ctx->sections, section);
    if (!section_ht)
    {
        ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
        return INI_SECTION_NOT_FOUND;
    }

    // Get the value from the section
    char const *found_value = ini_ht_get(section_ht, key);
    if (!found_value)
    {
        ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
        return INI_KEY_NOT_FOUND;
    }

    // Make a copy for the caller
    *value = ini_strdup(found_value);
    if (!*value)
    {
        ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
        return INI_MEMORY_ERROR;
    }

    ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
    return INI_SUCCESS;
}

INI_PUBLIC_API ini_error_t ini_save(ini_context_t const *ctx, char const *filepath)
{
    if (!ctx || !filepath)
    {
        return INI_INVALID_ARGUMENT;
    }

    // Check if file has write permission
    ini_file_permission_t perms = ini_get_file_permission(filepath);
    if (perms.write == 0)
        return INI_FILE_PERMISSION_DENIED;

    // Open file for writing
    FILE *file = ini_fopen(filepath, "w");
    if (!file)
        return INI_FILE_OPEN_FAILED;

    // Lock context for thread safety
    if (ini_mutex_lock((ini_mutex_t *)&ctx->mutex) != 0)
    {
        fclose(file);
        return INI_PLATFORM_ERROR;
    }

    // Iterate through all sections
    ini_ht_iterator_t sections_it = ini_ht_iterator((ini_ht_t *)ctx->sections);
    char *section_name;
    char *section_ptr_str;
    int first_section = 1;

    while (ini_ht_next(&sections_it, &section_name, &section_ptr_str) == INI_HT_SUCCESS)
    {
        ini_ht_t *section_ht = str_to_ptr(section_ptr_str);
        // Skip empty global section if it has no keys
        if (*section_name == '\0' && ini_ht_length(section_ht) == 0)
        {
            continue;
        }

        // Add newline between sections (except first)
        if (!first_section)
        {
            fprintf(file, "\n");
        }
        first_section = 0;

        // Write section header (except for global section)
        if (*section_name != '\0')
        {
            fprintf(file, "[%s]\n", section_name);
        }

        // Write key-value pairs
        ini_ht_iterator_t pairs_it = ini_ht_iterator(section_ht);
        char *key;
        char *value;

        while (ini_ht_next(&pairs_it, &key, &value) == INI_HT_SUCCESS)
        {
            // Determine if value needs quotes
            int need_quotes = 0;
            if (strchr(value, ' ') || strchr(value, '\t') ||
                strchr(value, ';') || strchr(value, '#'))
            {
                need_quotes = 1;
            }

            if (need_quotes)
            {
                fprintf(file, "%s=\"%s\"\n", key, value);
            }
            else
            {
                fprintf(file, "%s=%s\n", key, value);
            }
        }
    }

    ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
    if (fclose(file) != 0)
    {
        return INI_CLOSE_FAILED;
    }

    return INI_SUCCESS;
}

INI_PUBLIC_API ini_error_t ini_save_section_value(ini_context_t const *ctx,
                                                  char const *filepath,
                                                  char const *section,
                                                  char const *key)
{
    if (!ctx || !filepath || !section)
    {
        return INI_INVALID_ARGUMENT;
    }

    // Lock context for thread safety
    if (ini_mutex_lock((ini_mutex_t *)&ctx->mutex) != 0)
    {
        return INI_PLATFORM_ERROR;
    }

    // Find section
    ini_ht_t *section_ht = (ini_ht_t *)ini_ht_get(ctx->sections, section);
    if (!section_ht)
    {
        ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
        return INI_SECTION_NOT_FOUND;
    }

    // If key specified, check if it exists
    if (key)
    {
        char const *value = ini_ht_get(section_ht, key);
        if (!value)
        {
            ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
            return INI_KEY_NOT_FOUND;
        }
    }

    // Check if file exists
    FILE *existing_file = NULL;
    int file_exists = 0;

#if INI_OS_WINDOWS
    if (_access(filepath, F_OK) == 0)
    {
        file_exists = 1;
    }
#else
    if (access(filepath, F_OK) == 0)
    {
        file_exists = 1;
    }
#endif

    // Create a temporary file for output
    char temp_path[1024];
    FILE *temp_file;

#if INI_OS_WINDOWS
    // Create temporary file path
    if (tmpnam_s(temp_path, sizeof(temp_path)) != 0)
    {
        ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
        return INI_FILE_OPEN_FAILED;
    }

    temp_file = ini_fopen(temp_path, "w");
    if (!temp_file)
    {
        ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
        return INI_FILE_OPEN_FAILED;
    }
#else
    // On Unix, use tmpfile() which automatically handles cleanup
    temp_file = tmpfile();
    if (!temp_file)
    {
        ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
        return INI_FILE_OPEN_FAILED;
    }
#endif

    // If file exists, read it and update the specific section
    if (file_exists)
    {
        if (ini_fopen(filepath, "r") != 0 || !existing_file)
        {
            fclose(temp_file);
            ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
            return INI_FILE_OPEN_FAILED;
        }

        char line[INI_LINE_MAX];
        char current_section[INI_LINE_MAX] = "";
        int in_target_section = 0;
        int target_section_written = 0;

        while (fgets(line, sizeof(line), existing_file))
        {
            // Check if line is a section
            if (line[0] == '[')
            {
                char *end = strchr(line, ']');
                if (end)
                {
                    *end = '\0';
                    strcpy(current_section, line + 1);
                    *end = ']';

                    // Check if entering the target section
                    if (strcmp(current_section, section) == 0)
                    {
                        in_target_section = 1;
                        target_section_written = 1;

                        // Write section header
                        fprintf(temp_file, "%s", line);

                        // If key is NULL, write all keys from the section
                        if (!key)
                        {
                            ini_ht_iterator_t it = ini_ht_iterator(section_ht);
                            char *k, *v;

                            while (ini_ht_next(&it, &k, &v) == INI_HT_SUCCESS)
                            {
                                int need_quotes = 0;
                                if (strchr(v, ' ') || strchr(v, '\t') ||
                                    strchr(v, ';') || strchr(v, '#'))
                                {
                                    need_quotes = 1;
                                }

                                if (need_quotes)
                                {
                                    fprintf(temp_file, "%s=\"%s\"\n", k, v);
                                }
                                else
                                {
                                    fprintf(temp_file, "%s=%s\n", k, v);
                                }
                            }

                            // Skip processing until next section
                            in_target_section = 0;
                        }
                        continue;
                    }
                    else
                    {
                        // Exiting the target section
                        in_target_section = 0;
                    }
                }
            }
            // Handle key-value pairs in target section
            else if (in_target_section && key && strchr(line, '='))
            {
                char *eq = strchr(line, '=');
                if (eq)
                {
                    *eq = '\0';
                    char *line_key = line;

                    // Trim key
                    while (*line_key && (*line_key == ' ' || *line_key == '\t'))
                    {
                        line_key++;
                    }

                    char *end = eq - 1;
                    while (end > line_key && (*end == ' ' || *end == '\t'))
                    {
                        *end-- = '\0';
                    }

                    // If this is our target key, replace it
                    if (strcmp(line_key, key) == 0)
                    {
                        // Get value from context
                        char const *value = ini_ht_get(section_ht, key);

                        // Determine if quotes needed
                        int need_quotes = 0;
                        if (strchr(value, ' ') || strchr(value, '\t') ||
                            strchr(value, ';') || strchr(value, '#'))
                        {
                            need_quotes = 1;
                        }

                        if (need_quotes)
                        {
                            fprintf(temp_file, "%s=\"%s\"\n", key, value);
                        }
                        else
                        {
                            fprintf(temp_file, "%s=%s\n", key, value);
                        }
                        continue;
                    }

                    // Restore the equals sign for other keys
                    *eq = '=';
                }
            }

            // Write unmodified line
            fprintf(temp_file, "%s", line);
        }

        fclose(existing_file);

        // If section wasn't found in file, append it
        if (!target_section_written)
        {
            // Add newline before section if needed
            if (ftell(temp_file) > 0)
            {
                fprintf(temp_file, "\n");
            }

            // Write section header
            fprintf(temp_file, "[%s]\n", section);

            // Write keys
            if (key)
            {
                // Write specific key
                char const *value = ini_ht_get(section_ht, key);
                int need_quotes = 0;
                if (strchr(value, ' ') || strchr(value, '\t') ||
                    strchr(value, ';') || strchr(value, '#'))
                {
                    need_quotes = 1;
                }

                if (need_quotes)
                {
                    fprintf(temp_file, "%s=\"%s\"\n", key, value);
                }
                else
                {
                    fprintf(temp_file, "%s=%s\n", key, value);
                }
            }
            else
            {
                // Write all keys
                ini_ht_iterator_t it = ini_ht_iterator(section_ht);
                char *k, *v;

                while (ini_ht_next(&it, &k, &v) == INI_HT_SUCCESS)
                {
                    int need_quotes = 0;
                    if (strchr(v, ' ') || strchr(v, '\t') ||
                        strchr(v, ';') || strchr(v, '#'))
                    {
                        need_quotes = 1;
                    }

                    if (need_quotes)
                    {
                        fprintf(temp_file, "%s=\"%s\"\n", k, v);
                    }
                    else
                    {
                        fprintf(temp_file, "%s=%s\n", k, v);
                    }
                }
            }
        }
    }
    else
    {
        // File doesn't exist, create it with just this section
        fprintf(temp_file, "[%s]\n", section);

        // Write keys
        if (key)
        {
            // Write specific key
            char const *value = ini_ht_get(section_ht, key);
            int need_quotes = 0;
            if (strchr(value, ' ') || strchr(value, '\t') ||
                strchr(value, ';') || strchr(value, '#'))
            {
                need_quotes = 1;
            }

            if (need_quotes)
            {
                fprintf(temp_file, "%s=\"%s\"\n", key, value);
            }
            else
            {
                fprintf(temp_file, "%s=%s\n", key, value);
            }
        }
        else
        {
            // Write all keys
            ini_ht_iterator_t it = ini_ht_iterator(section_ht);
            char *k, *v;

            while (ini_ht_next(&it, &k, &v) == INI_HT_SUCCESS)
            {
                int need_quotes = 0;
                if (strchr(v, ' ') || strchr(v, '\t') ||
                    strchr(v, ';') || strchr(v, '#'))
                {
                    need_quotes = 1;
                }

                if (need_quotes)
                {
                    fprintf(temp_file, "%s=\"%s\"\n", k, v);
                }
                else
                {
                    fprintf(temp_file, "%s=%s\n", k, v);
                }
            }
        }
    }

    // Close temporary file and flush to disk
    fflush(temp_file);

#if INI_OS_WINDOWS
    fclose(temp_file);

    // Remove existing file and rename temp file
    if (file_exists && remove(filepath) != 0)
    {
        remove(temp_path);
        ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
        return INI_FILE_OPEN_FAILED;
    }

    if (rename(temp_path, filepath) != 0)
    {
        remove(temp_path);
        ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
        return INI_FILE_OPEN_FAILED;
    }
#else
    // On Unix, rewind the temp file and copy to real file
    rewind(temp_file);

    FILE *dest_file = ini_fopen(filepath, "w");
    if (!dest_file)
    {
        fclose(temp_file);
        ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
        return INI_FILE_OPEN_FAILED;
    }

    char buffer[INI_LINE_MAX];
    while (fgets(buffer, sizeof(buffer), temp_file))
    {
        fputs(buffer, dest_file);
    }

    fclose(temp_file);
    fclose(dest_file);
#endif

    ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
    return INI_SUCCESS;
}

INI_PUBLIC_API ini_error_t ini_print(FILE *stream, ini_context_t const *ctx)
{
    if (!stream || !ctx)
    {
        return INI_INVALID_ARGUMENT;
    }

    // Lock context for thread safety
    if (ini_mutex_lock((ini_mutex_t *)&ctx->mutex) != 0)
    {
        return INI_PLATFORM_ERROR;
    }

    // Iterate through all sections
    ini_ht_iterator_t sections_it = ini_ht_iterator((ini_ht_t *)ctx->sections);
    char *section_name;
    char *section_ptr_str;

    while (ini_ht_next(&sections_it, &section_name, &section_ptr_str) == INI_HT_SUCCESS)
    {
        ini_ht_t *section_ht = str_to_ptr(section_ptr_str);
        // Print section header (except for global section)
        if (*section_name != '\0')
        {
            if (fprintf(stream, "[%s]\n", section_name) < 0)
            {
                ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
                return INI_PRINT_ERROR;
            }
        }
        else
        {
            if (fprintf(stream, "[Global]\n") < 0)
            {
                ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
                return INI_PRINT_ERROR;
            }
        }

        // Print key-value pairs
        ini_ht_iterator_t pairs_it = ini_ht_iterator(section_ht);
        char *key;
        char *value;

        while (ini_ht_next(&pairs_it, &key, &value) == INI_HT_SUCCESS)
        {
            if (fprintf(stream, "  %s = %s\n", key, value) < 0)
            {
                ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
                return INI_PRINT_ERROR;
            }
        }

        if (fprintf(stream, "\n") < 0)
        {
            ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
            return INI_PRINT_ERROR;
        }
    }

    ini_mutex_unlock((ini_mutex_t *)&ctx->mutex);
    return INI_SUCCESS;
}
