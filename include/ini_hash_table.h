#ifndef INI_HASH_TABLE_H
#define INI_HASH_TABLE_H

#include <stddef.h>
#include <stdint.h>

#include "ini_constants.h"
#include "ini_export.h"
#include "ini_mutex.h"

INI_EXTERN_C_BEGIN

/**
 * @brief Key-value pair for hash table entries.
 */
typedef struct
{
    char *key;   ///< Null-terminated string key.
    char *value; ///< Null-terminated string value.
} ini_ht_key_value_t;

/**
 * @brief Hash table structure.
 * @note Thread-safe if used with `ini_mutex_t`.
 */
typedef struct
{
    ini_ht_key_value_t *entries; ///< Array of key-value pairs.
    size_t capacity;             ///< Total slots in the table.
    size_t length;               ///< Number of active entries.
    ini_mutex_t mutex;           ///< Mutex for thread safety.
} ini_ht_t;

/**
 * @brief Iterator for traversing hash table entries.
 */
typedef struct
{
    ini_ht_t const *_table; ///< Pointer to the hash table.
    size_t _index;          ///< Current iteration index.
} ini_ht_iterator_t;

/**
 * @brief FNV-1a hash function for string keys.
 * @param key Null-terminated string to hash.
 * @return 64-bit hash value.
 * @see https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
 */
INI_PUBLIC_API uint64_t hash_key(char const *key);

/**
 * @brief Creates a new hash table.
 * @return Pointer to the table, or NULL on failure.
 * @post Check `ini_ht_last_error()` for failure details.
 */
INI_PUBLIC_API ini_ht_t *ini_ht_create(void);

/**
 * @brief Destroys a hash table and frees all resources.
 * @param table Table to destroy (safe to call with NULL).
 */
INI_PUBLIC_API ini_status_t ini_ht_destroy(ini_ht_t *table);

/**
 * @brief Retrieves a value by key.
 * @param table Hash table to query.
 * @param key Null-terminated string key.
 * @return Associated value, or NULL if key not found.
 * @note Thread-safe (uses mutex locking).
 */
INI_PUBLIC_API char const *ini_ht_get(ini_ht_t *table, char const *key);

/**
 * @brief Inserts or updates a key-value pair.
 * @param table Hash table to modify.
 * @param key Null-terminated string key (copied internally).
 * @param value Null-terminated string value (copied internally).
 * @return Previous value (if key existed), or NULL.
 * @note Thread-safe (uses mutex locking).
 */
INI_PUBLIC_API char const *ini_ht_set(ini_ht_t *table, char const *key, char const *value);

/**
 * @brief Returns the number of entries in the table.
 * @param table Hash table to query.
 * @return Entry count.
 */
INI_PUBLIC_API size_t ini_ht_length(ini_ht_t *table);

/**
 * @brief Initializes an iterator for the table.
 * @param table Hash table to iterate over.
 * @return Iterator positioned at the first entry.
 */
INI_PUBLIC_API ini_ht_iterator_t ini_ht_iterator(ini_ht_t *table);

/**
 * @brief Advances the iterator to the next entry.
 * @param it Iterator to advance.
 * @param[out] key Set to the current entry's key (do not free).
 * @param[out] value Set to the current entry's value (do not free).
 * @return 0 on success, -1 if no more entries.
 */
INI_PUBLIC_API ini_status_t ini_ht_next(ini_ht_iterator_t *it, char **key, char **value);

INI_EXTERN_C_END

#endif // !INI_HASH_TABLE_H
