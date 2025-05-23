#ifndef INI_CONSTANTS_H
#define INI_CONSTANTS_H

#include "ini_os_check.h"

#ifndef INI_PARSER_VERSION
#define INI_PARSER_VERSION "1.0.0"
#endif

#define INI_LINE_MAX 8192
#define INI_BUFFER_SIZE 2048
#define INI_HT_INITIAL_CAPACITY 16 ///< Initial capacity for the hash table. Must be a power of 2.

#if INI_OS_WINDOWS
    #include <io.h> // For _access()
    #include <windows.h>

    #define F_OK 0
    #define R_OK 4 // Read permission for _access()
    #define PATH_SEPARATOR '\\'
    #define INI_PATH_MAX MAX_PATH
#else
    #include <limits.h> // For PATH_MAX
    #include <unistd.h> // For access()

    #define PATH_SEPARATOR '/'
    #define INI_PATH_MAX PATH_MAX
#endif

#endif // !INI_CONSTANTS_H
