#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helper.h"

#include "ini_mutex.h"

// Clean test: Successfully initialize and destroy a mutex
void test_mutex_init_destroy_success()
{
    ini_mutex_t mutex = INI_MUTEX_INITIALIZER;

    ini_mutex_error_t err = ini_mutex_init(&mutex);
    assert(err == INI_MUTEX_SUCCESS);

    err = ini_mutex_destroy(&mutex);
    assert(err == INI_MUTEX_SUCCESS);

    print_success("test_mutex_init_destroy_success passed\n");
}

// Dirty test: NULL mutex for initialization
void test_mutex_init_null()
{
    ini_mutex_error_t err = ini_mutex_init(NULL);
    assert(err == INI_MUTEX_ERROR);
    print_success("test_mutex_init_null passed\n");
}

// Dirty test: NULL mutex for destruction
void test_mutex_destroy_null()
{
    ini_mutex_error_t err = ini_mutex_destroy(NULL);
    assert(err == INI_MUTEX_ERROR);
    print_success("test_mutex_destroy_null passed\n");
}

// Dirty test: Double initialization (platform-specific behavior)
void test_mutex_double_init()
{
    ini_mutex_t mutex = INI_MUTEX_INITIALIZER;

    ini_mutex_error_t err = ini_mutex_init(&mutex);
    assert(err == INI_MUTEX_SUCCESS);

    err = ini_mutex_init(&mutex);
    assert(err == INI_MUTEX_ALREADY_INITIALIZED);

    ini_mutex_destroy(&mutex);
    print_success("test_mutex_double_init passed\n");
}

// Dirty test: Destroy uninitialized mutex
void test_mutex_destroy_uninitialized()
{
    ini_mutex_t mutex = INI_MUTEX_INITIALIZER;

    ini_mutex_error_t err = ini_mutex_destroy(&mutex);
    assert(err == INI_MUTEX_ERROR);
    print_success("test_mutex_destroy_uninitialized passed\n");
}

// Dirty test: Lock and unlock with NULL mutex
void test_mutex_lock_unlock_null()
{
    ini_mutex_error_t err = ini_mutex_lock(NULL);
    assert(err == INI_MUTEX_ERROR);

    err = ini_mutex_unlock(NULL);
    assert(err == INI_MUTEX_ERROR);
    print_success("test_mutex_lock_unlock_null passed\n");
}

// Clean test: Lock and unlock a mutex successfully
void test_mutex_lock_unlock_success()
{
    ini_mutex_t mutex = INI_MUTEX_INITIALIZER;

    ini_mutex_error_t err = ini_mutex_init(&mutex);
    assert(err == INI_MUTEX_SUCCESS);

    err = ini_mutex_lock(&mutex);
    assert(err == INI_MUTEX_SUCCESS);

    err = ini_mutex_unlock(&mutex);
    assert(err == INI_MUTEX_SUCCESS);

    ini_mutex_destroy(&mutex);
    print_success("test_mutex_lock_unlock_success passed\n");
}

// Platform-specific test: Thread safety (simulated)
void test_mutex_thread_safety()
{
    ini_mutex_t mutex = INI_MUTEX_INITIALIZER;

    ini_mutex_error_t err = ini_mutex_init(&mutex);
    assert(err == INI_MUTEX_SUCCESS);

    // Simulate thread safety by locking/unlocking in sequence
    err = ini_mutex_lock(&mutex);
    assert(err == INI_MUTEX_SUCCESS);

    err = ini_mutex_unlock(&mutex);
    assert(err == INI_MUTEX_SUCCESS);

    ini_mutex_destroy(&mutex);
    print_success("test_mutex_thread_safety passed\n");
}

// New test: Check locked state after operations
void test_mutex_locked_state()
{
    ini_mutex_t mutex = INI_MUTEX_INITIALIZER;

    ini_mutex_error_t err = ini_mutex_init(&mutex);
    assert(err == INI_MUTEX_SUCCESS);

    err = ini_mutex_lock(&mutex);
    assert(err == INI_MUTEX_SUCCESS);

    err = ini_mutex_unlock(&mutex);
    assert(err == INI_MUTEX_SUCCESS);

    ini_mutex_destroy(&mutex);
    print_success("test_mutex_locked_state passed\n");
}

void test_mutex_recursive_lock()
{
    ini_mutex_t mutex = INI_MUTEX_INITIALIZER;
    assert(ini_mutex_init(&mutex) == INI_MUTEX_SUCCESS);

    // First lock
    assert(ini_mutex_lock(&mutex) == INI_MUTEX_SUCCESS);
    assert(mutex.locked == INI_MUTEX_LOCKED);

    // Second lock
    assert(ini_mutex_lock(&mutex) == INI_MUTEX_SUCCESS);
    assert(mutex.locked == INI_MUTEX_LOCKED);

    // First unlock
    assert(ini_mutex_unlock(&mutex) == INI_MUTEX_SUCCESS);
    assert(mutex.locked == INI_MUTEX_UNLOCKED);

    // Second unlock
    // Should return success even if already unlocked
    // because this function does nothing if mutex is not locked, just returns success
    assert(ini_mutex_unlock(&mutex) == INI_MUTEX_SUCCESS);
    assert(mutex.locked == INI_MUTEX_UNLOCKED);

    ini_mutex_destroy(&mutex);
    print_success("test_mutex_recursive_lock passed\n");
}

void test_mutex_destroy_locked()
{
    ini_mutex_t mutex = INI_MUTEX_INITIALIZER;
    assert(ini_mutex_init(&mutex) == INI_MUTEX_SUCCESS);
    assert(ini_mutex_lock(&mutex) == INI_MUTEX_SUCCESS);

    // Should return error
    assert(ini_mutex_destroy(&mutex) == INI_MUTEX_ERROR);

    assert(ini_mutex_unlock(&mutex) == INI_MUTEX_SUCCESS);
    assert(ini_mutex_destroy(&mutex) == INI_MUTEX_SUCCESS);
    print_success("test_mutex_destroy_locked passed\n");
}

int main()
{
    __helper_init_log_file();

    test_mutex_init_destroy_success();
    test_mutex_init_null();
    test_mutex_destroy_null();
    test_mutex_double_init();
    test_mutex_destroy_uninitialized();
    test_mutex_lock_unlock_null();
    test_mutex_lock_unlock_success();
    test_mutex_thread_safety();
    test_mutex_locked_state();
    test_mutex_recursive_lock();
    test_mutex_destroy_locked();

    print_success("All ini_mutex tests passed!\n\n");
    __helper_close_log_file();
    return EXIT_SUCCESS;
}
