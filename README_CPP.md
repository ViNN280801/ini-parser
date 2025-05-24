# C++11 INI Parser Wrapper

A modern, feature-rich C++11 wrapper for the high-performance C99 INI parser. This wrapper provides RAII, exception safety, type safety, and a clean STL-compatible interface while maintaining the performance and thread safety of the underlying C implementation.

## Features

### 🚀 Performance & Safety
- **Zero-copy architecture**: Minimal overhead over C implementation
- **RAII memory management**: Automatic resource cleanup
- **Exception safety**: Strong exception safety guarantees
- **Thread safety**: Inherits thread safety from C implementation
- **Move semantics**: Efficient C++11 move operations

### 🎯 Type Safety
- **Templated getters**: Type-safe value retrieval
- **Automatic conversions**: Built-in support for common types
- **Default value support**: Graceful handling of missing keys
- **Compile-time type checking**: Catch errors at compile time

### 🛠️ Modern C++ Interface
- **STL containers**: `std::string`, `std::vector`, `std::unordered_map`
- **Exception hierarchies**: Meaningful error types
- **Range-based iteration**: Modern C++ idioms
- **Smart pointers**: Automatic memory management

## Quick Start

```cpp
#include "IniParser.hpp"
using namespace ini;

// Load and parse
IniParser parser("config.ini");

// Type-safe access
int port = parser.get<int>("database", "port");
bool ssl = parser.get<bool>("database", "ssl_enabled");
std::string host = parser.getString("database", "host");

// With defaults
int timeout = parser.get<int>("database", "timeout", 30);

// Modify values
parser.set("database", "port", 5433);
parser.save("config.ini");
```

## Comprehensive API Reference

### Construction and Loading

```cpp
// Default constructor
IniParser parser;

// Load from file
IniParser parser("config.ini");

// Validate before loading
if (IniParser::validate("config.ini")) {
    parser.load("config.ini");
}

// Non-throwing load
auto status = parser.loadNoThrow("config.ini");
if (status != INI_STATUS_SUCCESS) {
    // Handle error
}
```

### Type-Safe Value Access

```cpp
// Basic types with automatic conversion
int intValue = parser.get<int>("section", "key");
long longValue = parser.get<long>("section", "key");
float floatValue = parser.get<float>("section", "key");
double doubleValue = parser.get<double>("section", "key");
bool boolValue = parser.get<bool>("section", "key");
std::string stringValue = parser.get<std::string>("section", "key");

// With default values
int port = parser.get<int>("database", "port", 5432);
std::string theme = parser.get<std::string>("ui", "theme", "light");

// Direct string access
std::string raw = parser.getString("section", "key");
```

### Boolean Value Parsing

The wrapper supports flexible boolean parsing:
- **True values**: `"true"`, `"1"`, `"yes"`, `"on"`
- **False values**: `"false"`, `"0"`, `"no"`, `"off"`

```cpp
bool enabled = parser.get<bool>("features", "enabled");  // "yes" → true
bool debug = parser.get<bool>("dev", "debug");          // "false" → false
```

### Value Modification

```cpp
// Set typed values
parser.set<int>("database", "port", 5433);
parser.set<bool>("features", "enabled", true);
parser.set<double>("timing", "timeout", 30.5);

// Set string values
parser.setString("database", "host", "localhost");

// Chaining operations
parser.set("ui", "theme", "dark")
      .set("ui", "font_size", 12)
      .save("config.ini");
```

### Existence Checking

```cpp
// Check if section exists
if (parser.hasSection("database")) {
    // Section exists
}

// Check if key exists
if (parser.hasKey("database", "password")) {
    auto pwd = parser.getString("database", "password");
}

// Safe access pattern
auto getValue = [&](const std::string& section, const std::string& key) -> std::optional<std::string> {
    if (parser.hasKey(section, key)) {
        return parser.getString(section, key);
    }
    return std::nullopt;
};
```

### Container Interface

```cpp
// Get all data as nested maps
auto allData = parser.getAllData();
for (const auto& [sectionName, section] : allData) {
    std::cout << "[" << sectionName << "]\n";
    for (const auto& [key, value] : section) {
        std::cout << key << " = " << value << "\n";
    }
}

// Get specific section
auto dbSection = parser.getSection("database");
std::cout << "Database has " << dbSection.size() << " settings\n";

// Enumerate sections and keys
auto sections = parser.getSectionNames();
for (const auto& section : sections) {
    auto keys = parser.getKeyNames(section);
    std::cout << section << " has keys: ";
    for (const auto& key : keys) {
        std::cout << key << " ";
    }
    std::cout << "\n";
}
```

### File Operations

```cpp
// Save entire configuration
parser.save("config.ini");

// Save specific section
parser.saveSection("database.ini", "database");

// Save specific key
std::string keyName = "host";
parser.saveSection("db_host.ini", "database", &keyName);

// Validation
try {
    IniParser::validateOrThrow("config.ini");
    std::cout << "File is valid\n";
} catch (const FileException& e) {
    std::cout << "Invalid: " << e.what() << "\n";
}
```

### Exception Handling

```cpp
try {
    auto parser = loadFile("config.ini");
    int port = parser.get<int>("database", "port");
} catch (const FileException& e) {
    std::cerr << "File error: " << e.what() << "\n";
    std::cerr << "Status: " << ini_status_to_string(e.status()) << "\n";
} catch (const KeyNotFoundException& e) {
    std::cerr << "Missing key '" << e.key() 
              << "' in section '" << e.section() << "'\n";
} catch (const std::invalid_argument& e) {
    std::cerr << "Type conversion error: " << e.what() << "\n";
} catch (const IniException& e) {
    std::cerr << "INI error: " << e.what() << "\n";
}
```

### Advanced Usage

#### Custom Type Conversion

```cpp
// Extend TypeConverter for custom types
namespace ini::detail {
template<>
struct TypeConverter<std::chrono::seconds> {
    static std::chrono::seconds fromString(const std::string& str) {
        return std::chrono::seconds(std::stoi(str));
    }
    
    static std::string toString(const std::chrono::seconds& value) {
        return std::to_string(value.count());
    }
};
}

// Now you can use custom types
auto timeout = parser.get<std::chrono::seconds>("network", "timeout");
```

#### Configuration Class Pattern

```cpp
class DatabaseConfig {
public:
    explicit DatabaseConfig(const IniParser& parser) 
        : host(parser.get<std::string>("database", "host", "localhost"))
        , port(parser.get<int>("database", "port", 5432))
        , username(parser.get<std::string>("database", "username"))
        , password(parser.get<std::string>("database", "password"))
        , ssl_enabled(parser.get<bool>("database", "ssl_enabled", false))
        , timeout(parser.get<double>("database", "timeout", 30.0))
    {}
    
    void save(IniParser& parser) const {
        parser.set("database", "host", host);
        parser.set("database", "port", port);
        parser.set("database", "username", username);
        parser.set("database", "password", password);
        parser.set("database", "ssl_enabled", ssl_enabled);
        parser.set("database", "timeout", timeout);
    }

private:
    std::string host;
    int port;
    std::string username;
    std::string password;
    bool ssl_enabled;
    double timeout;
};

// Usage
IniParser parser("app.ini");
DatabaseConfig dbConfig(parser);
```

#### RAII Configuration Manager

```cpp
class ConfigManager {
public:
    explicit ConfigManager(const std::string& filepath) 
        : m_filepath(filepath), m_parser(filepath) {}
    
    ~ConfigManager() {
        if (m_modified) {
            m_parser.save(m_filepath);
        }
    }
    
    template<typename T>
    T get(const std::string& section, const std::string& key) const {
        return m_parser.get<T>(section, key);
    }
    
    template<typename T>
    void set(const std::string& section, const std::string& key, const T& value) {
        m_parser.set(section, key, value);
        m_modified = true;
    }

private:
    std::string m_filepath;
    IniParser m_parser;
    bool m_modified = false;
};
```

## Exception Hierarchy

```
std::runtime_error
    └── IniException
        ├── FileException          // File I/O errors
        └── KeyNotFoundException   // Missing sections/keys

std::invalid_argument             // Type conversion errors
```

## Thread Safety

The wrapper maintains the thread safety of the underlying C implementation:
- Multiple readers are safe
- Writers require external synchronization
- Individual operations are atomic

```cpp
// Thread-safe reading
IniParser parser("config.ini");

std::thread t1([&]() {
    auto value1 = parser.get<int>("section", "key1");
});

std::thread t2([&]() {
    auto value2 = parser.get<std::string>("section", "key2");
});
```

## Performance Characteristics

- **Construction**: O(1) - lazy initialization
- **Loading**: O(n) where n = file size
- **Access**: O(1) average case (hash table lookup)
- **Cache population**: O(n) where n = total entries
- **Memory overhead**: Minimal - data stored in C structures

## Convenience Functions

```cpp
// Quick operations
auto parser = ini::loadFile("config.ini");
bool valid = ini::isValidIniFile("config.ini");

// Fluent interface
auto result = IniParser("config.ini")
    .set("section", "key", "value")
    .save("modified.ini")
    .getAllData();
```

## Build Requirements

- **C++11 compiler** (GCC 4.8+, Clang 3.3+, MSVC 2013+)
- **C99 INI parser library** (included)
- **Standard library** (`<string>`, `<unordered_map>`, `<vector>`, etc.)

## Platform Support

- ✅ **Linux** (GCC, Clang)
- ✅ **macOS** (Clang, GCC)  
- ✅ **Windows** (MSVC, MinGW)
- ✅ **Embedded systems** (with C++11 support)

## Examples

See `examples/cpp_example.cpp` for a comprehensive demonstration of all features.
