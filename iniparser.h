#ifndef INI_PARSER_H
#define INI_PARSER_H

// ==================== Platform checks ====================
#if defined(_WIN32) || defined(_WIN64)
    #define INI_OS_WINDOWS 1
    #define INI_OS_APPLE   0
    #define INI_OS_UNIX    0
#elif defined(__APPLE__) && defined(__MACH__)
    #define INI_OS_WINDOWS 0
    #define INI_OS_APPLE   1 // macOS or iOS
    #define INI_OS_UNIX    1  // but with Apple's peculiarities
#elif defined(__unix__) || defined(__linux__)
    #define INI_OS_WINDOWS 0
    #define INI_OS_APPLE   0
    #define INI_OS_UNIX    1 // Linux/BSD/etc
#else
    #define INI_OS_WINDOWS 0
    #define INI_OS_APPLE   0
    #define INI_OS_UNIX    0
#endif
// =========================================================

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef INI_PARSER_VERSION
#define INI_PARSER_VERSION "1.0.0"
#endif

#define INI_LINE_MAX 1024
#define INI_BUFFER_SIZE 2048

#ifdef INI_OS_WINDOWS
    #include <windows.h>
#ifdef INIPARSER_EXPORTS
    #define INIPARSER_API __declspec(dllexport)
#else
    #define INIPARSER_API __declspec(dllimport)
#endif
#else
    #include <pthread.h>
#endif

// Apple-specific optimizations
#if INI_OS_APPLE
    #include <dispatch/dispatch.h> // For GCD (Grand Central Dispatch)
#endif

    /// @brief Error codes returned by the INI parser.
    typedef enum
    {
        INI_SUCCESS = 0,                ///< Operation succeeded.
        INI_FILE_NOT_FOUND,             ///< Specified file does not exist.
        INI_FILE_EMPTY,                 ///< File is empty.
        INI_FILE_IS_DIR,                ///< Path points to a directory, not a file.
        INI_FILE_OPEN_FAILED,           ///< Failed to open the file.
        INI_FILE_BAD_FORMAT,            ///< File has invalid INI syntax.
        INI_SECTION_NOT_FOUND,          ///< Requested section does not exist.
        INI_KEY_NOT_FOUND,              ///< Requested key does not exist.
        INI_INVALID_ARGUMENT,           ///< Invalid argument passed to a function.
        INI_PLATFORM_ERROR,             ///< Platform-specific error (e.g., mutex failure).
        INI_CLOSE_FAILED,               ///< Failed to close the file.
        INI_PRINT_ERROR,                ///< Error during printing/formatting.
        INI_FILE_BAD_FORMAT_LINE,       ///< Syntax error at a specific line.
    } ini_error_t;

    typedef struct
    {
        ini_error_t error;        ///< Error code.
        char const *inipath;      ///< Path to the INI file.
        char const *srcpath;      ///< Path to the C source file where the error occurred.
        int ini_line_number;      ///< Line number in the INI file (if applicable -> 0 if not applicable).
        int src_line_number;      ///< Line number in the source file (if applicable -> 0 if not applicable).
        char const *custommsg;    ///< Message that can be provided by the developer.
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
#if INI_OS_WINDOWS
        CRITICAL_SECTION mutex;
#elif INI_OS_APPLE
    dispatch_semaphore_t semaphore; // GCD semaphore (optional)
#else
    pthread_mutex_t mutex;
#endif
    } ini_context_t;

    /**
     * @brief Validates an INI file without loading it.
    * @param filepath Path to the INI file.
    * @return Detailed error information (success if `error == INI_SUCCESS`).
    */
    INIPARSER_API ini_error_details_t ini_good(char const *filepath);

    /**
     * @brief Loads an INI file into a context.
     * @param ctx The context to populate.
     * @param filepath Path to the INI file.
     * @return Detailed error information.
     */
    INIPARSER_API ini_error_details_t ini_load(ini_context_t *ctx, char const *filepath);

    /**
     * @brief Creates a new INI context.
     * @return A pointer to the newly created context (or NULL on failure).
     */
    INIPARSER_API ini_context_t *ini_create_context();

    /**
     * @brief Frees an INI context and its resources.
     * @param ctx The context to free.
     * @return Detailed error information.
     */
    INIPARSER_API ini_error_details_t ini_free(ini_context_t *ctx);

    /**
     * @brief Retrieves a value from an INI context.
     * @param ctx The context to query.
     * @param section The section name (or NULL for global keys).
     * @param key The key name.
     * @param[out] value Pointer to store the retrieved value (must be freed by the caller).
     * @return Detailed error information.
     */
    INIPARSER_API ini_error_details_t ini_get_value(ini_context_t const *ctx, char const *section, char const *key, char **value);
    
    /**
     * @brief Prints the contents of an INI context (for debugging).
     * @param ctx The context to print.
     * @return Detailed error information.
     */
    INIPARSER_API ini_error_details_t ini_print(ini_context_t const *ctx);

#ifdef __cplusplus
}
#endif

#endif // !INI_PARSER_H
