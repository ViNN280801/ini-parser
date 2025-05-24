#define INI_IMPLEMENTATION
#include "ini_status.h"

INI_PUBLIC_API char const *ini_status_to_string(ini_status_t status)
{
    switch (status)
    {
    case INI_STATUS_SUCCESS:
        return "Success";
    case INI_STATUS_FILE_NOT_FOUND:
        return "File not found";
    case INI_STATUS_FILE_EMPTY:
        return "File is empty";
    case INI_STATUS_FILE_OPEN_FAILED:
        return "File open failed";
    case INI_STATUS_FILE_BAD_FORMAT:
        return "File bad format";
    case INI_STATUS_MEMORY_ERROR:
        return "Memory error";
    case INI_STATUS_SECTION_NOT_FOUND:
        return "Section not found";
    case INI_STATUS_KEY_NOT_FOUND:
        return "Key not found";
    case INI_STATUS_INVALID_ARGUMENT:
        return "Invalid argument";
    case INI_STATUS_PLATFORM_ERROR:
        return "Platform error";
    case INI_STATUS_CLOSE_FAILED:
        return "Close failed";
    case INI_STATUS_PRINT_ERROR:
        return "Print error";
    case INI_STATUS_FILE_PERMISSION_DENIED:
        return "File permission denied";
    case INI_STATUS_FILE_IS_DIR:
        return "File is a directory";
    case INI_STATUS_STAT_ERROR:
        return "stat() failed";
    case INI_STATUS_WIN_GETFILEATTRIBUTES_ERROR:
        return "GetFileAttributesExA() failed";
    case INI_STATUS_MUTEX_ALREADY_INITIALIZED:
        return "Mutex already initialized";
    case INI_STATUS_ITERATOR_END:
        return "Iterator has reached the end of the table";
    case INI_STATUS_UNKNOWN_ERROR:
        return "Unknown error";
    default:
        return "Unknown error (passed status is not defined)";
    }
}
