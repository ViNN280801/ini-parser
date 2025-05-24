#include "IniParser.hpp"
#include <cstdio>
#include <iostream>
#include <sstream>

extern "C"
{
#include "ini_hash_table.h"
#include "ini_parser.h"
}

namespace ini
{

    // ==================== Helper Functions ====================

    void IniParser::checkStatus(ini_status_t status)
    {
        switch (status)
        {
        case INI_STATUS_SUCCESS:
            return;
        case INI_STATUS_FILE_NOT_FOUND:
        case INI_STATUS_FILE_EMPTY:
        case INI_STATUS_FILE_OPEN_FAILED:
        case INI_STATUS_FILE_BAD_FORMAT:
        case INI_STATUS_CLOSE_FAILED:
        case INI_STATUS_FILE_PERMISSION_DENIED:
        case INI_STATUS_FILE_IS_DIR:
            throw FileException(status);
        case INI_STATUS_MEMORY_ERROR:
        case INI_STATUS_LACK_OF_MEMORY:
            throw IniException(status);
        default:
            throw IniException(status);
        }
    }

    void IniParser::ensureContext()
    {
        if (!m_context.get())
        {
            m_context = Context{};
        }
    }

    // ==================== Copy Operations ====================

    IniParser::IniParser(const IniParser &other)
    {
        if (other.m_context.get())
        {
            // Ensure we have a valid context first
            ensureContext();

            // Get the data from the other parser
            other.populateCache();

            // Copy data into our new context without using the cache
            for (const auto &section : other.m_data)
            {
                for (const auto &pair : section.second)
                {
                    // Use the low-level setString but don't rely on cache
                    auto *section_ht = ini_get_section_ht(m_context.get()->sections, section.first.c_str());
                    if (!section_ht)
                    {
                        // Create new section
                        section_ht = ini_ht_create();
                        if (!section_ht)
                        {
                            throw IniException(INI_STATUS_MEMORY_ERROR);
                        }
                        ini_store_section_ht(m_context.get()->sections, section.first.c_str(), section_ht);
                    }

                    // Set the key-value pair directly
                    auto result = ini_ht_set(section_ht, pair.first.c_str(), pair.second.c_str());
                    if (!result)
                    {
                        throw IniException(INI_STATUS_MEMORY_ERROR);
                    }
                }
            }

            // Invalidate cache so it will be rebuilt from our new context
            invalidateCache();
        }
    }

    IniParser &IniParser::operator=(const IniParser &other)
    {
        if (this != &other)
        {
            clear();
            if (other.m_context.get())
            {
                // Ensure we have a valid context
                ensureContext();

                // Get the data from the other parser
                other.populateCache();

                // Copy data into our new context
                for (const auto &section : other.m_data)
                {
                    for (const auto &pair : section.second)
                    {
                        // Use the low-level approach for better performance and safety
                        auto *section_ht = ini_get_section_ht(m_context.get()->sections, section.first.c_str());
                        if (!section_ht)
                        {
                            // Create new section
                            section_ht = ini_ht_create();
                            if (!section_ht)
                            {
                                throw IniException(INI_STATUS_MEMORY_ERROR);
                            }
                            ini_store_section_ht(m_context.get()->sections, section.first.c_str(), section_ht);
                        }

                        // Set the key-value pair directly
                        auto result = ini_ht_set(section_ht, pair.first.c_str(), pair.second.c_str());
                        if (!result)
                        {
                            throw IniException(INI_STATUS_MEMORY_ERROR);
                        }
                    }
                }

                // Invalidate cache so it will be rebuilt from our new context
                invalidateCache();
            }
        }
        return *this;
    }

    // ==================== Static Validation ====================

    bool IniParser::validate(std::string const &filepath) noexcept
    {
        try
        {
            return ini_good(filepath.c_str()) == INI_STATUS_SUCCESS;
        }
        catch (...)
        {
            return false;
        }
    }

    void IniParser::validateOrThrow(std::string const &filepath)
    {
        auto status = ini_good(filepath.c_str());
        checkStatus(status);
    }

    // ==================== File Operations ====================

    void IniParser::load(std::string const &filepath)
    {
        ensureContext();
        auto status = ini_load(m_context.get(), filepath.c_str());
        checkStatus(status);
        invalidateCache();
    }

    ini_status_t IniParser::loadNoThrow(std::string const &filepath) noexcept
    {
        try
        {
            ensureContext();
            auto status = ini_load(m_context.get(), filepath.c_str());
            if (status == INI_STATUS_SUCCESS)
            {
                invalidateCache();
            }
            return status;
        }
        catch (...)
        {
            return INI_STATUS_UNKNOWN_ERROR;
        }
    }

    void IniParser::save(std::string const &filepath) const
    {
        if (!m_context.get())
        {
            throw IniException("No data to save - parser is empty");
        }
        auto status = ini_save(m_context.get(), filepath.c_str());
        checkStatus(status);
    }

    void IniParser::saveSection(std::string const &filepath, std::string const &section,
                                const std::string *key) const
    {
        if (!m_context.get())
        {
            throw IniException("No data to save - parser is empty");
        }
        auto status = ini_save_section_value(m_context.get(), filepath.c_str(),
                                             section.c_str(), key ? key->c_str() : nullptr);
        checkStatus(status);
    }

    // ==================== Value Access ====================

    std::string IniParser::getString(std::string const &section, std::string const &key) const
    {
        if (!m_context.get())
        {
            throw KeyNotFoundException(section, key);
        }

        char *value = nullptr;
        auto status = ini_get_value(m_context.get(), section.c_str(), key.c_str(), &value);

        if (status == INI_STATUS_SECTION_NOT_FOUND || status == INI_STATUS_KEY_NOT_FOUND)
        {
            throw KeyNotFoundException(section, key);
        }

        checkStatus(status);

        if (!value)
        {
            throw KeyNotFoundException(section, key);
        }

        std::string result(value);
        free(value); // C function allocates with malloc
        return result;
    }

    bool IniParser::hasKey(std::string const &section, std::string const &key) const noexcept
    {
        try
        {
            getString(section, key);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool IniParser::hasSection(std::string const &section) const noexcept
    {
        if (!m_context.get())
        {
            return false;
        }

        // Try to get the section hash table
        auto *section_ht = ini_get_section_ht(m_context.get()->sections, section.c_str());
        return section_ht != nullptr;
    }

    // ==================== Value Modification ====================

    IniParser &IniParser::setString(std::string const &section, std::string const &key, std::string const &value)
    {
        ensureContext();

        // Get or create section
        auto *section_ht = ini_get_section_ht(m_context.get()->sections, section.c_str());
        if (!section_ht)
        {
            // Create new section
            section_ht = ini_ht_create();
            if (!section_ht)
            {
                throw IniException(INI_STATUS_MEMORY_ERROR);
            }
            ini_store_section_ht(m_context.get()->sections, section.c_str(), section_ht);
        }

        // Set the key-value pair
        auto result = ini_ht_set(section_ht, key.c_str(), value.c_str());
        if (!result)
        {
            throw IniException(INI_STATUS_MEMORY_ERROR);
        }

        invalidateCache();
        return *this;
    }

    // ==================== Container Interface ====================

    void IniParser::populateCache() const
    {
        if (m_dataCached || !m_context.get())
        {
            return;
        }

        m_data.clear();

        // Iterate through all sections
        ini_ht_iterator_t sections_it = ini_ht_iterator(m_context.get()->sections);
        char *section_name;
        char *section_ptr_str;

        while (ini_ht_next(&sections_it, &section_name, &section_ptr_str) == INI_STATUS_SUCCESS)
        {
            auto *section_ht = ini_get_section_ht(m_context.get()->sections, section_name);
            if (section_ht)
            {
                SectionMap section_map;

                // Iterate through key-value pairs in this section
                ini_ht_iterator_t pairs_it = ini_ht_iterator(section_ht);
                char *key;
                char *value;

                while (ini_ht_next(&pairs_it, &key, &value) == INI_STATUS_SUCCESS)
                {
                    section_map[key] = value;
                }

                m_data[section_name] = std::move(section_map);
            }
        }

        m_dataCached = true;
    }

    IniParser::DataMap IniParser::getAllData() const
    {
        populateCache();
        return m_data;
    }

    IniParser::SectionMap IniParser::getSection(std::string const &section) const
    {
        populateCache();
        auto it = m_data.find(section);
        if (it == m_data.end())
        {
            throw KeyNotFoundException(section, "");
        }
        return it->second;
    }

    std::vector<std::string> IniParser::getSectionNames() const
    {
        populateCache();
        std::vector<std::string> names;
        names.reserve(m_data.size());

        for (const auto &pair : m_data)
        {
            names.push_back(pair.first);
        }

        return names;
    }

    std::vector<std::string> IniParser::getKeyNames(std::string const &section) const
    {
        auto section_map = getSection(section); // This will throw if section not found
        std::vector<std::string> names;
        names.reserve(section_map.size());

        for (const auto &pair : section_map)
        {
            names.push_back(pair.first);
        }

        return names;
    }

    // ==================== Utility ====================

    void IniParser::print(std::ostream &stream) const
    {
        if (!m_context.get())
        {
            stream << "[Empty INI Parser]\n";
            return;
        }

#if defined(__GLIBC__) || defined(__APPLE__)
        // Use open_memstream on systems that support it
        char *buffer = nullptr;
        size_t size = 0;
        FILE *memstream = open_memstream(&buffer, &size);

        if (memstream)
        {
            auto status = ini_print(memstream, m_context.get());
            fclose(memstream);

            if (status == INI_STATUS_SUCCESS && buffer)
            {
                stream.write(buffer, size);
                free(buffer);
                return;
            }

            if (buffer)
            {
                free(buffer);
            }
        }
#endif

        // Fallback: use populateCache and print manually
        populateCache();
        for (const auto &section : m_data)
        {
            if (!section.first.empty())
            {
                stream << "[" << section.first << "]\n";
            }
            else
            {
                stream << "[Global]\n";
            }

            for (const auto &pair : section.second)
            {
                stream << "  " << pair.first << " = " << pair.second << "\n";
            }
            stream << "\n";
        }
    }

    void IniParser::clear()
    {
        m_context = Context{}; // This will properly destroy the old context
        invalidateCache();
    }

    bool IniParser::empty() const noexcept
    {
        if (!m_context.get())
        {
            return true;
        }

        // Check if there are any sections
        ini_ht_iterator_t it = ini_ht_iterator(m_context.get()->sections);
        char *section_name;
        char *section_ptr_str;

        return ini_ht_next(&it, &section_name, &section_ptr_str) != INI_STATUS_SUCCESS;
    }

} // namespace ini
