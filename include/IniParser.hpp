#ifndef INI_PARSER_HPP
#define INI_PARSER_HPP

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

// Include C headers
extern "C"
{
#include "ini_parser.h"
#include "ini_status.h"
}

namespace ini
{
    // Forward declarations
    class IniParser;

    // ==================== Exception Classes ====================

    /**
     * @brief Base exception class for all INI parser errors
     */
    class IniException : public std::runtime_error
    {
    public:
        explicit IniException(ini_status_t status)
            : std::runtime_error(ini_status_to_string(status)), m_status(status) {}

        explicit IniException(std::string const &message)
            : std::runtime_error(message), m_status(INI_STATUS_UNKNOWN_ERROR) {}

        ini_status_t status() const noexcept { return m_status; }

    private:
        ini_status_t m_status;
    };

    /**
     * @brief Thrown when a file operation fails
     */
    class FileException : public IniException
    {
    public:
        explicit FileException(ini_status_t status) : IniException(status) {}
        explicit FileException(std::string const &message) : IniException(message) {}
    };

    /**
     * @brief Thrown when a section or key is not found
     */
    class KeyNotFoundException : public IniException
    {
    public:
        explicit KeyNotFoundException(std::string const &section, std::string const &key)
            : IniException("Key '" + key + "' not found in section '" + section + "'"), m_section(section), m_key(key) {}

        std::string const &section() const noexcept { return m_section; }
        std::string const &key() const noexcept { return m_key; }

    private:
        std::string m_section;
        std::string m_key;
    };

    // ==================== RAII Context Wrapper ====================

    /**
     * @brief RAII wrapper for ini_context_t
     */
    class Context
    {
    public:
        Context() : m_ctx(ini_create_context())
        {
            if (!m_ctx)
            {
                throw IniException(INI_STATUS_MEMORY_ERROR);
            }
        }

        ~Context() noexcept
        {
            if (m_ctx)
            {
                ini_free(m_ctx);
            }
        }

        // Non-copyable
        Context(const Context &) = delete;
        Context &operator=(const Context &) = delete;

        // Movable
        Context(Context &&other) noexcept : m_ctx(other.m_ctx)
        {
            other.m_ctx = nullptr;
        }

        Context &operator=(Context &&other) noexcept
        {
            if (this != &other)
            {
                if (m_ctx)
                {
                    ini_free(m_ctx);
                }
                m_ctx = other.m_ctx;
                other.m_ctx = nullptr;
            }
            return *this;
        }

        ini_context_t *get() const noexcept { return m_ctx; }
        operator ini_context_t *() const noexcept { return m_ctx; }

    private:
        ini_context_t *m_ctx;
    };

    // ==================== Type Conversion Helpers ====================

    namespace detail
    {
        template <typename T>
        struct TypeConverter
        {
            static T fromString(std::string const &str);
            static std::string toString(const T &value);
        };

        // Specializations for common types
        template <>
        struct TypeConverter<std::string>
        {
            static std::string fromString(std::string const &str) { return str; }
            static std::string toString(std::string const &value) { return value; }
        };

        template <>
        struct TypeConverter<int>
        {
            static int fromString(std::string const &str) { return std::stoi(str); }
            static std::string toString(int const &value) { return std::to_string(value); }
        };

        template <>
        struct TypeConverter<long>
        {
            static long fromString(std::string const &str) { return std::stol(str); }
            static std::string toString(long const &value) { return std::to_string(value); }
        };

        template <>
        struct TypeConverter<long long>
        {
            static long long fromString(std::string const &str) { return std::stoll(str); }
            static std::string toString(long long const &value) { return std::to_string(value); }
        };

        template <>
        struct TypeConverter<float>
        {
            static float fromString(std::string const &str) { return std::stof(str); }
            static std::string toString(float const &value) { return std::to_string(value); }
        };

        template <>
        struct TypeConverter<double>
        {
            static double fromString(std::string const &str) { return std::stod(str); }
            static std::string toString(double const &value) { return std::to_string(value); }
        };

        template <>
        struct TypeConverter<bool>
        {
            static bool fromString(std::string const &str)
            {
                if (str == "true" || str == "1" || str == "yes" || str == "on")
                    return true;
                if (str == "false" || str == "0" || str == "no" || str == "off")
                    return false;
                throw std::invalid_argument("Invalid boolean value: " + str);
            }

            static std::string toString(const bool &value) { return value ? "true" : "false"; }
        };

    } // namespace detail

    // ==================== Main IniParser Class ====================

    /**
     * @brief Modern C++11 wrapper for the C99 INI parser
     *
     * Provides RAII, exception safety, and type-safe operations while
     * leveraging the underlying thread-safe C implementation.
     */
class IniParser
{
public:
        using SectionMap = std::unordered_map<std::string, std::string>;
        using DataMap = std::unordered_map<std::string, SectionMap>;

        // ==================== Constructors ====================

        /**
         * @brief Default constructor - creates empty parser
         */
        IniParser() = default;

        /**
         * @brief Constructor that loads from file
         * @param filepath Path to INI file to load
         * @throws FileException if file cannot be loaded
         */
        explicit IniParser(std::string const &filepath)
        {
            load(filepath);
        }

        /**
         * @brief Destructor
         */
        ~IniParser() = default;

        // Move semantics
        IniParser(IniParser &&) = default;
        IniParser &operator=(IniParser &&) = default;

        // Copy semantics (expensive, creates new context)
        IniParser(IniParser const &other);
        IniParser &operator=(IniParser const &other);

        // ==================== File Operations ====================

        /**
         * @brief Validates an INI file without loading it
         * @param filepath Path to validate
         * @return true if file is valid INI format
         */
        static bool validate(std::string const &filepath) noexcept;

        /**
         * @brief Validates an INI file without loading it (throwing version)
         * @param filepath Path to validate
         * @throws FileException if file is invalid
         */
        static void validateOrThrow(std::string const &filepath);

        /**
         * @brief Loads INI file
         * @param filepath Path to INI file
         * @throws FileException if loading fails
         */
        void load(std::string const &filepath);

        /**
         * @brief Loads INI file (non-throwing version)
         * @param filepath Path to INI file
         * @return Status code
         */
        ini_status_t loadNoThrow(std::string const &filepath) noexcept;

        /**
         * @brief Saves to INI file
         * @param filepath Path to save to
         * @throws FileException if saving fails
         */
        void save(std::string const &filepath) const;

        /**
         * @brief Saves specific section to INI file
         * @param filepath Path to save to
         * @param section Section name
         * @param key Optional key name (nullptr saves entire section)
         * @throws FileException if saving fails
         */
        void saveSection(std::string const &filepath, std::string const &section,
                         const std::string *key = nullptr) const;

        // ==================== Value Access ====================

        /**
         * @brief Gets string value
         * @param section Section name
         * @param key Key name
         * @return Value as string
         * @throws KeyNotFoundException if not found
         */
        std::string getString(std::string const &section, std::string const &key) const;

        /**
         * @brief Gets typed value
         * @tparam T Type to convert to
         * @param section Section name
         * @param key Key name
         * @return Value converted to type T
         * @throws KeyNotFoundException if not found
         * @throws std::invalid_argument if conversion fails
         */
        template <typename T>
        T get(std::string const &section, std::string const &key) const
        {
            auto str = getString(section, key);
            return detail::TypeConverter<T>::fromString(str);
        }

        /**
         * @brief Gets value with default
         * @tparam T Type to convert to
         * @param section Section name
         * @param key Key name
         * @param defaultValue Default value if not found
         * @return Value or default
         */
        template <typename T>
        T get(std::string const &section, std::string const &key, const T &defaultValue) const noexcept
        {
            try
            {
                return get<T>(section, key);
            }
            catch (...)
            {
                return defaultValue;
            }
        }

        /**
         * @brief Checks if key exists
         * @param section Section name
         * @param key Key name
         * @return true if key exists
         */
        bool hasKey(std::string const &section, std::string const &key) const noexcept;

        /**
         * @brief Checks if section exists
         * @param section Section name
         * @return true if section exists
         */
        bool hasSection(std::string const &section) const noexcept;

        // ==================== Value Modification ====================

        /**
         * @brief Sets string value
         * @param section Section name
         * @param key Key name
         * @param value Value to set
         * @return Reference to this parser for chaining
         */
        IniParser &setString(std::string const &section, std::string const &key, std::string const &value);

        /**
         * @brief Sets typed value
         * @tparam T Type of value
         * @param section Section name
         * @param key Key name
         * @param value Value to set
         * @return Reference to this parser for chaining
         */
        template <typename T>
        IniParser &set(std::string const &section, std::string const &key, const T &value)
        {
            setString(section, key, detail::TypeConverter<T>::toString(value));
            return *this;
        }

        // ==================== Container Interface ====================

        /**
         * @brief Gets all data as nested map (expensive operation)
         * @return Complete data structure
         */
        DataMap getAllData() const;

        /**
         * @brief Gets section as map
         * @param section Section name
         * @return Section data
         * @throws KeyNotFoundException if section not found
         */
        SectionMap getSection(std::string const &section) const;

        /**
         * @brief Gets all section names
         * @return Vector of section names
         */
        std::vector<std::string> getSectionNames() const;

        /**
         * @brief Gets all key names in section
         * @param section Section name
         * @return Vector of key names
         * @throws KeyNotFoundException if section not found
         */
        std::vector<std::string> getKeyNames(std::string const &section) const;

        // ==================== Utility ====================

        /**
         * @brief Prints contents to stream
         * @param stream Output stream
         */
        void print(std::ostream &stream = std::cout) const;

        /**
         * @brief Clears all data
         */
        void clear();

        /**
         * @brief Checks if parser is empty
         * @return true if no data loaded
         */
        bool empty() const noexcept;

private:
        Context m_context;
        mutable DataMap m_data; ///< Cached data (populated on demand)
        mutable bool m_dataCached = false;

        // Helper methods
        void ensureContext();
        void invalidateCache() noexcept
        {
            m_dataCached = false;
            m_data.clear();
        }
        void populateCache() const;
        static void checkStatus(ini_status_t status);
    };

    // ==================== Convenience Functions ====================

    /**
     * @brief Quick load and parse INI file
     * @param filepath Path to INI file
     * @return Loaded parser instance
     * @throws FileException if loading fails
     */
    inline IniParser loadFile(std::string const &filepath)
    {
        return IniParser(filepath);
    }

    /**
     * @brief Quick validation of INI file
     * @param filepath Path to validate
     * @return true if valid
     */
    inline bool isValidIniFile(std::string const &filepath) noexcept
    {
        return IniParser::validate(filepath);
    }

} // namespace ini

#endif // !INI_PARSER_HPP
