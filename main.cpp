#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <openspm_cli.hpp>
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