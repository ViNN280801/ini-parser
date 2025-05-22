#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <stdio.h>

#include "ini_hash_table.h"

#ifndef INI_PARSER_VERSION
#define INI_PARSER_VERSION "1.0.0"
#endif

#define INI_LINE_MAX 8192
#define INI_BUFFER_SIZE 2048

INI_EXTERN_C_BEGIN

/// @brief Error codes returned by the INI parser.
typedef enum
{
    INI_SUCCESS = 0,            ///< Operation succeeded.
    INI_FILE_NOT_FOUND,         ///< Specified file does not exist.
    INI_FILE_EMPTY,             ///< File is empty.
    INI_FILE_OPEN_FAILED,       ///< Failed to open the file.
    INI_FILE_BAD_FORMAT,        ///< File has invalid INI syntax.
    INI_MEMORY_ERROR,           ///< Failed to allocate/reallocate/free memory.
    INI_SECTION_NOT_FOUND,      ///< Requested section does not exist.
    INI_KEY_NOT_FOUND,          ///< Requested key does not exist.
    INI_INVALID_ARGUMENT,       ///< Invalid argument passed to a function.
    INI_PLATFORM_ERROR,         ///< Platform-specific error (e.g., mutex failure).
    INI_CLOSE_FAILED,           ///< Failed to close the file.
    INI_PRINT_ERROR,            ///< Error during printing/formatting.
    INI_FILE_PERMISSION_DENIED, ///< Permission denied (e.g. read-only file)
    INI_FILE_IS_DIR,            ///< Path points to a directory
    INI_UNKNOWN_ERROR           ///< Unknown error
} ini_error_t;

/**
 * @brief Converts an `ini_error_t` to a human-readable string.
 * @param error The error code to convert.
 * @return A static string describing the error.
 */
INI_PUBLIC_API char const *ini_error_to_string(ini_error_t error);

/// @brief Represents an INI context using nested hash tables.
typedef struct
{
    ini_ht_t *sections; ///< Top-level hash table: section_name → (ini_ht_t* of key-value pairs).
    ini_mutex_t mutex;  ///< Mutex for thread safety.
} ini_context_t;

/**
 * @brief Initializes a new INI parser context.
 *
 * Creates a thread-safe context for parsing INI files, including:
 * - A top-level hash table for sections.
 * - A mutex for thread safety.
 *
 * @return Pointer to the newly created context, or NULL on failure.
 * @note On failure, check `ini_ht_last_error()` for details (e.g., memory allocation errors).
 * @warning The caller is responsible for freeing the context with `ini_free()`.
 */
INI_PUBLIC_API ini_context_t *ini_create_context();

/**
 * @brief Finalizes and frees an INI parser context.
 *
 * Safely deallocates all resources, including:
 * - Nested hash tables for sections and key-value pairs.
 * - The mutex.
 *
 * @param ctx Context to free (safe to call with NULL).
 * @return INI_SUCCESS on success, or INI_MEMORY_ERROR/INI_PLATFORM_ERROR on failure.
 * @note Thread-safe: Locks the mutex before cleanup.
 */
INI_PUBLIC_API ini_error_t ini_free(ini_context_t *ctx);

/**
 * @brief Validates an INI file's existence, accessibility, and basic format.
 *
 * Checks if the file:
 * 1. Exists and is accessible
 * 2. Is a regular file (not a directory)
 * 3. Is not empty
 * 4. Has valid INI syntax (sections and key-value pairs)
 *
 * @param filepath Path to the INI file to validate
 * @return INI_SUCCESS if valid, or appropriate error code
 * @note Thread-safe: Uses no shared resources
 */
INI_PUBLIC_API ini_error_t ini_good(char const *filepath);

/**
 * @brief Loads an INI file into a context.
 * @param[in, out] ctx The context to populate. Assumes it is NULL.
 *                     It will be initialized with ini_create_context() if it is NULL.
 *                     Otherwise, it will be freed with ini_free() before loading the new file.
 * @param[in] filepath Path to the INI file.
 * @return Error details (INI_SUCCESS on success).
 * @note Thread-safe: Uses mutex/semaphore internally.
 */
INI_PUBLIC_API ini_error_t ini_load(ini_context_t *ctx, char const *filepath);

/**
 * @brief Gets a value from a section in the INI context.
 * @param ctx Context to query.
 * @param section Section name (NULL for global keys).
 * @param key Key name, for example: "gui" or "gui.mainwindow"
 * @param[out] value Retrieved value (caller must free).
 * @return Error details (INI_SUCCESS on success).
 * @note Thread-safe: Uses mutex/semaphore internally.
 */
INI_PUBLIC_API ini_error_t ini_get_value(ini_context_t const *ctx,
                                         char const *section,
                                         char const *key,
                                         char **value);

/**
 * @brief Saves an INI context to a file.
 * @param ctx Context to save.
 * @param filepath Path to save to.
 * @return Error details (INI_SUCCESS on success).
 * @note Thread-safe: Uses mutex/semaphore internally.
 */
INI_PUBLIC_API ini_error_t ini_save(ini_context_t const *ctx, char const *filepath);

/**
 * @brief Saves a specific section and key/value pair to a file.
 * @param ctx Context to query.
 * @param filepath Path to save to.
 * @param section Section name.
 * @param key Key name to save, or NULL to save all keys in the section.
 * @return Error details (INI_SUCCESS on success).
 * @note Thread-safe: Uses mutex/semaphore internally.
 * @note If the file already exists, only the specified section/key will be updated,
 *       preserving all other content.
 */
INI_PUBLIC_API ini_error_t ini_save_section_value(ini_context_t const *ctx,
                                                  char const *filepath,
                                                  char const *section,
                                                  char const *key);

/**
 * @brief Prints the INI context contents (for debugging).
 * @param stream Stream to print to. Example: stderr, stdout, file, etc.
 * @param ctx Context to print.
 * @return Error details (INI_SUCCESS on success).
 */
INI_PUBLIC_API ini_error_t ini_print(FILE *stream, ini_context_t const *ctx);

INI_EXTERN_C_END

#endif // !INI_PARSER_H
