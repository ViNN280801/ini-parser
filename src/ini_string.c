#define INI_IMPLEMENTATION
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ini_string.h"

INI_PUBLIC_API char *ini_strdup(char const *s)
{
    char *t;
    size_t len;
    if (!s)
        return NULL;

    len = strlen(s) + 1;
    t = (char *)malloc(len);
    if (t)
        memcpy(t, s, len);
    return t;
}

INI_PUBLIC_API unsigned strstrip(char *s)
{
    char *last = NULL;
    char *dest = s;

    if (s == NULL)
        return 0;

    last = s + strlen(s);
    while (isspace((unsigned char)*s) && *s)
        s++;

    while (last > s)
    {
        if (!isspace((unsigned char)*(last - 1)))
            break;
        last--;
    }
    *last = (char)0;

    memmove(dest, s, last - s + 1);
    return last - s;
}
