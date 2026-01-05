/**
 * @file repository_manager.hpp
 * @brief Repository management functionality for OpenSPM
 */
#pragma once
#include <string>
#include <vector>
namespace openspm
{
    /**
     * @brief Structure containing repository information
     */
    struct RepositoryInfo
    {
        std::string url;          ///< Repository URL
        std::string name;         ///< Repository name
        std::string description;  ///< Repository description
        std::string mantainer;    ///< Repository maintainer
    };
    
    /**
     * @brief Get list of all configured repository URLs
     * @return Vector of repository URLs
     */
    std::vector<std::string> getRepositoryList();
    
    /**
     * @brief Add a repository to the configuration
     * @param repoInfo Repository information to add
     * @return true on success, false on error
     */
    bool addRepository(RepositoryInfo repoInfo);
    
    /**
     * @brief Get repository information from local storage
     * @param repoUrl URL of the repository
     * @param outInfo Output parameter for repository information
     * @return true on success, false on error
     */
    bool getRepositoryInfo(const std::string &repoUrl, RepositoryInfo &outInfo);
    
    /**
     * @brief Fetch repository information from remote URL
     * @param repoUrl URL of the repository
     * @param outInfo Output parameter for repository information
     * @return true on success, false on error
     */
    bool fetchRepositoryInfo(const std::string &repoUrl, RepositoryInfo &outInfo);
    
    /**
     * @brief Remove a repository from the configuration
     * @param repoInfo Repository information to remove
     * @return true on success, false on error
     */
    bool removeRepository(RepositoryInfo repoInfo);
    
    /**
     * @brief Verify that a repository URL is valid and accessible
     * @param repoUrl URL of the repository to verify
     * @return true if repository is valid and accessible
     */
    bool verifyRepository(const std::string &repoUrl);
    
    /**
     * @brief Validate repository information structure
     * @param repoInfo Repository information to validate
     * @return true if repository information is valid
     */
    bool validateRepositoryInfo(const RepositoryInfo &repoInfo);
    
    /**
     * @brief Update all configured repositories
     * @return 0 on success, non-zero on error
     */
    int updateAllRepositories();
};