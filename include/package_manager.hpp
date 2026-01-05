/**
 * @file package_manager.hpp
 * @brief Package management functionality for OpenSPM
 */
#pragma once
#include <string>
#include <vector>
namespace openspm
{
    /**
     * @brief Structure containing package information
     */
    struct PackageInfo
    {
        std::string name;         ///< Package name
        std::string version;      ///< Package version
        std::string description;  ///< Package description
        std::string maintainer;   ///< Package maintainer
        std::string tags;         ///< Semicolon-separated package tags
        std::string url;          ///< Package download URL
    };
    
    /**
     * @brief Update package lists from all repositories
     * @return 0 on success, non-zero on error
     */
    int updatePackages();
    
    /**
     * @brief Fetch package list from a repository
     * @param repoUrl URL of the repository
     * @param outPackages Output parameter for the package list
     * @return 0 on success, non-zero on error
     */
    int fetchPackageListFromRepository(const std::string &repoUrl, std::vector<PackageInfo> &outPackages);
    
    /**
     * @brief Install a package by name
     * @param packageName Name of the package to install
     * @return 0 on success, non-zero on error
     */
    int installPackage(const std::string &packageName);
    
    /**
     * @brief Remove an installed package
     * @param packageName Name of the package to remove
     * @return 0 on success, non-zero on error
     */
    int removePackage(const std::string &packageName);
    
    /**
     * @brief List all installed packages
     * @param outPackages Output parameter for the installed package list
     * @return 0 on success, non-zero on error
     */
    int listInstalledPackages(std::vector<PackageInfo> &outPackages);
} // namespace openspm