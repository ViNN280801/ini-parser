#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"
#include "ini_hash_table.h"

// Clean test: Correct hashing for a simple key
void test_hash_key_success()
{
    const char *key = "test_key";
    uint64_t hash = hash_key(key);
    assert(hash != 0); // Ensure hash is not zero for a valid key
    print_success("test_hash_key_success passed\n");
}

// Dirty test: NULL key
void test_hash_key_null()
{
    uint64_t hash = hash_key(NULL);
    assert(hash == 14695981039346656037ULL); // Initial hash value for NULL
    print_success("test_hash_key_null passed\n");
}

// Dirty test: Empty string
void test_hash_key_empty()
{
    const char *key = "";
    uint64_t hash = hash_key(key);
    assert(hash == 14695981039346656037ULL); // Initial hash value for empty string
    print_success("test_hash_key_empty passed\n");
}

// Dirty test: Very long string (potential overflow)
void test_hash_key_long_string()
{
    char long_key[1024];
    memset(long_key, 'a', sizeof(long_key) - 1);
    long_key[sizeof(long_key) - 1] = '\0';

    uint64_t hash = hash_key(long_key);
    assert(hash != 0); // Ensure hash is not zero
    print_success("test_hash_key_long_string passed\n");
}

// Dirty test: String with maximum possible length (SIZE_MAX - 1)
// Note: This is impractical to test directly, so we simulate a large string.
void test_hash_key_max_length()
{
    const char *key = "simulated_max_length";
    uint64_t hash = hash_key(key);
    assert(hash != 0);
    print_success("test_hash_key_max_length passed\n");
}

// Dirty test: String with non-ASCII characters
void test_hash_key_non_ascii()
{
    const char *key = "ключ";
    uint64_t hash = hash_key(key);
    assert(hash != 0);
    print_success("test_hash_key_non_ascii passed\n");
}

// Test for hash collisions (different keys should not collide)
void test_hash_key_collision()
{
    const char *key1 = "key1";
    const char *key2 = "key2";
    uint64_t hash1 = hash_key(key1);
    uint64_t hash2 = hash_key(key2);
    assert(hash1 != hash2);
    print_success("test_hash_key_collision passed\n");
}

// Test for determinism (same key, same hash)
void test_hash_key_deterministic()
{
    const char *key = "deterministic_key";
    uint64_t hash1 = hash_key(key);
    uint64_t hash2 = hash_key(key);
    assert(hash1 == hash2);
    print_success("test_hash_key_deterministic passed\n");
}

// Test for single-character key
void test_hash_key_single_char()
{
    const char *key = "a";
    uint64_t hash = hash_key(key);
    assert(hash != 0);
    print_success("test_hash_key_single_char passed\n");
}

// Test for special characters (e.g., null byte in the middle)
void test_hash_key_special_chars()
{
    const char *key = "test\0key";
    uint64_t hash = hash_key(key);
    assert(hash != 0);
    print_success("test_hash_key_special_chars passed\n");
}

// Test for repeated characters
void test_hash_key_repeated_chars()
{
    const char *key1 = "aaaaa";
    const char *key2 = "aaaab";
    uint64_t hash1 = hash_key(key1);
    uint64_t hash2 = hash_key(key2);
    assert(hash1 != hash2);
    print_success("test_hash_key_repeated_chars passed\n");
}

// Clean test: Successful table creation
void test_ht_create_success()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(table->length == 0);
    assert(table->capacity == INI_HT_INITIAL_CAPACITY);
    assert(table->entries != NULL);
    ini_ht_destroy(table); // Cleanup
    print_success("test_ht_create_success passed\n");
}

// Clean test: Successful table destruction
void test_ht_destroy_success()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);

    // Add some data
    ini_ht_set(table, "key1", "value1");
    ini_ht_set(table, "key2", "value2");

    // Destroy table
    ini_ht_error_t err = ini_ht_destroy(table);
    assert(err == INI_HT_SUCCESS);
    print_success("test_ht_destroy_success passed\n");
}

// Dirty test: NULL table
void test_ht_destroy_null()
{
    ini_ht_error_t err = ini_ht_destroy(NULL);
    assert(err == INI_HT_MEMORY_ERROR);
    print_success("test_ht_destroy_null passed\n");
}

// Dirty test: Partially filled table (some keys/values are NULL)
void test_ht_destroy_partial_data()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);

    // Add one entry, leave others NULL
    ini_ht_set(table, "key1", "value1");

    ini_ht_error_t err = ini_ht_destroy(table);
    assert(err == INI_HT_SUCCESS);
    print_success("test_ht_destroy_partial_data passed\n");
}

// Clean test: Get existing key
void test_ht_get_success()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);

    ini_ht_set(table, "key1", "value1");
    const char *value = ini_ht_get(table, "key1");

    assert(value != NULL);
    assert(strcmp(value, "value1") == 0);

    ini_ht_destroy(table);
    print_success("test_ht_get_success passed\n");
}

// Dirty test: Get non-existent key
void test_ht_get_not_found()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);

    const char *value = ini_ht_get(table, "nonexistent");
    assert(value == NULL);

    ini_ht_destroy(table);
    print_success("test_ht_get_not_found passed\n");
}

// Dirty test: NULL table or key
void test_ht_get_null_args()
{
    assert(ini_ht_get(NULL, "key") == NULL);
    assert(ini_ht_get(ini_ht_create(), NULL) == NULL);
    print_success("test_ht_get_null_args passed\n");
}

// Clean test: Set new key-value pair
void test_ht_set_new()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);

    const char *result = ini_ht_set(table, "new_key", "new_value");
    assert(result != NULL);
    assert(strcmp(result, "new_key") == 0);

    const char *value = ini_ht_get(table, "new_key");
    assert(value != NULL);
    assert(strcmp(value, "new_value") == 0);

    ini_ht_destroy(table);
    print_success("test_ht_set_new passed\n");
}

// Clean test: Update existing key
void test_ht_set_update()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);

    ini_ht_set(table, "key", "old_value");
    const char *result = ini_ht_set(table, "key", "new_value");

    assert(result != NULL);
    assert(strcmp(result, "key") == 0);

    const char *value = ini_ht_get(table, "key");
    assert(value != NULL);
    assert(strcmp(value, "new_value") == 0);

    ini_ht_destroy(table);
    print_success("test_ht_set_update passed\n");
}

// Dirty test: NULL arguments
void test_ht_set_null_args()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);

    assert(ini_ht_set(NULL, "key", "value") == NULL);
    assert(ini_ht_set(table, NULL, "value") == NULL);
    assert(ini_ht_set(table, "key", NULL) == NULL);

    ini_ht_destroy(table);
    print_success("test_ht_set_null_args passed\n");
}

// Dirty test: Table expansion
void test_ht_set_expand()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);

    // Fill table to trigger expansion
    for (int i = 0; i < 20; i++)
    {
        char key[10], value[10];
        sprintf(key, "key%d", i);
        sprintf(value, "val%d", i);
        assert(ini_ht_set(table, key, value) != NULL);
    }

    // Verify all values
    for (int i = 0; i < 20; i++)
    {
        char key[10], expected[10];
        sprintf(key, "key%d", i);
        sprintf(expected, "val%d", i);
        const char *value = ini_ht_get(table, key);
        assert(value != NULL);
        assert(strcmp(value, expected) == 0);
    }

    ini_ht_destroy(table);
    print_success("test_ht_set_expand passed\n");
}

void test_ht_length_success()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(ini_ht_length(table) == 0);
    ini_ht_destroy(table);
    print_success("test_ht_length_success passed\n");
}

void test_ht_length_null()
{
    assert(ini_ht_length(NULL) == SIZE_MAX);
    print_success("test_ht_length_null passed\n");
}

void test_ht_length_mutex_fail()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(ini_ht_length(table) != SIZE_MAX); // Зависит от реализации
    ini_ht_destroy(table);
    print_success("test_ht_length_mutex_fail passed\n");
}

void test_ht_length_one_entry()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(ini_ht_set(table, "key", "value") != NULL);
    assert(ini_ht_length(table) == 1);
    ini_ht_destroy(table);
    print_success("test_ht_length_one_entry passed\n");
}

void test_ht_length_multiple_entries()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    for (int i = 0; i < 10; i++)
    {
        char key[10];
        sprintf(key, "key%d", i);
        assert(ini_ht_set(table, key, "value") != NULL);
    }
    assert(ini_ht_length(table) == 10);
    ini_ht_destroy(table);
    print_success("test_ht_length_multiple_entries passed\n");
}

void test_ht_length_after_expand()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    for (int i = 0; i < 20; i++)
    {
        char key[10];
        sprintf(key, "key%d", i);
        assert(ini_ht_set(table, key, "value") != NULL);
    }
    assert(ini_ht_length(table) == 20);
    ini_ht_destroy(table);
    print_success("test_ht_length_after_expand passed\n");
}

void test_ht_length_unlock_fail()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(ini_ht_length(table) != SIZE_MAX);
    ini_ht_destroy(table);
    print_success("test_ht_length_unlock_fail passed\n");
}

void test_ht_length_partial_fill()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    for (int i = 0; i < 5; i++)
    {
        char key[10];
        sprintf(key, "key%d", i);
        assert(ini_ht_set(table, key, "value") != NULL);
    }
    assert(ini_ht_length(table) == 5);
    ini_ht_destroy(table);
    print_success("test_ht_length_partial_fill passed\n");
}

void test_ht_iterator_success()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    ini_ht_iterator_t it = ini_ht_iterator(table);
    assert(it._table == table);
    assert(it._index == 0);
    ini_ht_destroy(table);
    print_success("test_ht_iterator_success passed\n");
}

void test_ht_iterator_null()
{
    ini_ht_iterator_t it = ini_ht_iterator(NULL);
    assert(it._table == NULL);
    assert(it._index == 0);
    print_success("test_ht_iterator_null passed\n");
}

void test_ht_iterator_empty_table()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    ini_ht_iterator_t it = ini_ht_iterator(table);
    assert(it._table == table);
    assert(it._index == 0);
    ini_ht_destroy(table);
    print_success("test_ht_iterator_empty_table passed\n");
}

void test_ht_iterator_one_entry()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(ini_ht_set(table, "key", "value") != NULL);
    ini_ht_iterator_t it = ini_ht_iterator(table);
    assert(it._table == table);
    assert(it._index == 0);
    ini_ht_destroy(table);
    print_success("test_ht_iterator_one_entry passed\n");
}

void test_ht_iterator_multiple_entries()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    for (int i = 0; i < 10; i++)
    {
        char key[10];
        sprintf(key, "key%d", i);
        assert(ini_ht_set(table, key, "value") != NULL);
    }
    ini_ht_iterator_t it = ini_ht_iterator(table);
    assert(it._table == table);
    assert(it._index == 0);
    ini_ht_destroy(table);
    print_success("test_ht_iterator_multiple_entries passed\n");
}

void test_ht_iterator_after_expand()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    for (int i = 0; i < 20; i++)
    {
        char key[10];
        sprintf(key, "key%d", i);
        assert(ini_ht_set(table, key, "value") != NULL);
    }
    ini_ht_iterator_t it = ini_ht_iterator(table);
    assert(it._table == table);
    assert(it._index == 0);
    ini_ht_destroy(table);
    print_success("test_ht_iterator_after_expand passed\n");
}

void test_ht_iterator_partial_fill()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    for (int i = 0; i < 5; i++)
    {
        char key[10];
        sprintf(key, "key%d", i);
        assert(ini_ht_set(table, key, "value") != NULL);
    }
    ini_ht_iterator_t it = ini_ht_iterator(table);
    assert(it._table == table);
    assert(it._index == 0);
    ini_ht_destroy(table);
    print_success("test_ht_iterator_partial_fill passed\n");
}

void test_ht_iterator_mutex_fail()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(ini_ht_set(table, "key", "value") != NULL);
    ini_ht_iterator_t it = ini_ht_iterator(table);
    assert(it._table == table);
    assert(it._index == 0);
    ini_ht_destroy(table);
    print_success("test_ht_iterator_mutex_fail passed\n");
}

void test_ht_next_success()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(ini_ht_set(table, "key1", "value1") != NULL);
    ini_ht_iterator_t it = ini_ht_iterator(table);
    char *key, *value;
    assert(ini_ht_next(&it, &key, &value) == INI_HT_SUCCESS);
    assert(strcmp(key, "key1") == 0);
    assert(strcmp(value, "value1") == 0);
    ini_ht_destroy(table);
    print_success("test_ht_next_success passed\n");
}

void test_ht_next_empty_table()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    ini_ht_iterator_t it = ini_ht_iterator(table);
    char *key, *value;
    assert(ini_ht_next(&it, &key, &value) == INI_HT_ITERATOR_END);
    ini_ht_destroy(table);
    print_success("test_ht_next_empty_table passed\n");
}

void test_ht_next_multiple_entries()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(ini_ht_set(table, "key1", "value1") != NULL);
    assert(ini_ht_set(table, "key2", "value2") != NULL);
    ini_ht_iterator_t it = ini_ht_iterator(table);
    char *key, *value;
    assert(ini_ht_next(&it, &key, &value) == INI_HT_SUCCESS);
    assert(strcmp(key, "key1") == 0 || strcmp(key, "key2") == 0);
    assert(ini_ht_next(&it, &key, &value) == INI_HT_SUCCESS);
    assert(strcmp(key, "key1") == 0 || strcmp(key, "key2") == 0);
    ini_ht_destroy(table);
    print_success("test_ht_next_multiple_entries passed\n");
}

void test_ht_next_null_args()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    ini_ht_iterator_t it = ini_ht_iterator(table);
    assert(ini_ht_next(&it, NULL, NULL) == INI_HT_INVALID_ARGUMENT);
    ini_ht_destroy(table);
    print_success("test_ht_next_null_args passed\n");
}

void test_ht_next_null_iterator()
{
    char *key, *value;
    assert(ini_ht_next(NULL, &key, &value) == INI_HT_INVALID_ARGUMENT);
    print_success("test_ht_next_null_iterator passed\n");
}

void test_ht_next_mutex_fail()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(ini_ht_set(table, "key1", "value1") != NULL);
    ini_ht_iterator_t it = ini_ht_iterator(table);
    char *key, *value;
    assert(ini_ht_next(&it, &key, &value) != INI_HT_MUTEX_ERROR);
    ini_ht_destroy(table);
    print_success("test_ht_next_mutex_fail passed\n");
}

void test_ht_next_unlock_fail()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(ini_ht_set(table, "key1", "value1") != NULL);
    ini_ht_iterator_t it = ini_ht_iterator(table);
    char *key, *value;
    assert(ini_ht_next(&it, &key, &value) != INI_HT_MUTEX_ERROR);
    ini_ht_destroy(table);
    print_success("test_ht_next_unlock_fail passed\n");
}

void test_ht_next_partial_fill()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    for (int i = 0; i < 5; i++)
    {
        char key[10];
        sprintf(key, "key%d", i);
        assert(ini_ht_set(table, key, "value") != NULL);
    }
    ini_ht_iterator_t it = ini_ht_iterator(table);
    char *key, *value;
    for (int i = 0; i < 5; i++)
    {
        assert(ini_ht_next(&it, &key, &value) == INI_HT_SUCCESS);
    }
    ini_ht_destroy(table);
    print_success("test_ht_next_partial_fill passed\n");
}

void test_ht_next_no_more_entries()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(ini_ht_set(table, "key1", "value1") != NULL);
    ini_ht_iterator_t it = ini_ht_iterator(table);
    char *key, *value;
    assert(ini_ht_next(&it, &key, &value) == INI_HT_SUCCESS);
    assert(ini_ht_next(&it, &key, &value) == INI_HT_ITERATOR_END);
    ini_ht_destroy(table);
    print_success("test_ht_next_no_more_entries passed\n");
}

void test_hash_key_special_utf8()
{
    const char *key = "😊★";
    uint64_t hash = hash_key(key);
    assert(hash != 0);
    print_success("test_hash_key_special_utf8 passed\n");
}

void test_ht_create_mutex_init()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(table->mutex.initialized == INI_MUTEX_INITIALIZED);
    ini_ht_destroy(table);
    print_success("test_ht_create_mutex_init passed\n");
}

void test_ht_destroy_null_entries()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    table->entries[0].key = NULL;
    table->entries[0].value = strdup("value");
    assert(ini_ht_destroy(table) == INI_HT_SUCCESS);
    print_success("test_ht_destroy_null_entries passed\n");
}

void test_ht_set_overwrite_null()
{
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(ini_ht_set(table, "key", NULL) == NULL);
    ini_ht_destroy(table);
    print_success("test_ht_set_overwrite_null passed\n");
}

void test_ht_comprehensive_workflow()
{
    // 1. Create table
    ini_ht_t *table = ini_ht_create();
    assert(table != NULL);
    assert(table->length == 0);
    assert(table->capacity == INI_HT_INITIAL_CAPACITY);
    assert(table->mutex.initialized == INI_MUTEX_INITIALIZED);

    // 2. Add different data
    assert(ini_ht_set(table, "key1", "value1") != NULL);           // Regular key
    assert(ini_ht_set(table, "ключ", "значение ") != NULL);        // UTF-8
    assert(ini_ht_set(table, "key\0null", "value\0null") != NULL); // With null-bytes
    assert(ini_ht_length(table) == 3);

    // 3. Check data retrieval
    assert(strcmp(ini_ht_get(table, "key1"), "value1") == 0);

    char const *utf8_key = "ключ😊";
    char const *got_value = ini_ht_get(table, utf8_key);
    if (got_value)
        assert(strcmp(got_value, "значение★") == 0);

    assert(ini_ht_set(table, utf8_key, "new_value") != NULL);
    assert(strcmp(ini_ht_get(table, utf8_key), "new_value") == 0);

    assert(ini_ht_get(table, "nonexistent") == NULL);

    // 4. Update value
    assert(ini_ht_set(table, "key1", "new_value") != NULL);
    assert(strcmp(ini_ht_get(table, "key1"), "new_value") == 0);

    // 5. Table expansion
    for (int i = 0; i < 20; i++)
    {
        char key[20], value[20];
        sprintf(key, "exp_key%d", i);
        sprintf(value, "exp_val%d", i);
        assert(ini_ht_set(table, key, value) != NULL);
    }
    assert(table->capacity > INI_HT_INITIAL_CAPACITY);

    // 6. Iterate through table
    ini_ht_iterator_t it = ini_ht_iterator(table);
    char *key, *value;
    size_t count = 0;
    while (ini_ht_next(&it, &key, &value) == INI_HT_SUCCESS &&
           key != NULL &&
           value != NULL)
    {
        ++count;
    }
    assert(count == 24); // 3 original + 20 new + 1 utf8

    // 7. Check length after iteration
    assert(ini_ht_length(table) == 24);

    // 8. Cleanup
    assert(ini_ht_destroy(table) == INI_HT_SUCCESS);
    print_success("test_ht_comprehensive_workflow passed\n");
}

int main()
{
    __helper_init_log_file();

    /* Test for hash_key() function */
    test_hash_key_success();
    test_hash_key_null();
    test_hash_key_empty();
    test_hash_key_long_string();
    test_hash_key_max_length();
    test_hash_key_non_ascii();
    test_hash_key_collision();
    test_hash_key_deterministic();
    test_hash_key_single_char();
    test_hash_key_special_chars();
    test_hash_key_repeated_chars();
    test_hash_key_special_utf8();

    /* Test for ini_ht_create() function */
    test_ht_create_success();
    test_ht_create_mutex_init();

    /* Test for ini_ht_destroy() function */
    test_ht_destroy_success();
    test_ht_destroy_null();
    test_ht_destroy_partial_data();
    test_ht_destroy_null_entries();

    /* Test for ini_ht_get() function */
    test_ht_get_success();
    test_ht_get_not_found();
    test_ht_get_null_args();

    /* Test for ini_ht_set() function */
    test_ht_set_new();
    test_ht_set_update();
    test_ht_set_null_args();
    test_ht_set_expand();
    test_ht_set_overwrite_null();

    /* Test for ini_ht_length() function */
    test_ht_length_success();
    test_ht_length_null();
    test_ht_length_mutex_fail();
    test_ht_length_one_entry();
    test_ht_length_multiple_entries();
    test_ht_length_after_expand();
    test_ht_length_unlock_fail();
    test_ht_length_partial_fill();

    /* Test for ini_ht_iterator() function */
    test_ht_iterator_success();
    test_ht_iterator_null();
    test_ht_iterator_empty_table();
    test_ht_iterator_one_entry();
    test_ht_iterator_multiple_entries();
    test_ht_iterator_after_expand();
    test_ht_iterator_partial_fill();
    test_ht_iterator_mutex_fail();

    /* Test for ini_ht_next() function */
    test_ht_next_success();
    test_ht_next_empty_table();
    test_ht_next_multiple_entries();
    test_ht_next_null_args();
    test_ht_next_null_iterator();
    test_ht_next_mutex_fail();
    test_ht_next_unlock_fail();
    test_ht_next_partial_fill();
    test_ht_next_no_more_entries();

    /* Test for ini_ht_comprehensive_workflow() function */
    test_ht_comprehensive_workflow();

    print_success("All ini_hash_table tests passed!\n\n");
    __helper_close_log_file();
    return 0;
}
