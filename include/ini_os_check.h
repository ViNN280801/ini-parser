#ifndef INI_OS_CHECK_H
#define INI_OS_CHECK_H

// ==================== Platform checks ====================
/// @link https://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor
#if defined(_WIN32) || defined(_WIN64)
    #define INI_OS_WINDOWS 1
    #define INI_OS_APPLE 0
    #define INI_OS_LINUX 0
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC
        #define INI_OS_WINDOWS 0
        #define INI_OS_APPLE 1
        #define INI_OS_LINUX 0
    #else
        #error "Unsupported Apple platform"
    #endif // Check if the Apple platform is supported
#elif defined(__linux__)
    #define INI_OS_WINDOWS 0
    #define INI_OS_APPLE 0
    #define INI_OS_LINUX 1
#else
    #error "Unsupported platform"
#endif // Check if the platform is supported

#endif // !INI_OS_CHECK_H
