#ifndef INI_STRING_H
#define INI_STRING_H

/**
 * @brief Creates a duplicate of the input string.
 *
 * @param s Null-terminated string to duplicate. If NULL, returns NULL.
 * @return Pointer to newly allocated string copy, or NULL on allocation failure.
 * @note Caller is responsible for freeing the returned pointer.
 */
char *ini_strdup(char const *s);

/**
 * @brief Strips whitespace from both ends of a string (in-place).
 *
 * @param s Null-terminated string to process (modified in-place).
 * @return Length of the stripped string (0 if input was NULL or empty after stripping).
 * @note Handles all standard isspace() characters (spaces, tabs, newlines, etc.).
 */
unsigned strstrip(char *s);

#endif // !INI_STRING_H
