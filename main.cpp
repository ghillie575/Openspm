/**
 * @file main.cpp
 * @brief Entry point for OpenSPM package manager
 * 
 * Handles command-line argument parsing, privilege checking,
 * and dispatching to the CLI module for command execution.
 */
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#ifdef _WIN32
#include <windows.h>
#include <sddl.h>
#else
#include <unistd.h>
#endif
#include <openspm_cli.hpp>

/**
 * @brief Main entry point
 * 
 * Performs the following steps:
 * 1. Checks for administrator/root privileges
 * 2. Parses command-line arguments into command, args, and flags
 * 3. Dispatches to CLI module for command execution
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return 0 on success, non-zero on error
 */
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: openspm <command> [args...] [flags...]\n";
        return 1;
    }
#ifdef _WIN32
    // Check for administrator privileges on Windows
    {
        BOOL isAdmin = FALSE;
        PSID administratorsGroup = NULL;
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        if (AllocateAndInitializeSid(&NtAuthority, 2,
                                     SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                                     0, 0, 0, 0, 0, 0, &administratorsGroup))
        {
            if (!CheckTokenMembership(NULL, administratorsGroup, &isAdmin))
                isAdmin = FALSE;
            FreeSid(administratorsGroup);
        }
        if (!isAdmin)
        {
            std::cerr << "Administrator privileges required.\n";
            return 1;
        }
    }
#else
    // Check for root privileges on Unix-like systems
    {
        if (geteuid() != 0)
        {
            std::cerr << "Administrator (root) privileges required.\n";
            return 1;
        }
    }
#endif
    std::string command = argv[1];

    // Parse arguments and flags
    std::vector<std::string> commandArgs;
    std::vector<std::pair<std::string, std::string>> flagsWithValues;
    std::vector<std::string> flagsWithoutValues;

    for (int i = 2; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg.rfind("-", 0) == 0)
        {
            // Long flags (--flag)
            if (arg.rfind("--", 0) == 0)
            {
                std::size_t eqPos = arg.find('=');
                if (eqPos != std::string::npos)
                {
                    // Flag with value using = syntax (--flag=value)
                    std::string flag = arg.substr(0, eqPos);
                    std::string value = arg.substr(eqPos + 1);
                    flagsWithValues.emplace_back(flag, value);
                }
                else if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    // Flag with value as next argument (--flag value)
                    flagsWithValues.emplace_back(arg, argv[++i]);
                }
                else
                {
                    // Boolean flag (--flag)
                    flagsWithoutValues.push_back(arg);
                }
            }
            else
            {
                // Short flags (-f)
                flagsWithoutValues.push_back(arg);
            }
        }
        else
        {
            // Positional argument
            commandArgs.push_back(arg);
        }
    }
    
    // Dispatch to CLI module
    return openspm::cli::processCommandLine(command, commandArgs, flagsWithValues, flagsWithoutValues);
}