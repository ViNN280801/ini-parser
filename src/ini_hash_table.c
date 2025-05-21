#define INI_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>

#include "ini_hash_table.h"
#include "ini_string.h"

ini_ht_error_t __ini_details_ht_set_entry(ini_ht_key_value_t *entries, size_t capacity,
                                          char const *key, char const *value, size_t *plength);
ini_ht_error_t __ini_details_ht_expand(ini_ht_t *table);
ini_ht_key_value_t *__ini_details_ht_get_entry(ini_ht_key_value_t *entries,
                                               size_t capacity,
                                               char const *key);

INI_PUBLIC_API uint64_t hash_key(char const *key)
{
    uint64_t hash = 14695981039346656037ULL;

    if (key == NULL)
        return hash; // Return the initial hash value if the key is NULL

    char const *p = key;
    while (*p)
    {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= 1099511628211ULL;
        ++p;
    }
    return hash;
}

INI_PUBLIC_API ini_ht_t *ini_ht_create(void)
{
    ini_ht_t *table = malloc(sizeof(ini_ht_t));
    if (!table)
        return NULL;

    table->length = 0;
    table->capacity = INI_HT_INITIAL_CAPACITY;
    table->entries = calloc(table->capacity, sizeof(ini_ht_key_value_t));
    if (!table->entries)
    {
        if (table)
            free(table);
        return NULL;
    }

    if (ini_mutex_init(&table->mutex) != INI_MUTEX_SUCCESS)
    {
        if (table->entries)
            free(table->entries);
        if (table)
            free(table);
        return NULL;
    }

    return table;
}

INI_PUBLIC_API ini_ht_error_t ini_ht_destroy(ini_ht_t *table)
{
    if (!table || !table->entries)
        return INI_HT_INVALID_ARGUMENT;

    for (size_t i = 0; i < table->capacity; i++)
    {
        if (table->entries[i].key)
            free(table->entries[i].key);
        if (table->entries[i].value)
            free(table->entries[i].value);
    }

    if (table->entries)
        free(table->entries);

    ini_mutex_destroy(&table->mutex);
    if (table)
        free(table);

    return INI_HT_SUCCESS;
}

INI_PUBLIC_API char const *ini_ht_get(ini_ht_t *table, char const *key)
{
    if (!table || !key)
        return NULL;

    if (ini_mutex_lock(&table->mutex) != INI_MUTEX_SUCCESS)
        return NULL;

    ini_ht_key_value_t *entry = __ini_details_ht_get_entry(table->entries, table->capacity, key);
    if (ini_mutex_unlock(&table->mutex) != INI_MUTEX_SUCCESS)
        return NULL;

    return entry ? entry->value : NULL;
}

INI_PUBLIC_API char const *ini_ht_set(ini_ht_t *table, char const *key, char const *value)
{
    if (!table || !key)
        return NULL;

    if (ini_mutex_lock(&table->mutex) != INI_MUTEX_SUCCESS)
        return NULL;

    if (table->length >= table->capacity / 2)
    {
        if (__ini_details_ht_expand(table) != INI_HT_SUCCESS)
        {
            ini_mutex_unlock(&table->mutex);
            return NULL;
        }
    }

    if (__ini_details_ht_set_entry(table->entries, table->capacity, key, value, &table->length) != INI_HT_SUCCESS)
    {
        ini_mutex_unlock(&table->mutex);
        return NULL;
    }

    ini_mutex_unlock(&table->mutex);
    return value;
}

INI_PUBLIC_API size_t ini_ht_length(ini_ht_t *table)
{
    if (!table)
        return SIZE_MAX;

    if (ini_mutex_lock(&table->mutex) != INI_MUTEX_SUCCESS)
        return SIZE_MAX;

    size_t len = table->length;
    if (ini_mutex_unlock(&table->mutex) != INI_MUTEX_SUCCESS)
        return SIZE_MAX;

    return len;
}

INI_PUBLIC_API ini_ht_iterator_t ini_ht_iterator(ini_ht_t *table)
{
    ini_ht_iterator_t it = {0};
    if (table)
    {
        it._table = table;
        it._index = 0;
    }
    return it;
}

INI_PUBLIC_API ini_ht_error_t ini_ht_next(ini_ht_iterator_t *it, char **key, char **value)
{
    if (!it || !it->_table || !key || !value)
        return INI_HT_INVALID_ARGUMENT;

    if (ini_mutex_lock((ini_mutex_t *)&it->_table->mutex) != INI_MUTEX_SUCCESS)
        return INI_HT_MUTEX_ERROR;

    while (it->_index < it->_table->capacity)
    {
        size_t i = it->_index++;
        if (it->_table->entries[i].key)
        {
            *key = it->_table->entries[i].key;
            *value = it->_table->entries[i].value;
            if (ini_mutex_unlock((ini_mutex_t *)&it->_table->mutex) != INI_MUTEX_SUCCESS)
                return INI_HT_MUTEX_ERROR;
            return INI_HT_SUCCESS;
        }
    }

    if (ini_mutex_unlock((ini_mutex_t *)&it->_table->mutex) != INI_MUTEX_SUCCESS)
        return INI_HT_MUTEX_ERROR;

    return INI_HT_ITERATOR_END;
}

ini_ht_error_t __ini_details_ht_set_entry(ini_ht_key_value_t *entries, size_t capacity,
                                          char const *key, char const *value, size_t *plength)
{
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    while (entries[index].key != NULL)
    {
        if (strcmp(key, entries[index].key) == 0)
        {
            char *new_value = ini_strdup(value);
            if (!new_value)
                return INI_HT_MEMORY_ERROR;

            // Free the old value before replacing it
            if (entries[index].value)
                free(entries[index].value);

            entries[index].value = new_value;
            return INI_HT_SUCCESS;
        }
        index = (index + 1) % capacity;
    }

    char *new_key = ini_strdup(key);
    if (!new_key)
        return INI_HT_MEMORY_ERROR;

    char *new_value = ini_strdup(value);
    if (!new_value)
    {
        free(new_key);
        return INI_HT_MEMORY_ERROR;
    }

    entries[index].key = new_key;
    entries[index].value = new_value;
    if (plength)
        (*plength)++;

    return INI_HT_SUCCESS;
}

ini_ht_error_t __ini_details_ht_expand(ini_ht_t *table)
{
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity || new_capacity > SIZE_MAX / sizeof(ini_ht_key_value_t))
        return INI_HT_LACK_OF_MEMORY;

    ini_ht_key_value_t *new_entries = calloc(new_capacity, sizeof(ini_ht_key_value_t));
    if (!new_entries)
        return INI_HT_MEMORY_ERROR;

    for (size_t i = 0; i < table->capacity; i++)
    {
        if (table->entries[i].key)
        {
            if (__ini_details_ht_set_entry(new_entries, new_capacity, table->entries[i].key,
                                           table->entries[i].value, NULL) != INI_HT_SUCCESS)
            {
                for (size_t j = 0; j < new_capacity; j++)
                {
                    if (new_entries[j].key)
                        free(new_entries[j].key);
                    if (new_entries[j].value)
                        free(new_entries[j].value);
                }
                free(new_entries);
                return INI_HT_MEMORY_ERROR;
            }
        }
    }

    for (size_t i = 0; i < table->capacity; i++)
    {
        if (table->entries[i].key)
        {
            free(table->entries[i].key);
            free(table->entries[i].value);
        }
    }

    // Replace old table with new
    if (table->entries)
        free(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
    return INI_HT_SUCCESS;
}

ini_ht_key_value_t *__ini_details_ht_get_entry(ini_ht_key_value_t *entries,
                                               size_t capacity,
                                               char const *key)
{
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    while (entries[index].key != NULL)
    {
        if (strcmp(key, entries[index].key) == 0)
        {
            return &entries[index];
        }
        index = (index + 1) % capacity;
    }

    return NULL;
}
