#include <stdlib.h>
#include <string.h>

#include "ini_hash_table.h"
#include "ini_string.h"

#define INITIAL_CAPACITY 16 ///< Initial capacity for the hash table. Must be a power of 2.

uint64_t hash_key(char const *key)
{
    uint64_t hash = 14695981039346656037ULL;
    char const *p = key;
    while (*p)
    {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= 1099511628211ULL;
        ++p;
    }
    return hash;
}

ini_ht_t *ini_ht_create(void)
{
    ini_ht_t *table = malloc(sizeof(ini_ht_t));
    if (!table)
        return NULL;

    table->length = 0;
    table->capacity = INITIAL_CAPACITY;
    table->entries = calloc(table->capacity, sizeof(ini_ht_key_value_t));
    if (!table->entries)
    {
        free(table);
        return NULL;
    }

    if (ini_mutex_init(&table->mutex) != 0)
    {
        free(table->entries);
        free(table);
        return NULL;
    }

    return table;
}

void ini_ht_destroy(ini_ht_t *table)
{
    if (!table)
        return;

    for (size_t i = 0; i < table->capacity; i++)
    {
        free(table->entries[i].key);
        free(table->entries[i].value);
    }

    free(table->entries);
    ini_mutex_destroy(&table->mutex);
    free(table);
}

ini_ht_key_value_t *ht_get_entry(ini_ht_key_value_t *entries, size_t capacity, char const *key)
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

char const *ini_ht_get(ini_ht_t *table, char const *key)
{
    if (!table || !key)
        return NULL;

    ini_mutex_lock(&table->mutex);
    ini_ht_key_value_t *entry = ht_get_entry(table->entries, table->capacity, key);
    ini_mutex_unlock(&table->mutex);

    return entry ? entry->value : NULL;
}

int ht_set_entry(ini_ht_key_value_t *entries, size_t capacity,
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
                return -1;

            free(entries[index].value);
            entries[index].value = new_value;
            return 0;
        }
        index = (index + 1) % capacity;
    }

    char *new_key = ini_strdup(key);
    char *new_value = ini_strdup(value);
    if (!new_key || !new_value)
    {
        free(new_key);
        free(new_value);
        return -1;
    }

    entries[index].key = new_key;
    entries[index].value = new_value;
    if (plength)
        (*plength)++;
    return 0;
}

int ht_expand(ini_ht_t *table)
{
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity || new_capacity > SIZE_MAX / sizeof(ini_ht_key_value_t))
        return -1;

    ini_ht_key_value_t *new_entries = calloc(new_capacity, sizeof(ini_ht_key_value_t));
    if (!new_entries)
        return -1;

    for (size_t i = 0; i < table->capacity; i++)
    {
        if (table->entries[i].key)
        {
            if (ht_set_entry(new_entries, new_capacity, table->entries[i].key,
                             table->entries[i].value, NULL) != 0)
            {
                for (size_t j = 0; j < new_capacity; j++)
                {
                    free(new_entries[j].key);
                    free(new_entries[j].value);
                }
                free(new_entries);
                return -1;
            }
            free(table->entries[i].key);
            free(table->entries[i].value);
        }
    }

    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
    return 0;
}

char const *ini_ht_set(ini_ht_t *table, char const *key, char const *value)
{
    if (!table || !key || !value)
        return NULL;

    ini_mutex_lock(&table->mutex);

    if (table->length >= table->capacity / 2)
    {
        if (ht_expand(table) != 0)
        {
            ini_mutex_unlock(&table->mutex);
            return NULL;
        }
    }

    if (ht_set_entry(table->entries, table->capacity, key, value, &table->length) != 0)
    {
        ini_mutex_unlock(&table->mutex);
        return NULL;
    }

    ini_mutex_unlock(&table->mutex);
    return key;
}

size_t ini_ht_length(ini_ht_t *table)
{
    if (!table)
        return 0;

    ini_mutex_lock(&table->mutex);
    size_t len = table->length;
    ini_mutex_unlock(&table->mutex);

    return len;
}

ini_ht_iterator_t ini_ht_iterator(ini_ht_t *table)
{
    ini_ht_iterator_t it = {0};
    if (table)
    {
        it._table = table;
        it._index = 0;
    }
    return it;
}

int ini_ht_next(ini_ht_iterator_t *it, char **key, char **value)
{
    if (!it || !it->_table || !key || !value)
        return -1;

    ini_mutex_lock((ini_mutex_t *)&it->_table->mutex);

    while (it->_index < it->_table->capacity)
    {
        size_t i = it->_index++;
        if (it->_table->entries[i].key)
        {
            *key = it->_table->entries[i].key;
            *value = it->_table->entries[i].value;
            ini_mutex_unlock((ini_mutex_t *)&it->_table->mutex);
            return 0;
        }
    }

    ini_mutex_unlock((ini_mutex_t *)&it->_table->mutex);
    return -1;
}
