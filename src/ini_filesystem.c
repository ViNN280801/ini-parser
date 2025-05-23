#define INI_IMPLEMENTATION
#include "ini_filesystem.h"

#include <errno.h>
#include <string.h>

#if INI_OS_LINUX || INI_OS_APPLE
#include <sys/stat.h>
#endif

INI_PUBLIC_API ini_file_permission_t ini_get_file_permission(char const *filepath)
{
    ini_file_permission_t permission = {0}; // Default perms: 000

    if (!filepath || strlen(filepath) == 0)
        return permission;

#if INI_OS_WINDOWS
    DWORD attributes = GetFileAttributesA(filepath);
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        // On Windows, assume write permissions if the parent directory is writable
        char dir[INI_PATH_MAX];
        strcpy(dir, filepath);

        char *last_slash = strrchr(dir, '\\');
        if (!last_slash)
            last_slash = strrchr(dir, '/');

        if (last_slash)
        {
            *last_slash = '\0';
            if (dir[0] == '\0')
                strcpy(dir, ".");

            DWORD dir_attrs = GetFileAttributesA(dir);
            if (dir_attrs != INVALID_FILE_ATTRIBUTES &&
                !(dir_attrs & FILE_ATTRIBUTE_READONLY))
            {
                permission.read = 1;
                permission.write = 1;
            }
        }
        else
        {
            DWORD dir_attrs = GetFileAttributesA(".");
            if (dir_attrs != INVALID_FILE_ATTRIBUTES &&
                !(dir_attrs & FILE_ATTRIBUTE_READONLY))
            {
                permission.read = 1;
                permission.write = 1;
            }
        }
        return permission;
    }

    if (_access(filepath, R_OK) == 0)
        permission.read = 1;
    if (!(attributes & FILE_ATTRIBUTE_READONLY))
        permission.write = 1;

    char const *ext = strrchr(filepath, '.');
    if (ext && (_stricmp(ext, ".exe") == 0 ||
                _stricmp(ext, ".bat") == 0 ||
                _stricmp(ext, ".cmd") == 0 ||
                _stricmp(ext, ".msi") == 0))
    {
        permission.execute = 1;
    }
#else
    if (access(filepath, F_OK) == 0)
    {
        // File exists: check permissions directly
        if (access(filepath, R_OK) == 0)
            permission.read = 1;
        if (access(filepath, W_OK) == 0)
            permission.write = 1;
        if (access(filepath, X_OK) == 0)
            permission.execute = 1;
    }
    else
    {
        // File doesn't exist: check parent directory's write permission
        char dir[INI_PATH_MAX];
        strcpy(dir, filepath);

        char *last_slash = strrchr(dir, '/');
        if (last_slash)
        {
            *last_slash = '\0';
            if (dir[0] == '\0')
                strcpy(dir, ".");

            if (access(dir, W_OK) == 0)
                permission.write = 1;
        }
        else
        {
            if (access(".", W_OK) == 0)
                permission.write = 1;
        }
    }
#endif

    return permission;
}

INI_PUBLIC_API ini_status_t ini_file_exists(char const *filepath)
{
    if (!filepath || strlen(filepath) == 0)
        return INI_STATUS_INVALID_ARGUMENT;

#if INI_OS_WINDOWS
    DWORD attrs = GetFileAttributesA(filepath);
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
            return INI_STATUS_FILE_NOT_FOUND;
        return INI_STATUS_UNKNOWN_ERROR;
    }
    return INI_STATUS_SUCCESS;
#else
    struct stat sb;
    if (stat(filepath, &sb) == -1)
    {
        if (errno == ENOENT)
            return INI_STATUS_FILE_NOT_FOUND;
        return INI_STATUS_UNKNOWN_ERROR;
    }
    return INI_STATUS_SUCCESS;
#endif
}

INI_PUBLIC_API FILE *ini_fopen(char const *filepath, char const *mode)
{
    if (!filepath || !mode)
        return NULL;

    /// Validate mode string against standard modes
    /// @see https://en.cppreference.com/w/cpp/io/c/fopen
    char const *valid_modes[] = {"r", "w", "a", "r+", "w+", "a+",
                                 "rb", "wb", "ab", "r+b", "w+b", "a+b",
                                 "rt", "wt", "at", "r+t", "w+t", "a+t"};
    int valid = 0;
    for (size_t i = 0; i < sizeof(valid_modes) / sizeof(valid_modes[0]); i++)
    {
        if (strcmp(mode, valid_modes[i]) == 0)
        {
            valid = 1;
            break;
        }
    }
    if (!valid)
        return NULL;

    FILE *file = NULL;

#if INI_OS_WINDOWS
    /// Use secure version on Windows
    /// @see https://learn.microsoft.com/ru-ru/cpp/c-runtime-library/reference/fopen-s-wfopen-s?view=msvc-170
    if (fopen_s(&file, filepath, mode) != 0)
        return NULL;
#else
    /// Standard fopen on POSIX systems
    /// @see https://en.cppreference.com/w/c/io/fopen
    file = fopen(filepath, mode);
    if (!file)
        return NULL;
#endif

    return file;
}

INI_PUBLIC_API ini_status_t ini_is_file_directory(char const *filepath)
{
    if (!filepath || strlen(filepath) == 0)
        return INI_STATUS_INVALID_ARGUMENT;

#if INI_OS_WINDOWS
    DWORD fileAttributes = GetFileAttributes(filepath);
    if (fileAttributes == INVALID_FILE_ATTRIBUTES)
        return INI_STATUS_FILE_NOT_FOUND;

    if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return INI_STATUS_FILE_IS_DIR;
    else
        return INI_STATUS_FILE_NOT_FOUND;
#else
    struct stat sb;
    if (stat(filepath, &sb) == 0 && S_ISDIR(sb.st_mode))
        return INI_STATUS_FILE_IS_DIR;
    else
        return INI_STATUS_FILE_NOT_FOUND;
#endif
}

INI_PUBLIC_API ini_status_t ini_check_file_status(char const *filepath)
{
    if (!filepath || strlen(filepath) == 0)
        return INI_STATUS_INVALID_ARGUMENT;

    if (ini_is_file_directory(filepath) == INI_STATUS_FILE_IS_DIR)
        return INI_STATUS_FILE_IS_DIR;

#if INI_OS_WINDOWS
    DWORD attrs = GetFileAttributesA(filepath);
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        switch (GetLastError())
        {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return INI_STATUS_FILE_NOT_FOUND;
        case ERROR_ACCESS_DENIED:
            return INI_STATUS_FILE_PERMISSION_DENIED;
        default:
            return INI_STATUS_UNKNOWN_ERROR;
        }
    }
#else
    struct stat st;
    if (stat(filepath, &st) != 0)
    {
        switch (errno)
        {
        case ENOENT:
            return INI_STATUS_FILE_NOT_FOUND;
        case EACCES:
            return INI_STATUS_FILE_PERMISSION_DENIED;
        default:
            return INI_STATUS_UNKNOWN_ERROR;
        }
    }
    if (!S_ISREG(st.st_mode))
        return INI_STATUS_FILE_BAD_FORMAT;
    if (st.st_size == 0)
        return INI_STATUS_FILE_EMPTY;
#endif

    return INI_STATUS_SUCCESS;
}

INI_PUBLIC_API ini_status_t ini_get_file_size(char const *filepath, size_t *size)
{
    if (!filepath || !size)
        return INI_STATUS_INVALID_ARGUMENT;

    if (ini_is_file_directory(filepath) == INI_STATUS_FILE_IS_DIR)
        return INI_STATUS_FILE_IS_DIR;

#if INI_OS_WINDOWS
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (!GetFileAttributesExA(filepath, GetFileExInfoStandard, &fileInfo))
        return INI_STATUS_WIN_GETFILEATTRIBUTES_ERROR;

    LARGE_INTEGER fileSize;
    fileSize.LowPart = fileInfo.nFileSizeLow;
    fileSize.HighPart = fileInfo.nFileSizeHigh;
    *size = (size_t)fileSize.QuadPart;
#else
    struct stat sb;
    if (stat(filepath, &sb) == -1)
        return INI_STATUS_STAT_ERROR;
    *size = (size_t)sb.st_size;
#endif

    return INI_STATUS_SUCCESS;
}
