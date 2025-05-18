#ifndef INI_MUTEX_H
#define INI_MUTEX_H

#include <stddef.h>

#include "ini_os_check.h"

#if INI_OS_WINDOWS
    #include <windows.h>
    typedef CRITICAL_SECTION ini_mutex_t; ///< Windows mutex type.
    #define INI_MUTEX_INITIALIZER {0}     ///< Static initializer for Windows.
#else
    #include <pthread.h>
    typedef pthread_mutex_t ini_mutex_t; ///< POSIX mutex type.
    #define INI_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER ///< POSIX static initializer.
#endif

#ifdef DEBUG
    /**
     * @brief Debug assertion for mutex validation.
     * @param m Pointer to mutex to validate.
     *
     * In debug builds, aborts program if mutex pointer is NULL with error location.
     * No effect in release builds.
     */
    #define INI_ASSERT_MUTEX(m)                                                  \
        do                                                                       \
        {                                                                        \
            if (!(m))                                                            \
            {                                                                    \
                fprintf(stderr, "Invalid mutex at %s:%d\n", __FILE__, __LINE__); \
                abort();                                                         \
            }                                                                    \
        } while (0)
#else
    #define INI_ASSERT_MUTEX(m) ///< Empty in release builds.
#endif

/**
 * @brief Initialize a mutex.
 * @param mutex Pointer to mutex to initialize.
 * @return 0 on success, -1 on error.
 *
 * Creates a recursive mutex that can be locked multiple times by the same thread.
 * Required for dynamic initialization (for static, use INI_MUTEX_INITIALIZER).
 */
int ini_mutex_init(ini_mutex_t *mutex);

/**
 * @brief Destroy a mutex.
 * @param mutex Pointer to mutex to destroy.
 * @return 0 on success, -1 on error.
 *
 * Releases all resources associated with the mutex.
 * Mutex must not be locked by any thread when destroyed.
 */
int ini_mutex_destroy(ini_mutex_t *mutex);

/**
 * @brief Lock a mutex.
 * @param mutex Pointer to mutex to lock.
 * @return 0 on success, -1 on error.
 *
 * Blocks until the mutex is available.
 * For recursive mutexes, same thread can lock multiple times.
 */
int ini_mutex_lock(ini_mutex_t *mutex);

/**
 * @brief Try to lock a mutex without blocking.
 * @param mutex Pointer to mutex to try locking.
 * @return 0 if locked successfully, -1 if mutex is already locked.
 *
 * Non-blocking version of ini_mutex_lock().
 */
int ini_mutex_trylock(ini_mutex_t *mutex);

/**
 * @brief Unlock a mutex.
 * @param mutex Pointer to mutex to unlock.
 * @return 0 on success, -1 on error.
 *
 * Releases the mutex. For recursive mutexes, must be called same number of times as lock.
 */
int ini_mutex_unlock(ini_mutex_t *mutex);

#endif // !INI_MUTEX_H
