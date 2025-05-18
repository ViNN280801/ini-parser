#include "ini_mutex.h"

ini_mutex_error_t ini_mutex_init(ini_mutex_t *mutex)
{
    if (!mutex)
        return INI_MUTEX_ERROR;
#if INI_OS_WINDOWS
    InitializeCriticalSection(mutex);
    return INI_MUTEX_SUCCESS;
#else
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr))
        return INI_MUTEX_ERROR;
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    int ret = pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    return (ret == 0) ? INI_MUTEX_SUCCESS : INI_MUTEX_ERROR;
#endif
}

ini_mutex_error_t ini_mutex_destroy(ini_mutex_t *mutex)
{
    if (!mutex)
        return INI_MUTEX_ERROR;
#if INI_OS_WINDOWS
    DeleteCriticalSection(mutex);
    return INI_MUTEX_SUCCESS;
#else
    return pthread_mutex_destroy(mutex) == 0 ? INI_MUTEX_SUCCESS : INI_MUTEX_ERROR;
#endif
}

ini_mutex_error_t ini_mutex_lock(ini_mutex_t *mutex)
{
    if (!mutex)
        return INI_MUTEX_ERROR;
#if INI_OS_WINDOWS
    EnterCriticalSection(mutex);
    return INI_MUTEX_SUCCESS;
#else
    return pthread_mutex_lock(mutex) == 0 ? INI_MUTEX_SUCCESS : INI_MUTEX_ERROR;
#endif
}

ini_mutex_error_t ini_mutex_trylock(ini_mutex_t *mutex)
{
    if (!mutex)
        return INI_MUTEX_ERROR;
#if INI_OS_WINDOWS
    return TryEnterCriticalSection(mutex) ? INI_MUTEX_SUCCESS : INI_MUTEX_ERROR;
#else
    return pthread_mutex_trylock(mutex) == 0 ? INI_MUTEX_SUCCESS : INI_MUTEX_ERROR;
#endif
}

ini_mutex_error_t ini_mutex_unlock(ini_mutex_t *mutex)
{
    if (!mutex)
        return INI_MUTEX_ERROR;
#if INI_OS_WINDOWS
    LeaveCriticalSection(mutex);
    return INI_MUTEX_SUCCESS;
#else
    return pthread_mutex_unlock(mutex) == 0 ? INI_MUTEX_SUCCESS : INI_MUTEX_ERROR;
#endif
}
