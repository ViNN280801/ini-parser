#ifndef INI_FILESYSTEM_H
#define INI_FILESYSTEM_H

#include <stdio.h>

#include "ini_export.h"
#include "ini_os_check.h"

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

/**
 * @brief File system operation result codes
 */
typedef enum
{
    INI_FS_SUCCESS = 0,                ///< Operation completed successfully
    INI_FS_FILE_NOT_FOUND,             ///< Specified file doesn't exist
    INI_FS_FILE_IS_DIR,                ///< Path points to a directory
    INI_FS_FILE_BAD_FORMAT,            ///< Path points to non-regular file
    INI_FS_FILE_EMPTY,                 ///< File exists but is empty
    INI_FS_ACCESS_DENIED,              ///< Insufficient permissions
    INI_FS_INVALID_PARAM,              ///< Invalid parameters provided
    INI_FS_UNKNOWN_ERROR,              ///< Other unspecified error
    INI_FS_STAT_ERROR,                 ///< stat() failed
    INI_FS_WIN_GETFILEATTRIBUTES_ERROR ///< GetFileAttributesExA() failed
} ini_fs_error_t;

typedef struct
{
    int read;    // 0 = false, 1 = true
    int write;   // 0 = false, 1 = true
    int execute; // 0 = false, 1 = true
} ini_file_permission_t;

/**
 * @brief Get the file permission of a file.
 *
 * @param filepath The path to the file.
 * @return The file permission of the file. If filepath is NULL, the function will return 0 0 0.
 * @example
 * @code
 * char const *filepath = "test.ini";
 * ini_file_permission_t permission = ini_get_file_permission(filepath);
 * // permission.read = 1
 * // permission.write = 1
 * // permission.execute = 0
 * @endcode
 */
INI_PUBLIC_API ini_file_permission_t ini_get_file_permission(char const *filepath);

/**
 * @brief Open a file.
 *
 * @param filepath The path to the file.
 * @param mode The mode to open the file in.
 * @return The file pointer.
 */
INI_PUBLIC_API FILE *ini_fopen(char const *filepath, char const *mode);

/**
 * @brief Check file existence and type
 * @param filepath Path to the file to check
 * @return Error code indicating file status
 * @retval INI_FS_SUCCESS File exists and is accessible
 * @retval INI_FS_FILE_NOT_FOUND File doesn't exist
 * @retval INI_FS_FILE_IS_DIR Path points to directory
 * @retval INI_FS_FILE_BAD_FORMAT Path points to special file
 * @retval INI_FS_FILE_EMPTY File exists but is empty
 */
INI_PUBLIC_API ini_fs_error_t ini_check_file_status(char const *filepath);

/**
 * @brief Get file size in bytes
 * @param filepath Path to the file
 * @param[out] size Pointer to store file size
 * @return Error code
 * @note Returns 0 for empty files, error for non-existent files
 */
INI_PUBLIC_API ini_fs_error_t ini_get_file_size(char const *filepath, size_t *size);

#endif // !INI_FILESYSTEM_H
