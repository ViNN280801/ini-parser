#include <cstdio>
#include <fstream>
#include <iostream>

#include "IniParser.hpp"

using namespace ini;

void createSampleIniFile()
{
    std::ofstream file("sample.ini");
    file << "[database]\n";
    file << "host=localhost\n";
    file << "port=5432\n";
    file << "username=admin\n";
    file << "password=\"secret password\"\n";
    file << "timeout=30.5\n";
    file << "ssl_enabled=true\n";
    file << "\n";
    file << "[ui]\n";
    file << "theme=dark\n";
    file << "font_size=12\n";
    file << "auto_save=yes\n";
    file << "\n";
    file << "[advanced]\n";
    file << "debug_mode=false\n";
    file << "log_level=2\n";
}

int main()
{
    try
    {
        std::cout << "=== C++11 INI Parser Wrapper Demo ===\n\n";

        // Create a sample INI file
        createSampleIniFile();
        std::cout << "Created sample.ini file\n";

        // 1. Validation
        std::cout << "\n1. File Validation:\n";
        if (IniParser::validate("sample.ini"))
        {
            std::cout << "---> sample.ini is valid\n";
        }
        else
        {
            std::cout << "✗ sample.ini is invalid\n";
            return EXIT_FAILURE;
        }

        // 2. Loading
        std::cout << "\n2. Loading file:\n";
        IniParser parser("sample.ini");
        std::cout << "---> Loaded sample.ini\n";

        // 3. Basic string access
        std::cout << "\n3. Basic string access:\n";
        std::cout << "Database host: " << parser.getString("database", "host") << "\n";
        std::cout << "UI theme: " << parser.getString("ui", "theme") << "\n";

        // 4. Type-safe access
        std::cout << "\n4. Type-safe access:\n";
        int port = parser.get<int>("database", "port");
        std::cout << "Database port (int): " << port << "\n";

        double timeout = parser.get<double>("database", "timeout");
        std::cout << "Database timeout (double): " << timeout << "\n";

        bool ssl = parser.get<bool>("database", "ssl_enabled");
        std::cout << "SSL enabled (bool): " << (ssl ? "true" : "false") << "\n";

        // 5. Default values
        std::cout << "\n5. Default values:\n";
        int nonexistent = parser.get<int>("database", "nonexistent", 999);
        std::cout << "Nonexistent key with default: " << nonexistent << "\n";

        std::string missing_section = parser.get<std::string>("missing", "key", "default_value");
        std::cout << "Missing section with default: " << missing_section << "\n";

        // 6. Checking existence
        std::cout << "\n6. Checking existence:\n";
        std::cout << "Has section 'database': " << (parser.hasSection("database") ? "yes" : "no") << "\n";
        std::cout << "Has key 'database.host': " << (parser.hasKey("database", "host") ? "yes" : "no") << "\n";
        std::cout << "Has key 'database.missing': " << (parser.hasKey("database", "missing") ? "yes" : "no") << "\n";

        // 7. Getting section names and keys
        std::cout << "\n7. Enumerating sections and keys:\n";
        auto sections = parser.getSectionNames();
        std::cout << "Sections: ";
        for (auto const &section : sections)
        {
            std::cout << "[" << section << "] ";
        }
        std::cout << "\n";

        auto dbKeys = parser.getKeyNames("database");
        std::cout << "Database keys: ";
        for (auto const &key : dbKeys)
        {
            std::cout << key << " ";
        }
        std::cout << "\n";

        // 8. Modifying values
        std::cout << "\n8. Modifying values:\n";
        parser.set<int>("database", "port", 5433);
        parser.set<bool>("ui", "auto_save", false);
        parser.setString("new_section", "new_key", "new_value");

        std::cout << "Modified port: " << parser.get<int>("database", "port") << "\n";
        std::cout << "Modified auto_save: " << parser.get<bool>("ui", "auto_save") << "\n";
        std::cout << "New value: " << parser.getString("new_section", "new_key") << "\n";

        // 9. Getting all data as containers
        std::cout << "\n9. Container interface:\n";
        auto allData = parser.getAllData();
        std::cout << "Total sections: " << allData.size() << "\n";

        auto uiSection = parser.getSection("ui");
        std::cout << "UI section has " << uiSection.size() << " keys\n";

        // 10. Printing
        std::cout << "\n10. Printing contents:\n";
        parser.print();

        // 11. Saving
        std::cout << "\n11. Saving:\n";
        parser.save("modified.ini");
        std::cout << "---> Saved to modified.ini\n";

        try
        {
            // Save just one section
            parser.saveSection("database_only.ini", "database");
            std::cout << "---> Saved database section to database_only.ini\n";
        }
        catch (const std::exception &e)
        {
            std::cout << "Note: Could not save database section (" << e.what() << ")\n";
        }

        // 12. Copy semantics
        std::cout << "\n12. Copy semantics:\n";
        IniParser parser2 = parser; // Copy constructor
        std::cout << "Copied parser has " << parser2.getSectionNames().size() << " sections\n";

        // 13. Move semantics
        std::cout << "\n13. Move semantics:\n";
        IniParser parser3 = std::move(parser2); // Move constructor
        std::cout << "Moved parser has " << parser3.getSectionNames().size() << " sections\n";
        std::cout << "Original parser is empty: " << (parser2.empty() ? "yes" : "no") << "\n";

        // 14. Exception handling
        std::cout << "\n14. Exception handling:\n";
        try
        {
            parser.getString("nonexistent", "key");
        }
        catch (const KeyNotFoundException &e)
        {
            std::cout << "Caught KeyNotFoundException: " << e.what() << "\n";
            std::cout << "Section: " << e.section() << ", Key: " << e.key() << "\n";
        }

        try
        {
            parser.get<int>("database", "host"); // Try to convert "localhost" to int
        }
        catch (const std::invalid_argument &e)
        {
            std::cout << "Caught type conversion error: " << e.what() << "\n";
        }

        // 15. Non-throwing versions
        std::cout << "\n15. Non-throwing operations:\n";
        IniParser parser4;
        auto status = parser4.loadNoThrow("nonexistent.ini");
        std::cout << "Load status for nonexistent file: " << ini_status_to_string(status) << "\n";

        // 16. Utility functions
        std::cout << "\n16. Utility functions:\n";
        std::cout << "Is valid INI file: " << (isValidIniFile("sample.ini") ? "yes" : "no") << "\n";
        std::cout << "Quick load: " << loadFile("sample.ini").getSectionNames().size() << " sections\n";

        // Removing files
        std::remove("sample.ini");
        std::remove("modified.ini");
        std::remove("database_only.ini");

        std::cout << "\n=== Demo completed successfully! ===\n";
    }
    catch (IniException const &e)
    {
        std::cerr << "INI Error: " << e.what() << "\n";
        std::cerr << "Status code: " << static_cast<int>(e.status()) << "\n";
        return EXIT_FAILURE;
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown error\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
