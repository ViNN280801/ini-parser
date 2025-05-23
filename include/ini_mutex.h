#ifndef INI_MUTEX_H
#define INI_MUTEX_H

#include <stddef.h>

#include "ini_export.h"
#include "ini_os_check.h"
#include "ini_status.h"

#if INI_OS_WINDOWS
#include <windows.h>
typedef CRITICAL_SECTION ini_mutex_base_t; ///< Windows mutex type.
#else
#include <pthread.h>
typedef pthread_mutex_t ini_mutex_base_t; ///< POSIX mutex type.
#endif

INI_EXTERN_C_BEGIN

typedef struct
{
    ini_mutex_base_t base; ///< Mutex base type (platform specific, for Windows: CRITICAL_SECTION, for POSIX: pthread_mutex_t).
    int initialized;       ///< INI_MUTEX_INITIALIZED if initialized, INI_MUTEX_NOT_INITIALIZED if not initialized.
    int locked;            ///< INI_MUTEX_LOCKED if locked, INI_MUTEX_UNLOCKED if not locked.
} ini_mutex_t;

#define INI_MUTEX_INITIALIZER {0, INI_MUTEX_NOT_INITIALIZED, INI_MUTEX_UNLOCKED}

#define INI_MUTEX_INITIALIZED 1
#define INI_MUTEX_NOT_INITIALIZED 0
#define INI_MUTEX_LOCKED 1
#define INI_MUTEX_UNLOCKED 0

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
 * @return INI_MUTEX_SUCCESS on success, INI_MUTEX_ERROR on error.
 *
 * Creates a recursive mutex that can be locked multiple times by the same thread.
 * Required for dynamic initialization (for static, use INI_MUTEX_INITIALIZER).
 */
INI_PUBLIC_API ini_status_t ini_mutex_init(ini_mutex_t *mutex);

/**
 * @brief Destroy a mutex.
 * @param mutex Pointer to mutex to destroy.
 * @return INI_MUTEX_SUCCESS on success, INI_MUTEX_ERROR on error.
 *
 * Releases all resources associated with the mutex.
 * Mutex must not be locked by any thread when destroyed.
 */
INI_PUBLIC_API ini_status_t ini_mutex_destroy(ini_mutex_t *mutex);

/**
 * @brief Lock a mutex.
 * @param mutex Pointer to mutex to lock.
 * @return INI_MUTEX_SUCCESS on success, INI_MUTEX_ERROR on error.
 *
 * Blocks until the mutex is available.
 * For recursive mutexes, same thread can lock multiple times.
 */
INI_PUBLIC_API ini_status_t ini_mutex_lock(ini_mutex_t *mutex);

/**
 * @brief Unlock a mutex.
 * @param mutex Pointer to mutex to unlock.
 * @return INI_MUTEX_SUCCESS on success, INI_MUTEX_ERROR on error.
 *
 * Releases the mutex. For recursive mutexes, must be called same number of times as lock.
 */
INI_PUBLIC_API ini_status_t ini_mutex_unlock(ini_mutex_t *mutex);

INI_EXTERN_C_END

#endif // !INI_MUTEX_H
