#define INI_IMPLEMENTATION

#include "ini_mutex.h"

ini_status_t ini_mutex_init(ini_mutex_t *mutex)
{
    if (!mutex)
        return INI_STATUS_INVALID_ARGUMENT;

    // Check initialization state
    if (mutex->initialized == INI_MUTEX_INITIALIZED)
        return INI_STATUS_MUTEX_ALREADY_INITIALIZED;

#if INI_OS_WINDOWS
    InitializeCriticalSection(&mutex->base);
#else
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr))
        return INI_STATUS_MUTEX_ERROR;

    // Set to NON-RECURSIVE (default behavior)
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);

    if (pthread_mutex_init(&mutex->base, &attr))
    {
        pthread_mutexattr_destroy(&attr);
        return INI_STATUS_MUTEX_ERROR;
    }
    pthread_mutexattr_destroy(&attr);
#endif

    mutex->initialized = INI_MUTEX_INITIALIZED;
    return INI_STATUS_SUCCESS;
}

ini_status_t ini_mutex_destroy(ini_mutex_t *mutex)
{
    if (!mutex || !mutex->initialized)
        return INI_STATUS_INVALID_ARGUMENT;

    if (mutex->locked == INI_MUTEX_LOCKED)
        return INI_STATUS_MUTEX_ERROR; // mutex is locked, cannot be destroyed

#if INI_OS_WINDOWS
    DeleteCriticalSection(&mutex->base);
#else
    if (pthread_mutex_destroy(&mutex->base))
        return INI_STATUS_MUTEX_ERROR;
#endif

    mutex->initialized = INI_MUTEX_NOT_INITIALIZED;
    return INI_STATUS_SUCCESS;
}

ini_status_t ini_mutex_lock(ini_mutex_t *mutex)
{
    if (!mutex || !mutex->initialized)
        return INI_STATUS_INVALID_ARGUMENT;

    if (mutex->locked == INI_MUTEX_LOCKED)
        return INI_STATUS_SUCCESS; // already locked

#if INI_OS_WINDOWS
    EnterCriticalSection(&mutex->base); // Will deadlock if already locked by same thread
#else
    if (pthread_mutex_lock(&mutex->base))
        return INI_STATUS_MUTEX_ERROR;
#endif

    mutex->locked = INI_MUTEX_LOCKED;
    return INI_STATUS_SUCCESS;
}

ini_status_t ini_mutex_unlock(ini_mutex_t *mutex)
{
    if (!mutex || !mutex->initialized)
        return INI_STATUS_INVALID_ARGUMENT;

    if (mutex->locked == INI_MUTEX_UNLOCKED)
        return INI_STATUS_SUCCESS; // already unlocked
#if INI_OS_WINDOWS
    LeaveCriticalSection(&mutex->base);
#else
    if (pthread_mutex_unlock(&mutex->base))
        return INI_STATUS_MUTEX_ERROR;
#endif
    mutex->locked = INI_MUTEX_UNLOCKED;
    return INI_STATUS_SUCCESS;
}
