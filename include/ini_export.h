#ifndef INI_EXPORT_H
#define INI_EXPORT_H

#include "ini_os_check.h"

#ifdef __cplusplus
    #define INI_EXTERN_C_BEGIN extern "C" {
    #define INI_EXTERN_C_END   }
#else
    #define INI_EXTERN_C_BEGIN
    #define INI_EXTERN_C_END
#endif

#if INI_OS_WINDOWS
    #ifdef _MSC_VER // MSVC compiler
        #ifdef INIPARSER_EXPORTS
            #define INIPARSER_API __declspec(dllexport)
        #else
            #define INIPARSER_API __declspec(dllimport)
        #endif
    #else
        // MinGW or other Windows compilers
        #ifdef INIPARSER_EXPORTS
            #define INIPARSER_API __attribute__((dllexport))
        #else
            #define INIPARSER_API __attribute__((dllimport))
        #endif
    #endif
#else
    // Linux/macOS
    #ifdef INIPARSER_EXPORTS
        #define INIPARSER_API __attribute__((visibility("default")))
    #else
        #define INIPARSER_API
    #endif
#endif

#ifdef INI_IMPLEMENTATION
    #if INI_OS_WINDOWS && defined(_MSC_VER)
        #define INI_PUBLIC_API __declspec(dllexport)
    #elif INI_OS_WINDOWS
        #define INI_PUBLIC_API __attribute__((dllexport))
    #else
        #define INI_PUBLIC_API __attribute__((visibility("default")))
    #endif
#else
    #define INI_PUBLIC_API INIPARSER_API ///< Macro for marking functions that should be available from outside.
#endif

#endif // !INI_EXPORT_H
