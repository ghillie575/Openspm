/**
 * @file main.cpp
 * @brief Entry point for OpenSPM package manager
 * 
 * Parses command-line arguments and delegates to the CLI processor.
 * 
 * @mainpage OpenSPM - Open Source Package Manager
 * 
 * @section intro_sec Introduction
 * 
 * OpenSPM is a lightweight, cross-platform package manager designed for distributing 
 * and managing software packages from remote repositories.
 * 
 * @section features_sec Features
 * 
 * - Simple package installation and removal
 * - Support for multiple remote repositories
 * - Tag-based package compatibility system
 * - Automatic dependency resolution
 * - Compressed archive-based package storage
 * - Colorized terminal output
 * 
 * @section usage_sec Executable Usage
 * 
 * @subsection usage_basic Basic Usage
 * 
 * The basic syntax for running OpenSPM is:
 * @code
 * openspm <command> [args...] [flags...]
 * @endcode
 * 
 * @subsection usage_config Configuration
 * 
 * Run the interactive configuration wizard:
 * @code
 * openspm configure
 * @endcode
 * 
 * This will prompt you to set:
 * - Data directory (default: /etc/openspm/)
 * - Target installation directory (default: /usr/local/)
 * - Color output preference
 * 
 * @subsection usage_repos Repository Management
 * 
 * Add a repository:
 * @code
 * openspm add-repo <repository-url>
 * @endcode
 * 
 * Update repository information:
 * @code
 * openspm update-repos
 * @endcode
 * 
 * Update package lists:
 * @code
 * openspm update-packages
 * @endcode
 * 
 * Update both repositories and packages:
 * @code
 * openspm update
 * @endcode
 * 
 * @subsection usage_packages Package Management
 * 
 * List available packages:
 * @code
 * openspm list
 * @endcode
 * 
 * Install a package:
 * @code
 * openspm install <package-name>
 * @endcode
 * 
 * Remove a package:
 * @code
 * openspm remove <package-name>
 * @endcode
 * 
 * @subsection usage_flags Command-line Flags
 * 
 * Global flags that can be used with any command:
 * 
 * - --data-dir <path>: Override the data directory
 * - --target-dir <path>: Override the target installation directory
 * - --tags <tags>: Override supported tags (semicolon-separated)
 * - --no-color or -nc: Disable colored output
 * 
 * Example:
 * @code
 * openspm install mypackage --data-dir /custom/data --no-color
 * @endcode
 * 
 * @subsection usage_version Version Information
 * 
 * Display version and build date:
 * @code
 * openspm version
 * @endcode
 * 
 * @section building_sec Building from Source
 * 
 * @subsection build_deps Build Dependencies
 * 
 * - CMake 3.10.0 or higher
 * - C++17 compatible compiler (GCC, Clang, or MSVC)
 * - pkg-config
 * - libarchive
 * - libzstd (Zstandard compression)
 * 
 * @subsection build_steps Build Steps
 * 
 * @code
 * # Install system dependencies (Ubuntu/Debian)
 * sudo apt-get install cmake build-essential pkg-config libzstd-dev libarchive-dev
 * 
 * # Configure CMake
 * cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
 * 
 * # Build
 * cmake --build build --config Release --parallel
 * 
 * # The executable will be at build/openspm
 * @endcode
 * 
 * @section modules_sec Code Modules
 * 
 * The codebase is organized into several modules:
 * - @ref openspm::cli - Command-line interface
 * - Archive - Archive management for package storage
 * - Config - Configuration management
 * - Logger - Logging utilities
 * - Package Manager - Package installation and removal
 * - Repository Manager - Repository management
 * - Utils - Utility functions
 * 
 * @section author_sec Author
 * 
 * Developed by ghillie575
 */
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <openspm_cli.hpp>

/**
 * @brief Main entry point for OpenSPM
 * 
 * Parses command-line arguments into commands, arguments, and flags,
 * then passes them to the CLI processor for execution.
 * 
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return 0 on success, non-zero on error
 */
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: openspm <command> [args...] [flags...]\n";
        return 1;
    }

    std::string command = argv[1];

    std::vector<std::string> commandArgs;
    std::vector<std::pair<std::string, std::string>> flagsWithValues;
    std::vector<std::string> flagsWithoutValues;

    // Parse command-line arguments
    for (int i = 2; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg.rfind("-", 0) == 0)
        {
            if (arg.rfind("--", 0) == 0)
            {
                // Long flag: check for value
                std::size_t eqPos = arg.find('=');
                if (eqPos != std::string::npos)
                {
                    // Flag with value using '=' separator
                    std::string flag = arg.substr(0, eqPos);
                    std::string value = arg.substr(eqPos + 1);
                    flagsWithValues.emplace_back(flag, value);
                }
                else if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    // Flag with value as next argument
                    flagsWithValues.emplace_back(arg, argv[++i]);
                }
                else
                {
                    // Boolean flag without value
                    flagsWithoutValues.push_back(arg);
                }
            }
            else
            {
                // Short flag (without value)
                flagsWithoutValues.push_back(arg);
            }
        }
        else
        {
            // Positional argument
            commandArgs.push_back(arg);
        }
    }
    return openspm::cli::processCommandLine(command, commandArgs, flagsWithValues, flagsWithoutValues);
}