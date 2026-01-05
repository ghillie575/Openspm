/**
 * @file repository_manager.hpp
 * @brief Repository metadata and management operations
 * 
 * Handles repository configuration, fetching repository metadata,
 * and maintaining the list of configured repositories.
 */
#pragma once
#include <string>
#include <vector>
namespace openspm
{
    /**
     * @brief Information about a package repository
     */
    struct RepositoryInfo
    {
        std::string url;          ///< Repository base URL
        std::string name;         ///< Repository name
        std::string description;  ///< Repository description
        std::string mantainer;    ///< Repository maintainer (note: typo preserved for compatibility)
    };
    
    /**
     * @brief Get list of all configured repository URLs
     * @return Vector of repository URLs
     */
    std::vector<std::string> getRepositoryList();
    
    /**
     * @brief Add a repository to the configuration
     * @param repoInfo Repository information
     * @return true on success, false on error
     */
    bool addRepository(RepositoryInfo repoInfo);
    
    /**
     * @brief Get repository information from cache or network
     * @param repoUrl Repository URL
     * @param outInfo Structure to populate with repository info
     * @return true on success, false on error
     */
    bool getRepositoryInfo(const std::string &repoUrl, RepositoryInfo &outInfo);
    
    /**
     * @brief Fetch repository information from network
     * @param repoUrl Repository URL
     * @param outInfo Structure to populate with repository info
     * @return true on success, false on error
     */
    bool fetchRepositoryInfo(const std::string &repoUrl, RepositoryInfo &outInfo);
    
    /**
     * @brief Remove a repository from configuration
     * @param repoInfo Repository information (only URL is used)
     * @return true on success, false on error
     */
    bool removeRepository(RepositoryInfo repoInfo);
    
    /**
     * @brief Verify repository accessibility (not fully implemented)
     * @param repoUrl Repository URL
     * @return true if repository is accessible
     */
    bool verifyRepository(const std::string &repoUrl);
    
    /**
     * @brief Validate repository information structure
     * @param repoInfo Repository info to validate
     * @return true if all required fields are present
     */
    bool validateRepositoryInfo(const RepositoryInfo &repoInfo);
    
    /**
     * @brief Update metadata for all configured repositories
     * @return 0 on success, non-zero on error
     */
    int updateAllRepositories();
};