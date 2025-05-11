#ifndef INI_PARSER_H
#define INI_PARSER_H

// ==================== Platform checks ====================
/// @link https://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor
#if defined(_WIN32) || defined(_WIN64)
#define INI_OS_WINDOWS 1
#define INI_OS_APPLE 0
#define INI_OS_UNIX 0
#elif __APPLE__
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS, tvOS, or watchOS Simulator
#define INI_OS_WINDOWS 0
#define INI_OS_APPLE 1
#define INI_OS_UNIX 0
#elif TARGET_OS_MACCATALYST
// Mac's Catalyst (ports iOS API into Mac, like UIKit).
#define INI_OS_WINDOWS 0
#define INI_OS_APPLE 1
#define INI_OS_UNIX 0
#elif TARGET_OS_IPHONE
// iOS, tvOS, or watchOS device
#define INI_OS_WINDOWS 0
#define INI_OS_APPLE 1
#define INI_OS_UNIX 0
#elif TARGET_OS_MAC
// Other kinds of Apple platforms
#define INI_OS_WINDOWS 0
#define INI_OS_APPLE 1
#define INI_OS_UNIX 0
#else
#error "Unknown Apple platform"
#endif
#elif defined(__unix__) || defined(__linux__)
#define INI_OS_WINDOWS 0
#define INI_OS_APPLE 0
#define INI_OS_UNIX 1 // Linux/BSD/etc
#else
#define INI_OS_WINDOWS 0
#define INI_OS_APPLE 0
#define INI_OS_UNIX 0
#endif
// =========================================================

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef INI_PARSER_VERSION
#define INI_PARSER_VERSION "1.0.0"
#endif

#define INI_ERRSTACK_SIZE 1024
#define INI_LINE_MAX 1024
#define INI_BUFFER_SIZE 2048

#if INI_OS_WINDOWS
#ifdef _MSC_VER
#include <windows.h>
#endif
#ifdef INIPARSER_EXPORTS
#define INIPARSER_API __declspec(dllexport)
#else
#define INIPARSER_API __declspec(dllimport)
#endif
#else
#include <pthread.h>
#define INIPARSER_API __attribute__((visibility("default"))) // For GCC/Clang
#endif

// Apple-specific optimizations
#if INI_OS_APPLE
#include <dispatch/dispatch.h> // For GCD (Grand Central Dispatch)
#endif

    /// @brief Error codes returned by the INI parser.
    typedef enum
    {
        INI_SUCCESS = 0,          ///< Operation succeeded.
        INI_FILE_NOT_FOUND,       ///< Specified file does not exist.
        INI_FILE_EMPTY,           ///< File is empty.
        INI_FILE_IS_DIR,          ///< Path points to a directory, not a file.
        INI_FILE_OPEN_FAILED,     ///< Failed to open the file.
        INI_FILE_BAD_FORMAT,      ///< File has invalid INI syntax.
        INI_SECTION_NOT_FOUND,    ///< Requested section does not exist.
        INI_KEY_NOT_FOUND,        ///< Requested key does not exist.
        INI_INVALID_ARGUMENT,     ///< Invalid argument passed to a function.
        INI_PLATFORM_ERROR,       ///< Platform-specific error (e.g., mutex failure).
        INI_CLOSE_FAILED,         ///< Failed to close the file.
        INI_MEMORY_ERROR,         ///< Failed to allocate/reallocate/free memory.
        INI_PRINT_ERROR,          ///< Error during printing/formatting.
        INI_FILE_BAD_FORMAT_LINE, ///< Syntax error at a specific line.
    } ini_error_t;

    /// @brief Common stack of errors for all the modules.
    INIPARSER_API extern ini_error_t __ini_errstack[INI_ERRSTACK_SIZE];

#if INI_OS_WINDOWS
    extern CRITICAL_SECTION __ini_errstack_mutex;
#elif INI_OS_APPLE
extern dispatch_semaphore_t __ini_errstack_semaphore;
#else
extern pthread_mutex_t __ini_errstack_mutex;
#endif

    /// @brief Private function, initializes the error stack with INI_SUCCESS and initializes the mutex/semaphore.
    INIPARSER_API void __ini_init_errstack();

    /// @brief Private function, finalizes the error stack and destroys the mutex/semaphore.
    INIPARSER_API void __ini_finalize_errstack();

    /// @brief Private function, clears the error stack (sets all the errors to INI_SUCCESS).
    INIPARSER_API void __ini_clear_errstack();

    /// @brief Private function, adds an error to the error stack.
    INIPARSER_API void __ini_add_in_errstack(ini_error_t error);

    /// @brief Private function, returns 1 if `__errstack` contains specified error, otherwise 0.
    INIPARSER_API int __ini_has_in_errstack(ini_error_t error);

    /**
     * @brief Checks if there are any errors in the error stack.
     * @return 1 if there were errors in operations, 0 otherwise.
     */
    INIPARSER_API int ini_has_error();

    typedef struct
    {
        ini_error_t error;     ///< Error code.
        char const *inipath;   ///< Path to the INI file.
        char const *srcpath;   ///< Path to the C source file where the error occurred.
        int ini_line_number;   ///< Line number in the INI file (if applicable -> 0 if not applicable).
        int src_line_number;   ///< Line number in the source file (if applicable -> 0 if not applicable).
        char const *custommsg; ///< Message that can be provided by the developer.
    } ini_error_details_t;

    /**
     * @brief Converts an `ini_error_t` to a human-readable string.
     * @param error The error code to convert.
     * @return A static string describing the error.
     */
    INIPARSER_API char const *ini_error_to_string(ini_error_t error);

    /**
     * @brief Converts an `ini_error_details_t` to a human-readable string.
     * @param error_detailed Detailed error structure.
     * @return A static string describing the error.
     */
    INIPARSER_API char const *ini_error_details_to_string(ini_error_details_t error_detailed);

    typedef struct
    {
        char *key;
        char *value;
    } ini_key_value_t;

    typedef struct ini_section_t
    {
        char *name;
        ini_key_value_t *pairs;
        int pair_count;
        struct ini_section_t *subsections;
        int subsection_count;
    } ini_section_t;

    typedef struct
    {
        ini_section_t *sections;
        int section_count;

        /// @brief Mutex for thread safety when using the context in a multi-threaded environment.
#if INI_OS_WINDOWS
        CRITICAL_SECTION mutex;
#elif INI_OS_APPLE
    dispatch_semaphore_t semaphore; // GCD semaphore (optional)
#else
    pthread_mutex_t mutex;
#endif
    } ini_context_t;

    /// @brief Public function, initializes the INI parser.
    INIPARSER_API void ini_initialize();

    /// @brief Public function, finalizes the INI parser.
    INIPARSER_API void ini_finalize();

    /// @brief Private variable, checks if the INI parser is initialized. 1 - initialized, 0 - not initialized.
    INIPARSER_API extern int __ini_is_initialized;

    /// @brief Public function, checks if the INI parser is initialized. 1 - initialized, 0 - not initialized.
    INIPARSER_API int ini_is_initialized();

    /**
     * @brief Checks if an INI file is valid (exists, readable, and well-formatted).
     * @param filepath Path to the INI file.
     * @return Error details (INI_SUCCESS if valid).
     */
    INIPARSER_API ini_error_details_t ini_good(char const *filepath);

    /**
     * @brief Loads an INI file into a context.
     * @param[in, out] ctx The context to populate. Assumes it is NULL.
     *                     It will be initialized with ini_create_context() if it is NULL.
     *                     Otherwise, it will be freed with ini_free() before loading the new file.
     * @param[in] filepath Path to the INI file.
     * @return Error details (INI_SUCCESS on success).
     * @note Thread-safe: Uses mutex/semaphore internally.
     */
    INIPARSER_API ini_error_details_t ini_load(ini_context_t *ctx, char const *filepath);

    /**
     * @brief Allocates and initializes a new INI context.
     *        Does not clears the error stack if successful.
     * @return New context (NULL on failure).
     * @note Initializes platform-specific mutex/semaphore.
     */
    INIPARSER_API ini_context_t *ini_create_context();

    /**
     * @brief Frees an INI context and all its resources (sections, keys, mutex).
     *        Important: The context will be freed and NOT set to NULL.
     *        Does not clears the error stack if successful.
     * @param ctx Context to free.
     * @return Error details (INI_SUCCESS on success).
     */
    INIPARSER_API ini_error_details_t ini_free(ini_context_t *ctx);

    /**
     * @brief Gets a value from a section in the INI context.
     * @param ctx Context to query.
     * @param section Section name (NULL for global keys).
     * @param key Key name, for example: "gui" or "gui.mainwindow"
     * @param[out] value Retrieved value (caller must free).
     * @return Error details (INI_SUCCESS on success).
     * @note Thread-safe: Uses mutex/semaphore internally.
     */
    INIPARSER_API ini_error_details_t ini_get_value(ini_context_t const *ctx,
                                                    char const *section,
                                                    char const *key,
                                                    char **value);

    /**
     * @brief Prints the INI context contents (for debugging).
     * @param stream Stream to print to. Example: stderr, stdout, file, etc.
     * @param ctx Context to print.
     * @return Error details (INI_SUCCESS on success).
     */
    INIPARSER_API ini_error_details_t ini_print(FILE *stream, ini_context_t const *ctx);

    /**
     * @brief Locks the context's mutex/semaphore (internal use).
     * @param ctx Context to lock.
     * @return Error details (INI_SUCCESS on success).
     */
    INIPARSER_API ini_error_details_t __INI_MUTEX_LOCK(ini_context_t *ctx);

    /**
     * @brief Unlocks the context's mutex/semaphore (internal use).
     * @param ctx Context to unlock.
     * @return Error details (INI_SUCCESS on success).
     */
    INIPARSER_API ini_error_details_t __INI_MUTEX_UNLOCK(ini_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif // !INI_PARSER_H
