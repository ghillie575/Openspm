#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <openspm-frontend.hpp>
#include <repository_store.hpp>
int main(int argc, char *argv[])
{
    try
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

        // Parse arguments
        for (int i = 2; i < argc; ++i)
        {
            std::string arg = argv[i];

            // Handle flags
            if (arg.rfind("-", 0) == 0)
            {
                // Only --double-dash flags can have parameters
                if (arg.rfind("--", 0) == 0)
                {
                    // Support both "--flag value" and "--flag=value"
                    std::size_t eqPos = arg.find('=');
                    if (eqPos != std::string::npos)
                    {
                        // "--flag=value" form
                        std::string flag = arg.substr(0, eqPos);
                        std::string value = arg.substr(eqPos + 1);
                        flagsWithValues.emplace_back(flag, value);
                    }
                    else if (i + 1 < argc && argv[i + 1][0] != '-')
                    {
                        // "--flag value" form
                        flagsWithValues.emplace_back(arg, argv[++i]);
                    }
                    else
                    {
                        // "--flag" with no value
                        flagsWithoutValues.push_back(arg);
                    }
                }
                else
                {
                    // Single dash flags like -a, -b, -xyz
                    flagsWithoutValues.push_back(arg);
                }
            }
            else
            {
                // Not a flag -> command argument
                commandArgs.push_back(arg);
            }
        }
        openspm::frontend::processCommandLine(flagsWithValues, flagsWithoutValues, command, commandArgs);

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}