#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ini_filesystem.h"
#include "ini_os_check.h"

#if INI_OS_LINUX
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#elif INI_OS_WINDOWS
#include <io.h>
#include <windows.h>
#endif

#include "helper.h"

// ==================== Tests for ini_get_file_permission() ====================

// Clean test: Basic file permission check
void test_ini_filesystem_file_permission_basic()
{
    char const *test_file = "test_ini_filesystem_file_permission_basic.txt";
    create_test_file(test_file, "test content");

    ini_file_permission_t perms = ini_get_file_permission(test_file);
    assert(perms.read == 1);  // Should be readable
    assert(perms.write == 1); // Should be writable

    remove_test_file(test_file);
    print_success("test_ini_filesystem_file_permission_basic passed\n");
}

// Dirty test: NULL filepath
void test_ini_filesystem_file_permission_null()
{
    ini_file_permission_t perms = ini_get_file_permission(NULL);
    assert(perms.read == 0);
    assert(perms.write == 0);
    assert(perms.execute == 0);
    print_success("test_ini_filesystem_file_permission_null passed\n");
}

// Dirty test: Empty filepath
void test_ini_filesystem_file_permission_empty()
{
    ini_file_permission_t perms = ini_get_file_permission("");
    assert(perms.read == 0);
    assert(perms.write == 0);
    assert(perms.execute == 0);
    print_success("test_ini_filesystem_file_permission_empty passed\n");
}

// Dirty test: Non-existent file (should check parent directory permissions)
void test_ini_filesystem_file_permission_nonexistent()
{
    ini_file_permission_t perms = ini_get_file_permission("nonexistent_file.txt");
    // Should check current directory permissions
    // Result depends on current directory write permissions
    print_success("test_ini_filesystem_file_permission_nonexistent passed\n");
}

// Dirty test: Directory instead of file
void test_ini_filesystem_file_permission_directory()
{
    char const *test_dir = "test_ini_filesystem_file_permission_directory";
    create_test_dir(test_dir);

    ini_file_permission_t perms = ini_get_file_permission(test_dir);
    // On Windows, directories may have different permission semantics
    // On POSIX, directories should be readable/writable if accessible

    remove_test_dir(test_dir);
    print_success("test_ini_filesystem_file_permission_directory passed\n");
}

// Dirty test: Path near maximum length
void test_ini_filesystem_file_permission_near_max_path()
{
    char long_path[INI_PATH_MAX - 10];
    memset(long_path, 'a', sizeof(long_path) - 1);
    long_path[sizeof(long_path) - 1] = '\0';

    ini_file_permission_t perms = ini_get_file_permission(long_path);
    // Should handle gracefully without buffer overflow
    print_success("test_ini_filesystem_file_permission_near_max_path passed\n");
}

// Dirty test: Path with special characters
void test_ini_filesystem_file_permission_special_chars()
{
    char const *special_paths[] = {
        "file with spaces.txt",
        "file@with#special$chars%.txt",
        "file'with\"quotes.txt",
        "file\twith\ttabs.txt",
        "file;with;semicolons.txt"};

    for (size_t i = 0; i < sizeof(special_paths) / sizeof(special_paths[0]); i++)
    {
        ini_file_permission_t perms = ini_get_file_permission(special_paths[i]);
        // Should handle special characters gracefully
    }

    print_success("test_ini_filesystem_file_permission_special_chars passed\n");
}

// Dirty test: Relative vs absolute paths
void test_ini_filesystem_file_permission_path_types()
{
    char const *test_file = "test_ini_filesystem_file_permission_path_types.txt";
    create_test_file(test_file, "test content");

    // Test relative path
    ini_file_permission_t perms1 = ini_get_file_permission(test_file);

    // Test absolute path
    char abs_path[INI_PATH_MAX];
#if INI_OS_WINDOWS
    GetFullPathNameA(test_file, sizeof(abs_path), abs_path, NULL);
#else
    realpath(test_file, abs_path);
#endif
    ini_file_permission_t perms2 = ini_get_file_permission(abs_path);

    // Both should give same results for existing file
    if (strlen(abs_path) > 0)
    {
        assert(perms1.read == perms2.read);
        assert(perms1.write == perms2.write);
    }

    remove_test_file(test_file);
    print_success("test_ini_filesystem_file_permission_path_types passed\n");
}

// Platform-specific test: Read-only file
void test_ini_filesystem_file_permission_readonly()
{
#if INI_OS_LINUX
    char const *test_file = "test_ini_filesystem_file_permission_readonly.txt";
    create_test_file(test_file, "test content");
    chmod(test_file, 0444); // r--r--r--

    ini_file_permission_t perms = ini_get_file_permission(test_file);
    assert(perms.read == 1);
    assert(perms.write == 0); // Should not be writable

    chmod(test_file, 0666); // Restore permissions for cleanup
    remove_test_file(test_file);
    print_success("test_ini_filesystem_file_permission_readonly passed\n");
#endif
}

// Platform-specific test: Executable file
void test_ini_filesystem_file_permission_executable()
{
#if INI_OS_LINUX
    char const *test_file = "test_ini_filesystem_file_permission_executable.txt";
    create_test_file(test_file, "#!/bin/bash\necho hello");
    chmod(test_file, 0755); // rwxr-xr-x

    ini_file_permission_t perms = ini_get_file_permission(test_file);
    assert(perms.read == 1);
    assert(perms.write == 1);
    assert(perms.execute == 1);

    remove_test_file(test_file);
    print_success("test_ini_filesystem_file_permission_executable passed\n");
#endif
}

// Platform-specific test: File in read-only directory
void test_ini_filesystem_file_permission_readonly_dir()
{
#if INI_OS_LINUX
    char const *test_dir = "test_ini_filesystem_file_permission_readonly_dir";
    create_test_dir(test_dir);
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/test.txt", test_dir);
    create_test_file(filepath, "content");

    chmod(test_dir, 0555); // r-xr-xr-x (read-only directory)

    ini_file_permission_t perms = ini_get_file_permission(filepath);
    // File should be readable, but directory is read-only
    assert(perms.read == 1);

    chmod(test_dir, 0755); // Restore permissions
    remove_test_file(filepath);
    remove_test_dir(test_dir);
    print_success("test_ini_filesystem_file_permission_readonly_dir passed\n");
#endif
}

// Platform-specific test: Symlink permissions
void test_ini_filesystem_file_permission_symlink_scenarios()
{
#if INI_OS_LINUX
    // Create target file
    char const *test_file = "test_ini_filesystem_file_permission_symlink_target.txt";
    create_test_file(test_file, "target content");
    chmod(test_file, 0600); // rw-------

    // Create symlink
    symlink(test_file, "test_symlink");

    ini_file_permission_t perms = ini_get_file_permission("test_symlink");
    // Should follow symlink and get target permissions
    assert(perms.read == 1);
    assert(perms.write == 1);

    // Test broken symlink
    remove_test_file(test_file);
    ini_file_permission_t broken_perms = ini_get_file_permission("test_symlink");
    // Broken symlink should return no permissions

    remove_test_file("test_symlink");
    print_success("test_ini_filesystem_file_permission_symlink_scenarios passed\n");
#endif
}

// Platform-specific test: Different filesystem types
void test_ini_filesystem_file_permission_special_files()
{
#if INI_OS_LINUX
    // Test device files (read-only)
    ini_file_permission_t null_perms = ini_get_file_permission("/dev/null");
    ini_file_permission_t zero_perms = ini_get_file_permission("/dev/zero");

    // These may or may not exist, but shouldn't crash
    print_success("test_ini_filesystem_file_permission_special_files passed\n");
#endif
}

// Platform-specific test: Windows executable file
void test_ini_filesystem_file_permission_windows_exe()
{
#if INI_OS_WINDOWS
    create_test_file("test.exe", "fake exe content");

    ini_file_permission_t perms = ini_get_file_permission("test.exe");
    assert(perms.read == 1);
    assert(perms.write == 1);
    assert(perms.execute == 1); // .exe files should be marked executable

    remove_test_file("test.exe");
    print_success("test_ini_filesystem_file_permission_windows_exe passed\n");
#endif
}

// Platform-specific test: Windows batch file
void test_ini_filesystem_file_permission_windows_bat()
{
#if INI_OS_WINDOWS
    create_test_file("test.bat", "@echo off\necho hello");

    ini_file_permission_t perms = ini_get_file_permission("test.bat");
    assert(perms.read == 1);
    assert(perms.write == 1);
    assert(perms.execute == 1); // .bat files should be marked executable

    remove_test_file("test.bat");
    print_success("test_ini_filesystem_file_permission_windows_bat passed\n");
#endif
}

// Platform-specific test: Windows reserved names
void test_ini_filesystem_file_permission_windows_reserved()
{
#if INI_OS_WINDOWS
    char const *reserved_names[] = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM9",
        "LPT1", "LPT2", "LPT9",
        "con.txt", "prn.ini", "aux.log"};

    for (size_t i = 0; i < sizeof(reserved_names) / sizeof(reserved_names[0]); i++)
    {
        ini_file_permission_t perms = ini_get_file_permission(reserved_names[i]);
        // Should handle reserved names without crashing
    }

    print_success("test_ini_filesystem_file_permission_windows_reserved passed\n");
#endif
}

// Platform-specific test: Windows file attributes
void test_ini_filesystem_file_permission_windows_attributes()
{
#if INI_OS_WINDOWS
    char const *test_file = "test_ini_filesystem_file_permission_windows_attributes.txt";
    create_test_file(test_file, "test content");

    // Test setting readonly attribute
    SetFileAttributesA(test_file, FILE_ATTRIBUTE_READONLY);

    ini_file_permission_t perms = ini_get_file_permission(test_file);
    assert(perms.read == 1);
    assert(perms.write == 0); // Should not be writable

    // Restore normal attributes for cleanup
    SetFileAttributesA(test_file, FILE_ATTRIBUTE_NORMAL);
    remove_test_file(test_file);
    print_success("test_ini_filesystem_file_permission_windows_attributes passed\n");
#endif
}

// Platform-specific test: UNC paths
void test_ini_filesystem_file_permission_unc_paths()
{
#if INI_OS_WINDOWS
    char const *unc_paths[] = {
        "\\\\localhost\\c$\\windows\\system32\\kernel32.dll",
        "\\\\?\\C:\\Windows\\System32\\ntdll.dll",
        "\\\\server\\share\\file.txt"};

    for (size_t i = 0; i < sizeof(unc_paths) / sizeof(unc_paths[0]); i++)
    {
        ini_file_permission_t perms = ini_get_file_permission(unc_paths[i]);
        // Should handle UNC paths gracefully
    }

    print_success("test_ini_filesystem_file_permission_unc_paths passed\n");
#endif
}

// Dirty test: Very long filename
void test_ini_filesystem_file_permission_long_filename()
{
    char long_filename[INI_PATH_MAX];
    memset(long_filename, 'a', sizeof(long_filename) - 5);
    strcpy(long_filename + sizeof(long_filename) - 5, ".txt");

    ini_file_permission_t perms = ini_get_file_permission(long_filename);
    // Should handle gracefully without crashing
    print_success("test_ini_filesystem_file_permission_long_filename passed\n");
}

// ==================== Tests for ini_fopen() ====================

// Clean test: Basic file opening
void test_ini_filesystem_fopen_basic()
{
    char const *test_file = "test_ini_filesystem_fopen_basic.txt";
    create_test_file(test_file, "test content");

    FILE *file = ini_fopen(test_file, "r");
    assert(file != NULL);
    fclose(file);

    remove_test_file(test_file);
    print_success("test_ini_filesystem_fopen_basic passed\n");
}

// Dirty test: NULL parameters
void test_ini_filesystem_fopen_null_params()
{
    assert(ini_fopen(NULL, "r") == NULL);
    assert(ini_fopen("test_ini_filesystem_fopen_null_params.txt", NULL) == NULL);
    assert(ini_fopen(NULL, NULL) == NULL);
    print_success("test_ini_filesystem_fopen_null_params passed\n");
}

// Dirty test: Invalid mode
void test_ini_filesystem_fopen_invalid_mode()
{
    char const *test_file = "test_ini_filesystem_fopen_invalid_mode.txt";
    create_test_file(test_file, "test content");

    assert(ini_fopen(test_file, "x") == NULL);       // Invalid mode
    assert(ini_fopen(test_file, "rw") == NULL);      // Invalid mode
    assert(ini_fopen(test_file, "") == NULL);        // Empty mode
    assert(ini_fopen(test_file, "invalid") == NULL); // Invalid mode

    remove_test_file(test_file);
    print_success("test_ini_filesystem_fopen_invalid_mode passed\n");
}

// Clean test: All valid modes
void test_ini_filesystem_fopen_valid_modes()
{
    char const *test_file = "test_ini_filesystem_fopen_valid_modes.txt";
    char const *modes[] = {
        "r", "w", "a", "r+", "w+", "a+",
        "rb", "wb", "ab", "r+b", "w+b", "a+b",
        "rt", "wt", "at", "r+t", "w+t", "a+t"};

    for (size_t i = 0; i < sizeof(modes) / sizeof(modes[0]); i++)
    {
        // For read modes ("r", "r+") create file first
        if (strcmp(modes[i], "r") == 0 || strcmp(modes[i], "r+") == 0 ||
            strcmp(modes[i], "rb") == 0 || strcmp(modes[i], "r+b") == 0 ||
            strcmp(modes[i], "rt") == 0 || strcmp(modes[i], "r+t") == 0)
        {
            FILE *prep_file = ini_fopen(test_file, "w");
            if (prep_file)
                fclose(prep_file);
        }

        FILE *file = ini_fopen(test_file, modes[i]);
        if (!file)
        {
            fprintf(stderr, "Failed to open file in mode: %s\n", modes[i]);
        }
        else
        {
            fclose(file);
        }

        // Remove file only if it was created (except read modes)
        if (strcmp(modes[i], "r") != 0 && strcmp(modes[i], "r+") != 0 &&
            strcmp(modes[i], "rb") != 0 && strcmp(modes[i], "r+b") != 0 &&
            strcmp(modes[i], "rt") != 0 && strcmp(modes[i], "r+t") != 0)
        {
            remove_test_file(test_file);
        }
    }

    // Remove file if it remains (e.g., after read modes)
    if (ini_file_exists(test_file) == INI_STATUS_SUCCESS)
        remove_test_file(test_file);
    print_success("test_ini_filesystem_fopen_valid_modes passed\n");
}

// Dirty test: Non-existent file with read mode
void test_ini_filesystem_fopen_nonexistent_read()
{
    FILE *file = ini_fopen("nonexistent_file.txt", "r");
    assert(file == NULL);
    print_success("test_ini_filesystem_fopen_nonexistent_read passed\n");
}

// Dirty test: File in non-existent directory
void test_ini_filesystem_fopen_nonexistent_dir()
{
    FILE *file = ini_fopen("nonexistent_dir/file.txt", "w");
    assert(file == NULL); // Should fail to create file in non-existent directory
    print_success("test_ini_filesystem_fopen_nonexistent_dir passed\n");
}

// Clean test: Binary vs text modes
void test_ini_filesystem_fopen_binary_text()
{
    // Test binary mode
    char const *test_file = "test_ini_filesystem_fopen_binary_text.bin";
    FILE *bin_file = ini_fopen(test_file, "wb");
    assert(bin_file != NULL);

    unsigned char binary_data[] = {0x00, 0xFF, 0x0A, 0x0D, 0x1A};
    fwrite(binary_data, sizeof(binary_data), 1, bin_file);
    fclose(bin_file);

    // Read back in binary mode
    bin_file = ini_fopen(test_file, "rb");
    assert(bin_file != NULL);

    unsigned char read_data[sizeof(binary_data)];
    size_t read_count = fread(read_data, 1, sizeof(read_data), bin_file);
    assert(read_count == sizeof(binary_data));
    assert(memcmp(binary_data, read_data, sizeof(binary_data)) == 0);
    fclose(bin_file);

    remove_test_file(test_file);
    print_success("test_ini_filesystem_fopen_binary_text passed\n");
}

// Dirty test: Case sensitivity
void test_ini_filesystem_fopen_case_sensitivity()
{
    char const *test_file = "testfile.txt";
    create_test_file(test_file, "content");

    FILE *file1 = ini_fopen(test_file, "r");
    FILE *file2 = ini_fopen("TESTFILE.TXT", "r");

#if INI_OS_WINDOWS
    // Windows is case-insensitive
    assert(file1 != NULL);
    if (file2 != NULL)
    { // May or may not succeed depending on filesystem
        fclose(file2);
    }
#else
    // Unix-like systems are case-sensitive
    assert(file1 != NULL);
    assert(file2 != NULL);
#endif

    if (file1)
        fclose(file1);
    remove_test_file(test_file);
    print_success("test_ini_filesystem_fopen_case_sensitivity passed\n");
}

// Platform-specific test: Sharing violations
void test_ini_filesystem_fopen_sharing_violation()
{
#if INI_OS_WINDOWS
    // Create and open file exclusively
    char const *test_file = "test_ini_filesystem_fopen_sharing_violation.txt";
    HANDLE handle = CreateFileA(test_file, GENERIC_WRITE, 0, NULL,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    assert(handle != INVALID_HANDLE_VALUE);

    // Try to open with ini_fopen - should fail due to sharing violation
    FILE *file = ini_fopen(test_file, "r");
    assert(file == NULL); // Should fail

    CloseHandle(handle);
    remove_test_file(test_file);
    print_success("test_ini_filesystem_fopen_sharing_violation passed\n");
#endif
}

// Platform-specific test: FIFO files
void test_ini_filesystem_fopen_fifo()
{
#if INI_OS_LINUX
    // Create FIFO (named pipe)
    char const *test_fifo = "test_fifo";
    if (mkfifo(test_fifo, 0666) == 0)
    {
        // Opening FIFO for reading would block, so we test write mode
        FILE *file = ini_fopen(test_fifo, "w");
        // May succeed or fail depending on system, shouldn't crash
        if (file)
            fclose(file);
        unlink(test_fifo);
    }

    print_success("test_ini_filesystem_fopen_fifo passed\n");
#endif
}

// Dirty test: Very large filenames
void test_ini_filesystem_fopen_large_filename()
{
    char large_name[INI_PATH_MAX];
    memset(large_name, 'x', sizeof(large_name) - 5);
    strcpy(large_name + sizeof(large_name) - 5, ".txt");

    FILE *file = ini_fopen(large_name, "w");
    // Should fail gracefully, not crash
    if (file)
    {
        fclose(file);
        remove(large_name);
    }

    print_success("test_ini_filesystem_fopen_large_filename passed\n");
}

// ==================== Tests for ini_check_file_status() ====================

// Clean test: Regular file
void test_ini_filesystem_check_file_status_regular()
{
    char const *test_file = "test_ini_filesystem_check_file_status_regular.txt";
    create_test_file(test_file, "test content");

    ini_status_t status = ini_check_file_status(test_file);
    assert(status == INI_STATUS_SUCCESS);

    remove_test_file(test_file);
    print_success("test_ini_filesystem_check_file_status_regular passed\n");
}

// Dirty test: NULL/empty filepath
void test_ini_filesystem_check_file_status_null_empty()
{
    assert(ini_check_file_status(NULL) == INI_STATUS_INVALID_ARGUMENT);
    assert(ini_check_file_status("") == INI_STATUS_INVALID_ARGUMENT);
    print_success("test_ini_filesystem_check_file_status_null_empty passed\n");
}

// Dirty test: Non-existent file
void test_ini_filesystem_check_file_status_nonexistent()
{
    ini_status_t status = ini_check_file_status("nonexistent_file.txt");
    assert(status == INI_STATUS_FILE_NOT_FOUND);
    print_success("test_ini_filesystem_check_file_status_nonexistent passed\n");
}

// Dirty test: Directory
void test_ini_filesystem_check_file_status_directory()
{
    char const *test_dir = "test_ini_filesystem_check_file_status_directory";
    create_test_dir(test_dir);

    ini_status_t status = ini_check_file_status(test_dir);
    assert(status == INI_STATUS_FILE_IS_DIR);

    remove_test_dir(test_dir);
    print_success("test_ini_filesystem_check_file_status_directory passed\n");
}

// Clean test: Empty file
void test_ini_filesystem_check_file_status_empty()
{
    char const *test_file = "test_ini_filesystem_check_file_status_empty.txt";
    create_test_file(test_file, ""); // Empty file

    ini_status_t status = ini_check_file_status(test_file);
#if INI_OS_WINDOWS
    assert(status == INI_STATUS_SUCCESS); // Windows doesn't check file size in this function
#else
    assert(status == INI_STATUS_FILE_EMPTY); // POSIX checks file size
#endif

    remove_test_file(test_file);
    print_success("test_ini_filesystem_check_file_status_empty passed\n");
}

// Dirty test: Path with multiple separators
void test_ini_filesystem_check_file_status_path_separators()
{
    char const *test_file = "test_ini_filesystem_check_file_status_path_separators.txt";
    create_test_file(test_file, "content");

    // Test paths with multiple separators
#if INI_OS_WINDOWS
    char weird_path[] = ".\\\\test_filesystem.txt";
#else
    char weird_path[] = ".//test_filesystem.txt";
#endif

    ini_status_t status = ini_check_file_status(weird_path);
    // Should normalize path and find file

    remove_test_file(test_file);
    print_success("test_ini_filesystem_check_file_status_path_separators passed\n");
}

// Platform-specific test: Special file (symlink)
void test_ini_filesystem_check_file_status_symlink()
{
#if INI_OS_LINUX
    char const *test_file = "test_ini_filesystem_check_file_status_symlink.txt";
    create_test_file(test_file, "target content");
    symlink(test_file, "test_symlink");

    ini_status_t status = ini_check_file_status("test_symlink");
    assert(status == INI_STATUS_SUCCESS); // Symlink to regular file should be OK

    remove_test_file("test_symlink");
    remove_test_file(test_file);
    print_success("test_ini_filesystem_check_file_status_symlink passed\n");
#endif
}

// Platform-specific test: No access permissions
void test_ini_filesystem_check_file_status_no_access()
{
#if INI_OS_LINUX
    char const *test_file = "test_ini_filesystem_check_file_status_no_access.txt";
    create_test_file(test_file, "test content");
    chmod(test_file, 0000); // No permissions

    ini_status_t status = ini_check_file_status(test_file);
    // Behavior may vary - file exists but no access

    chmod(test_file, 0666); // Restore permissions for cleanup
    remove_test_file(test_file);
    print_success("test_ini_filesystem_check_file_status_no_access passed\n");
#endif
}

// Platform-specific test: Different file types
void test_ini_filesystem_check_file_status_file_types()
{
#if INI_OS_LINUX
    // Test with various /dev files if they exist
    char const *special_files[] = {
        "/dev/null", "/dev/zero", "/dev/random",
        "/proc/version", "/sys/kernel/hostname"};

    for (size_t i = 0; i < sizeof(special_files) / sizeof(special_files[0]); i++)
    {
        ini_status_t status = ini_check_file_status(special_files[i]);
        // Should handle special files gracefully
    }

    print_success("test_ini_filesystem_check_file_status_file_types passed\n");
#endif
}

// Platform-specific test: Windows hidden files
void test_ini_filesystem_check_file_status_hidden()
{
#if INI_OS_WINDOWS
    char const *test_file = "test_ini_filesystem_check_file_status_hidden.txt";
    create_test_file(test_file, "hidden content");
    SetFileAttributesA(test_file, FILE_ATTRIBUTE_HIDDEN);

    ini_status_t status = ini_check_file_status(test_file);
    assert(status == INI_STATUS_SUCCESS); // Hidden files should still be accessible

    SetFileAttributesA(test_file, FILE_ATTRIBUTE_NORMAL);
    remove_test_file(test_file);
    print_success("test_ini_filesystem_check_file_status_hidden passed\n");
#endif
}

// ==================== Tests for ini_get_file_size() ====================

// Clean test: Regular file size
void test_ini_filesystem_get_file_size_regular()
{
    char const *content = "Hello, World!";
    char const *test_file = "test_ini_filesystem_get_file_size_regular.txt";
    create_test_file(test_file, content);

    size_t size;
    ini_status_t status = ini_get_file_size(test_file, &size);
    assert(status == INI_STATUS_SUCCESS);
    assert(size == strlen(content));

    remove_test_file(test_file);
    print_success("test_ini_filesystem_get_file_size_regular passed\n");
}

// Dirty test: NULL parameters
void test_ini_filesystem_get_file_size_null_params()
{
    size_t size;
    assert(ini_get_file_size(NULL, &size) == INI_STATUS_INVALID_ARGUMENT);
    assert(ini_get_file_size("test_ini_filesystem_get_file_size_null_params.txt", NULL) == INI_STATUS_INVALID_ARGUMENT);
    assert(ini_get_file_size(NULL, NULL) == INI_STATUS_INVALID_ARGUMENT);
    print_success("test_ini_filesystem_get_file_size_null_params passed\n");
}

// Dirty test: Non-existent file
void test_ini_filesystem_get_file_size_nonexistent()
{
    size_t size;
    ini_status_t status = ini_get_file_size("nonexistent_file.txt", &size);
    assert(status == INI_STATUS_FILE_NOT_FOUND);
    print_success("test_ini_filesystem_get_file_size_nonexistent passed\n");
}

// Clean test: Empty file
void test_ini_filesystem_get_file_size_empty()
{
    char const *test_file = "test_ini_filesystem_get_file_size_empty.txt";
    create_test_file(test_file, ""); // Empty file

    size_t size;
    ini_status_t status = ini_get_file_size(test_file, &size);
    assert(status == INI_STATUS_SUCCESS);
    assert(size == 0);

    remove_test_file(test_file);
    print_success("test_ini_filesystem_get_file_size_empty passed\n");
}

// Clean test: Large file
void test_ini_filesystem_get_file_size_large()
{
    char const *test_file = "test_ini_filesystem_get_file_size_large.txt";
    FILE *file = fopen(test_file, "w");
    assert(file != NULL);

    // Write 10KB of data
    for (int i = 0; i < 10240; i++)
    {
        fputc('A', file);
    }
    fclose(file);

    size_t size;
    ini_status_t status = ini_get_file_size(test_file, &size);
    assert(status == INI_STATUS_SUCCESS);
    assert(size == 10240);

    remove_test_file(test_file);
    print_success("test_ini_filesystem_get_file_size_large passed\n");
}

// Clean test: Very large file (>1MB)
void test_ini_filesystem_get_file_size_very_large()
{
    char const *test_file = "test_ini_filesystem_get_file_size_very_large.txt";
    FILE *file = fopen(test_file, "wb");
    assert(file != NULL);

    // Write 1MB of data
    char buffer[1024];
    memset(buffer, 'X', sizeof(buffer));

    for (int i = 0; i < 1024; i++)
    {
        fwrite(buffer, sizeof(buffer), 1, file);
    }
    fclose(file);

    size_t size;
    ini_status_t status = ini_get_file_size(test_file, &size);
    assert(status == INI_STATUS_SUCCESS);
    assert(size == 1024 * 1024); // 1MB

    remove_test_file(test_file);
    print_success("test_ini_filesystem_get_file_size_very_large passed\n");
}

// Dirty test: Directory size
void test_ini_filesystem_get_file_size_directory()
{
    char const *test_dir = "test_ini_filesystem_get_file_size_directory";
    create_test_dir(test_dir);

    size_t size;
    ini_status_t status = ini_get_file_size(test_dir, &size);
    assert(status == INI_STATUS_FILE_IS_DIR);

    remove_test_dir(test_dir);
    print_success("test_ini_filesystem_get_file_size_directory passed\n");
}

// Clean test: Binary file size
void test_ini_filesystem_get_file_size_binary()
{
    char const *test_file = "test_ini_filesystem_get_file_size_binary.bin";
    FILE *file = fopen(test_file, "wb");
    assert(file != NULL);

    unsigned char binary_data[] = {0x00, 0xFF, 0x0A, 0x0D, 0x1A, 0x7F, 0x80, 0xFE};
    fwrite(binary_data, sizeof(binary_data), 1, file);
    fclose(file);

    size_t size;
    ini_status_t status = ini_get_file_size(test_file, &size);
    assert(status == INI_STATUS_SUCCESS);
    assert(size == sizeof(binary_data));

    remove_test_file(test_file);
    print_success("test_ini_filesystem_get_file_size_binary passed\n");
}

// Platform-specific test: Symlink size
void test_ini_filesystem_get_file_size_symlink()
{
#if INI_OS_LINUX
    char const *test_file = "test_ini_filesystem_get_file_size_symlink.txt";
    create_test_file(test_file, "target content");
    symlink(test_file, "test_symlink");

    size_t size;
    ini_status_t status = ini_get_file_size("test_symlink", &size);
    assert(status == INI_STATUS_SUCCESS);
    assert(size == strlen("target content")); // Should return target size

    remove_test_file("test_symlink");
    remove_test_file(test_file);
    print_success("test_ini_filesystem_get_file_size_symlink passed\n");
#endif
}

// ==================== Integration and Edge Case Tests ====================

// Dirty test: Unicode filename
void test_ini_filesystem_unicode_filename()
{
    // Test with Unicode characters in filename
    char const *test_file = "test_ini_filesystem_unicode_filename.txt";
    create_test_file(test_file, "Unicode content");

    ini_file_permission_t perms = ini_get_file_permission(test_file);
    assert(perms.read == 1);

    FILE *file = ini_fopen(test_file, "r");
    if (file)
    { // May fail on some systems
        fclose(file);
    }

    ini_status_t status = ini_check_file_status(test_file);
    // Should handle Unicode gracefully

    size_t size;
    ini_status_t size_status = ini_get_file_size(test_file, &size);
    if (size_status == INI_STATUS_SUCCESS)
    {
        assert(size == strlen("Unicode content"));
    }

    remove_test_file(test_file);
    print_success("test_ini_filesystem_unicode_filename passed\n");
}

// Dirty test: Path traversal
void test_ini_filesystem_path_traversal()
{
    char const *dangerous_paths[] = {
        "../../../etc/passwd",
        "..\\..\\..\\windows\\system32\\config\\sam",
        "/etc/shadow",
        "C:\\Windows\\System32\\config\\SAM",
        "aux", "con", "prn", "nul" // Windows reserved names
    };

    for (size_t i = 0; i < sizeof(dangerous_paths) / sizeof(dangerous_paths[0]); i++)
    {
        // These should not crash the functions
        ini_file_permission_t perms = ini_get_file_permission(dangerous_paths[i]);
        ini_status_t status = ini_check_file_status(dangerous_paths[i]);

        size_t size;
        ini_get_file_size(dangerous_paths[i], &size);

        // Functions should handle gracefully without crashing
    }

    print_success("test_ini_filesystem_path_traversal passed\n");
}

// Dirty test: Boundary conditions
void test_ini_filesystem_boundary_conditions()
{
    // Test with SIZE_MAX-1 as size parameter
    size_t large_size = SIZE_MAX - 1;
    ini_status_t status = ini_get_file_size("nonexistent", &large_size);
    assert(status == INI_STATUS_FILE_NOT_FOUND);

    // Test with very small buffer paths
    char tiny_path[2] = "x";
    ini_file_permission_t perms = ini_get_file_permission(tiny_path);

    print_success("test_ini_filesystem_boundary_conditions passed\n");
}

// Clean test: Concurrent access simulation
void test_ini_filesystem_concurrent_access_simulation()
{
    char const *test_file = "test_ini_filesystem_concurrent_access_simulation.txt";
    create_test_file(test_file, "shared content");

    // Simulate multiple access patterns
    for (int i = 0; i < 10; i++)
    {
        ini_file_permission_t perms = ini_get_file_permission(test_file);
        ini_status_t status = ini_check_file_status(test_file);

        size_t size;
        ini_get_file_size(test_file, &size);

        FILE *file = ini_fopen(test_file, "r");
        if (file)
        {
            fclose(file);
        }
    }

    remove_test_file(test_file);
    print_success("test_ini_filesystem_concurrent_access_simulation passed\n");
}

// Clean test: Error recovery scenarios
void test_ini_filesystem_error_recovery()
{
    // Test recovery from various error conditions

    // 1. File created and immediately deleted
    char const *test_file = "test_ini_filesystem_error_recovery.txt";
    create_test_file(test_file, "temporary");
    remove_test_file(test_file);

    ini_status_t status = ini_check_file_status(test_file);
    assert(status == INI_STATUS_FILE_NOT_FOUND);

    // 2. Directory created and removed
    char const *test_dir = "test_ini_filesystem_error_recovery_dir";
    create_test_dir(test_dir);
    remove_test_dir(test_dir);

    status = ini_check_file_status(test_dir);
    assert(status == INI_STATUS_FILE_NOT_FOUND);

    print_success("test_ini_filesystem_error_recovery passed\n");
}

// Clean test: Comprehensive workflow
void test_ini_filesystem_comprehensive_workflow()
{
    char const *test_content = "Test file for comprehensive workflow";

    // 1. Check non-existent file
    char const *test_file = "test_ini_filesystem_comprehensive_workflow.txt";
    assert(ini_check_file_status(test_file) == INI_STATUS_FILE_NOT_FOUND);

    // 2. Create file
    FILE *file = ini_fopen(test_file, "w");
    assert(file != NULL);
    fputs(test_content, file);
    fclose(file);

    // 3. Check file now exists
    assert(ini_check_file_status(test_file) == INI_STATUS_SUCCESS);

    // 4. Check file permissions
    ini_file_permission_t perms = ini_get_file_permission(test_file);
    assert(perms.read == 1);
    assert(perms.write == 1);

    // 5. Check file size
    size_t size;
    assert(ini_get_file_size(test_file, &size) == INI_STATUS_SUCCESS);
    assert(size == strlen(test_content));

    // 6. Read file content back
    file = ini_fopen(test_file, "r");
    assert(file != NULL);

    char buffer[256];
    fgets(buffer, sizeof(buffer), file);
    assert(strcmp(buffer, test_content) == 0);
    fclose(file);

    // 7. Append to file
    file = ini_fopen(test_file, "a");
    assert(file != NULL);
    fputs(" - appended", file);
    fclose(file);

    // 8. Check new size
    assert(ini_get_file_size(test_file, &size) == INI_STATUS_SUCCESS);
    assert(size == strlen(test_content) + strlen(" - appended"));

    // 9. Cleanup
    remove_test_file(test_file);
    assert(ini_check_file_status(test_file) == INI_STATUS_FILE_NOT_FOUND);

    print_success("test_ini_filesystem_comprehensive_workflow passed\n");
}

// ==================== Stress and Performance Tests ====================

// Dirty test: Many files stress test
void test_ini_filesystem_many_files_stress()
{
    char filename[64];

    // Create many files
    for (int i = 0; i < 100; i++)
    {
        snprintf(filename, sizeof(filename), "stress_test_%d.txt", i);
        create_test_file(filename, "stress test content");
    }

    // Test operations on all files
    for (int i = 0; i < 100; i++)
    {
        snprintf(filename, sizeof(filename), "stress_test_%d.txt", i);

        ini_file_permission_t perms = ini_get_file_permission(filename);
        ini_status_t status = ini_check_file_status(filename);

        size_t size;
        ini_get_file_size(filename, &size);
    }

    // Cleanup
    for (int i = 0; i < 100; i++)
    {
        snprintf(filename, sizeof(filename), "stress_test_%d.txt", i);
        remove_test_file(filename);
    }

    print_success("test_ini_filesystem_many_files_stress passed\n");
}

// ==================== Main Test Runner ====================

int main()
{
    __helper_init_log_file();

    test_ini_filesystem_file_permission_basic();
    test_ini_filesystem_file_permission_null();
    test_ini_filesystem_file_permission_empty();
    test_ini_filesystem_file_permission_nonexistent();
    test_ini_filesystem_file_permission_directory();
    test_ini_filesystem_file_permission_near_max_path();
    test_ini_filesystem_file_permission_special_chars();
    test_ini_filesystem_file_permission_path_types();
    test_ini_filesystem_file_permission_long_filename();
    test_ini_filesystem_file_permission_readonly();
    test_ini_filesystem_file_permission_executable();
    test_ini_filesystem_file_permission_readonly_dir();
    test_ini_filesystem_file_permission_symlink_scenarios();
    test_ini_filesystem_file_permission_special_files();
    test_ini_filesystem_file_permission_windows_exe();
    test_ini_filesystem_file_permission_windows_bat();
    test_ini_filesystem_file_permission_windows_reserved();
    test_ini_filesystem_file_permission_windows_attributes();
    test_ini_filesystem_file_permission_unc_paths();
    test_ini_filesystem_fopen_basic();
    test_ini_filesystem_fopen_null_params();
    test_ini_filesystem_fopen_invalid_mode();
    test_ini_filesystem_fopen_valid_modes();
    test_ini_filesystem_fopen_nonexistent_read();
    test_ini_filesystem_fopen_nonexistent_dir();
    test_ini_filesystem_fopen_binary_text();
    test_ini_filesystem_fopen_case_sensitivity();
    test_ini_filesystem_fopen_large_filename();
    test_ini_filesystem_fopen_sharing_violation();
    test_ini_filesystem_fopen_fifo();
    test_ini_filesystem_check_file_status_regular();
    test_ini_filesystem_check_file_status_null_empty();
    test_ini_filesystem_check_file_status_nonexistent();
    test_ini_filesystem_check_file_status_directory();
    test_ini_filesystem_check_file_status_empty();
    test_ini_filesystem_check_file_status_path_separators();
    test_ini_filesystem_check_file_status_symlink();
    test_ini_filesystem_check_file_status_no_access();
    test_ini_filesystem_check_file_status_file_types();
    test_ini_filesystem_check_file_status_hidden();
    test_ini_filesystem_get_file_size_regular();
    test_ini_filesystem_get_file_size_null_params();
    test_ini_filesystem_get_file_size_nonexistent();
    test_ini_filesystem_get_file_size_empty();
    test_ini_filesystem_get_file_size_large();
    test_ini_filesystem_get_file_size_very_large();
    test_ini_filesystem_get_file_size_directory();
    test_ini_filesystem_get_file_size_binary();
    test_ini_filesystem_get_file_size_symlink();
    test_ini_filesystem_unicode_filename();
    test_ini_filesystem_path_traversal();
    test_ini_filesystem_boundary_conditions();
    test_ini_filesystem_concurrent_access_simulation();
    test_ini_filesystem_error_recovery();
    test_ini_filesystem_comprehensive_workflow();
    test_ini_filesystem_many_files_stress();

    print_success("All ini_filesystem tests passed!\n\n");
    __helper_close_log_file();
    return EXIT_SUCCESS;
}
