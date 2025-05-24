#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <stdio.h>

#include "ini_hash_table.h"

INI_EXTERN_C_BEGIN

/// @brief Represents an INI context using nested hash tables.
typedef struct
{
    ini_ht_t *sections; ///< Top-level hash table: section_name → (ini_ht_t* of key-value pairs).
    ini_mutex_t mutex;  ///< Mutex for thread safety.
} ini_context_t;

/**
 * @brief Helper functions to store and retrieve section hash tables.
 * @param sections Top-level hash table.
 * @param section_name Section name.
 * @param section_ht Section hash table.
 * @return Error details (INI_SUCCESS on success).
 */
INI_PUBLIC_API void ini_store_section_ht(ini_ht_t *sections, char const *section_name, ini_ht_t *section_ht);

/**
 * @brief Retrieves a section hash table from the top-level hash table.
 * @param sections Top-level hash table.
 * @param section_name Section name.
 * @return Section hash table, or NULL if not found.
 */
INI_PUBLIC_API ini_ht_t *ini_get_section_ht(ini_ht_t *sections, char const *section_name);

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
INI_PUBLIC_API ini_status_t ini_free(ini_context_t *ctx);

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
INI_PUBLIC_API ini_status_t ini_good(char const *filepath);

/**
 * @brief Loads an INI file into a context.
 * @param[in, out] ctx The context to populate. Assumes it is NULL.
 *                     It will be initialized with ini_create_context() if it is NULL.
 *                     Otherwise, it will be freed with ini_free() before loading the new file.
 * @param[in] filepath Path to the INI file.
 * @return Error details (INI_SUCCESS on success).
 * @note Thread-safe: Uses mutex/semaphore internally.
 */
INI_PUBLIC_API ini_status_t ini_load(ini_context_t *ctx, char const *filepath);

/**
 * @brief Gets a value from a section in the INI context.
 * @param ctx Context to query.
 * @param section Section name (NULL for global keys).
 * @param key Key name, for example: "gui" or "gui.mainwindow"
 * @param[out] value Retrieved value (caller must free).
 * @return Error details (INI_SUCCESS on success).
 * @note Thread-safe: Uses mutex/semaphore internally.
 */
INI_PUBLIC_API ini_status_t ini_get_value(ini_context_t const *ctx,
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
INI_PUBLIC_API ini_status_t ini_save(ini_context_t const *ctx, char const *filepath);

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
INI_PUBLIC_API ini_status_t ini_save_section_value(ini_context_t const *ctx,
                                                   char const *filepath,
                                                   char const *section,
                                                   char const *key);

/**
 * @brief Prints the INI context contents (for debugging).
 * @param stream Stream to print to. Example: stderr, stdout, file, etc.
 * @param ctx Context to print.
 * @return Error details (INI_SUCCESS on success).
 */
INI_PUBLIC_API ini_status_t ini_print(FILE *stream, ini_context_t const *ctx);

INI_EXTERN_C_END

#endif // !INI_PARSER_H
