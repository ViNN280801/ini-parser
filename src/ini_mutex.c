#include "ini_mutex.h"

int ini_mutex_init(ini_mutex_t *mutex)
{
    if (!mutex)
        return -1;
#if INI_OS_WINDOWS
    InitializeCriticalSection(mutex);
    return 0;
#else
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr))
        return -1;
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    int ret = pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    return ret;
#endif
}

int ini_mutex_destroy(ini_mutex_t *mutex)
{
    if (!mutex)
        return -1;
#if INI_OS_WINDOWS
    DeleteCriticalSection(mutex);
    return 0;
#else
    return pthread_mutex_destroy(mutex);
#endif
}

int ini_mutex_lock(ini_mutex_t *mutex)
{
    if (!mutex)
        return -1;
#if INI_OS_WINDOWS
    EnterCriticalSection(mutex);
    return 0;
#else
    return pthread_mutex_lock(mutex);
#endif
}

int ini_mutex_trylock(ini_mutex_t *mutex)
{
    if (!mutex)
        return -1;
#if INI_OS_WINDOWS
    return TryEnterCriticalSection(mutex) ? 0 : -1;
#else
    return pthread_mutex_trylock(mutex);
#endif
}

int ini_mutex_unlock(ini_mutex_t *mutex)
{
    if (!mutex)
        return -1;
#if INI_OS_WINDOWS
    LeaveCriticalSection(mutex);
    return 0;
#else
    return pthread_mutex_unlock(mutex);
#endif
}
