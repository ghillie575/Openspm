#pragma once
#include <string>
#include <vector>
namespace openspm
{
    namespace cli
    {
        int configure();
        int processCommandLine(std::string command,
                               const std::vector<std::string> &commandArgs,
                               const std::vector<std::pair<std::string, std::string>> &flagsWithValues,
                               const std::vector<std::string> &flagsWithoutValues);
        int addRepository(const std::string &repoUrl, bool skipUpdate = true, bool interactive = true);
        int updateAll();
        int updateRepositories();
        int updatePackages();
        int listPackages();
        int createDefaultConfig();
        int processFlags(const std::vector<std::pair<std::string, std::string>> &flagsWithValues,
                         const std::vector<std::string> &flagsWithoutValues);
    }
}