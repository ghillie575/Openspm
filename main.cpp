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
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: openspm <command> [args...] [flags...]\n";
        return 1;
    }
#ifdef _WIN32

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
    {
        if (geteuid() != 0)
        {
            std::cerr << "Administrator (root) privileges required.\n";
            return 1;
        }
    }
#endif
    std::string command = argv[1];

    std::vector<std::string> commandArgs;
    std::vector<std::pair<std::string, std::string>> flagsWithValues;
    std::vector<std::string> flagsWithoutValues;

    for (int i = 2; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg.rfind("-", 0) == 0)
        {
            if (arg.rfind("--", 0) == 0)
            {
                std::size_t eqPos = arg.find('=');
                if (eqPos != std::string::npos)
                {
                    std::string flag = arg.substr(0, eqPos);
                    std::string value = arg.substr(eqPos + 1);
                    flagsWithValues.emplace_back(flag, value);
                }
                else if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    flagsWithValues.emplace_back(arg, argv[++i]);
                }
                else
                {
                    flagsWithoutValues.push_back(arg);
                }
            }
            else
            {
                flagsWithoutValues.push_back(arg);
            }
        }
        else
        {
            commandArgs.push_back(arg);
        }
    }
    return openspm::cli::processCommandLine(command, commandArgs, flagsWithValues, flagsWithoutValues);
}