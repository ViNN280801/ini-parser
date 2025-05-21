#ifndef INI_STRING_H
#define INI_STRING_H

#include "ini_export.h"

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
INI_PUBLIC_API unsigned strstrip(char *s);

#endif // !INI_STRING_H
