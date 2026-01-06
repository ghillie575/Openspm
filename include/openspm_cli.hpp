/**
 * @file openspm_cli.hpp
 * @brief Command-line interface for OpenSPM package manager
 * 
 * This module handles all user-facing commands, argument parsing,
 * and interactive configuration.
 */
#pragma once
#include <string>
#include <vector>
namespace openspm
{
    namespace cli
    {
        /**
         * @brief Run interactive configuration wizard
         * @return 0 on success, non-zero on error
         */
        int configure();
        
        /**
         * @brief Process and execute command-line arguments
         * @param command The main command (e.g., "add-repo", "list-packages")
         * @param commandArgs Positional arguments for the command
         * @param flagsWithValues Flags that have associated values (e.g., --data-dir=/path)
         * @param flagsWithoutValues Boolean flags (e.g., --debug, --no-color)
         * @return 0 on success, non-zero on error
         */
        int processCommandLine(std::string command,
                               const std::vector<std::string> &commandArgs,
                               const std::vector<std::pair<std::string, std::string>> &flagsWithValues,
                               const std::vector<std::string> &flagsWithoutValues);
        
        /**
         * @brief Add a package repository
         * @param repoUrl URL of the repository
         * @param skipUpdate If true, skip package index update
         * @param interactive If true, prompt for confirmation
         * @return 0 on success, non-zero on error
         */
        int addRepository(const std::string &repoUrl, bool skipUpdate = true, bool interactive = true);
        
        /**
         * @brief Update both repositories and package indices
         * @return 0 on success, non-zero on error
         */
        int updateAll();
        
        /**
         * @brief Update repository metadata
         * @return 0 on success, non-zero on error
         */
        int updateRepositories();
        
        /**
         * @brief Update package index from all repositories
         * @return 0 on success, non-zero on error
         */
        int updatePackages();
        
        /**
         * @brief List all compatible packages
         * @return 0 on success, non-zero on error
         */
        int listPackages();
        
        /**
         * @brief Create default configuration without user interaction
         * @return 0 on success, non-zero on error
         */
        int createDefaultConfig();
        
        /**
         * @brief Collect and install a package with its dependencies
         * @param packageName Name of package to collect and install
         * @return 0 on success, non-zero on error
         */
        int collectPackages(const std::string &packageName);
        
        /**
         * @brief Process command-line flags and update configuration
         * @param flagsWithValues Flags with values
         * @param flagsWithoutValues Boolean flags
         * @return 0 on success, non-zero on error
         */
        int processFlags(const std::vector<std::pair<std::string, std::string>> &flagsWithValues,
                         const std::vector<std::string> &flagsWithoutValues);
    }
}