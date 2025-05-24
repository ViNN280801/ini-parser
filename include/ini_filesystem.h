#ifndef INI_FILESYSTEM_H
#define INI_FILESYSTEM_H

#include <stdio.h>

#include "ini_constants.h"
#include "ini_export.h"
#include "ini_os_check.h"
#include "ini_status.h"

INI_EXTERN_C_BEGIN

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
 * @brief Check if a file exists.
 *
 * @param filepath The path to the file.
 * @return INI_STATUS_SUCCESS if the file exists, INI_STATUS_FILE_NOT_FOUND otherwise.
 */
INI_PUBLIC_API ini_status_t ini_file_exists(char const *filepath);

/**
 * @brief Open a file.
 *
 * @param filepath The path to the file.
 * @param mode The mode to open the file in.
 * @return The file pointer.
 */
INI_PUBLIC_API FILE *ini_fopen(char const *filepath, char const *mode);

/**
 * @brief Check if a file is a directory.
 *
 * @param filepath The path to the file.
 * @return INI_STATUS_FILE_IS_DIR if the file is a directory,
 * in other cases, it will return any other status code.
 */
INI_PUBLIC_API ini_status_t ini_is_file_directory(char const *filepath);

/**
 * @brief Check file existence and type
 * @param filepath Path to the file to check
 * @return Error code indicating file status
 * @retval INI_STATUS_SUCCESS File exists and is accessible
 * @retval INI_STATUS_FILE_NOT_FOUND File doesn't exist
 * @retval INI_STATUS_FILE_IS_DIR Path points to directory
 * @retval INI_STATUS_FILE_BAD_FORMAT Path points to special file
 * @retval INI_STATUS_FILE_EMPTY File exists but is empty
 */
INI_PUBLIC_API ini_status_t ini_check_file_status(char const *filepath);

/**
 * @brief Get file size in bytes
 * @param filepath Path to the file
 * @param[out] size Pointer to store file size
 * @return Error code
 * @note Returns 0 for empty files, error for non-existent files
 */
INI_PUBLIC_API ini_status_t ini_get_file_size(char const *filepath, size_t *size);

/**
 * @brief Check if a file contains UTF-8 BOM (Byte Order Mark).
 *
 * @param file Pointer to the opened file for reading.
 * @return INI_STATUS_HAS_UTF8_BOM if the file contains UTF-8 BOM,
 *         INI_STATUS_HASNT_UTF8_BOM if the file does not contain BOM,
 *         INI_STATUS_INVALID_ARGUMENT if file is NULL,
 *         The file pointer is rewound to the beginning if BOM is not found.
 */
INI_PUBLIC_API ini_status_t ini_check_utf8_bom(FILE *file);

INI_EXTERN_C_END

#endif // !INI_FILESYSTEM_H
