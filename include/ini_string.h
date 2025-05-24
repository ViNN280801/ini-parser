#ifndef INI_STRING_H
#define INI_STRING_H

#include "ini_export.h"

INI_EXTERN_C_BEGIN

/**
 * @brief Duplicates a string.
 * @param s String to duplicate. If NULL, returns NULL.
 * @return Pointer to the duplicated string or NULL on failure.
 */
INI_PUBLIC_API char *ini_strdup(char const *s);

/**
 * @brief Strips whitespace from the beginning and end of a string.
 * @param s String to strip.
 * @return The length of the string after stripping.
 */
INI_PUBLIC_API unsigned ini_strstrip(char *s);

INI_EXTERN_C_END

#endif // !INI_STRING_H
