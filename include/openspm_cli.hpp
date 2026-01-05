/**
 * @file openspm_cli.hpp
 * @brief Command-line interface for OpenSPM
 */
#pragma once
#include <string>
#include <vector>
namespace openspm
{
    namespace cli
    {
        /**
         * @brief Interactive configuration setup for OpenSPM
         * @return 0 on success, non-zero on error
         */
        int configure();
        
        /**
         * @brief Process command-line arguments and execute commands
         * @param command The command to execute
         * @param commandArgs Positional arguments for the command
         * @param flagsWithValues Flags that have associated values
         * @param flagsWithoutValues Boolean flags without values
         * @return 0 on success, non-zero on error
         */
        int processCommandLine(std::string command,
                               const std::vector<std::string> &commandArgs,
                               const std::vector<std::pair<std::string, std::string>> &flagsWithValues,
                               const std::vector<std::string> &flagsWithoutValues);
        
        /**
         * @brief Add a repository to OpenSPM
         * @param repoUrl URL of the repository to add
         * @param skipUpdate Skip updating the repository after adding
         * @return 0 on success, non-zero on error
         */
        int addRepository(const std::string &repoUrl, bool skipUpdate = true);
        
        /**
         * @brief Update all repositories and packages
         * @return 0 on success, non-zero on error
         */
        int updateAll();
        
        /**
         * @brief Update repository information
         * @return 0 on success, non-zero on error
         */
        int updateRepositories();
        
        /**
         * @brief Update package lists from repositories
         * @return 0 on success, non-zero on error
         */
        int updatePackages();
        
        /**
         * @brief List all available packages
         * @return 0 on success, non-zero on error
         */
        int listPackages();
        
        /**
         * @brief Process command-line flags and update configuration
         * @param flagsWithValues Flags that have associated values
         * @param flagsWithoutValues Boolean flags without values
         * @return 0 on success, non-zero on error
         */
        int processFlags(const std::vector<std::pair<std::string, std::string>> &flagsWithValues,
                         const std::vector<std::string> &flagsWithoutValues);
    }
}