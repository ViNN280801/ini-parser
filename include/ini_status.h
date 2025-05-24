#ifndef INI_STATUS_H
#define INI_STATUS_H

#include "ini_export.h"

INI_EXTERN_C_BEGIN

/// @brief Error codes returned by the INI parser.
typedef enum
{
    INI_STATUS_SUCCESS,                     ///< Operation succeeded.
    INI_STATUS_FILE_NOT_FOUND,              ///< Specified file does not exist.
    INI_STATUS_FILE_EMPTY,                  ///< File is empty.
    INI_STATUS_FILE_OPEN_FAILED,            ///< Failed to open the file.
    INI_STATUS_FILE_BAD_FORMAT,             ///< File has invalid INI syntax.
    INI_STATUS_MEMORY_ERROR,                ///< Failed to allocate/reallocate/free memory.
    INI_STATUS_SECTION_NOT_FOUND,           ///< Requested section does not exist.
    INI_STATUS_KEY_NOT_FOUND,               ///< Requested key does not exist.
    INI_STATUS_INVALID_ARGUMENT,            ///< Invalid argument passed to a function.
    INI_STATUS_MUTEX_ERROR,                 ///< Mutex operation failed.
    INI_STATUS_PLATFORM_ERROR,              ///< Platform-specific error (e.g., mutex failure).
    INI_STATUS_CLOSE_FAILED,                ///< Failed to close the file.
    INI_STATUS_LACK_OF_MEMORY,              ///< Lack of memory (for example, when expanding the table).
    INI_STATUS_PRINT_ERROR,                 ///< Error during printing/formatting.
    INI_STATUS_FILE_PERMISSION_DENIED,      ///< Permission denied (e.g. read-only file).
    INI_STATUS_FILE_IS_DIR,                 ///< Path points to a directory.
    INI_STATUS_STAT_ERROR,                  ///< stat() failed.
    INI_STATUS_WIN_GETFILEATTRIBUTES_ERROR, ///< GetFileAttributesExA() failed.
    INI_STATUS_MUTEX_ALREADY_INITIALIZED,   ///< Mutex already initialized.
    INI_STATUS_ITERATOR_END,                ///< Iterator has reached the end of the table.
    INI_STATUS_HAS_UTF8_BOM,                ///< File contains UTF-8 BOM.
    INI_STATUS_HASNT_UTF8_BOM,              ///< File does not contain UTF-8 BOM.
    INI_STATUS_UNKNOWN_ERROR                ///< Unknown error.
} ini_status_t;

/**
 * @brief Converts an `ini_status_t` to a human-readable string.
 * @param status The status code to convert.
 * @return A static string describing the status.
 */
INI_PUBLIC_API char const *ini_status_to_string(ini_status_t status);

INI_EXTERN_C_END

#endif // !INI_STATUS_H
