/**
 * @file package_manager.hpp
 * @brief Package management and metadata operations
 *
 * Handles package discovery, fetching from repositories,
 * and maintaining the local package index.
 */
#pragma once
#include <string>
#include <vector>
namespace openspm
{
    /**
     * @brief Information about a package
     */
    struct PackageInfo
    {
        std::string name;                      ///< Package name
        std::string version;                   ///< Package version
        std::string description;               ///< Package description
        std::string maintainer;                ///< Package maintainer
        std::vector<std::string> dependencies; ///< List of dependency package names
        std::string tags;                      ///< Semicolon-separated tags (e.g., "bin;linux-x86_64")
        std::string url;                       ///< Download URL for the package archive
    };

    /**
     * @brief Update local package index from all configured repositories
     * @return 0 on success, non-zero on error
     */
    int updatePackages();

    /**
     * @brief Fetch package list from a specific repository
     * @param repoUrl Repository base URL
     * @param outPackages Vector to populate with package information
     * @return 0 on success, non-zero on error
     */
    int fetchPackageListFromRepository(const std::string &repoUrl, std::vector<PackageInfo> &outPackages);

    /**
     * @brief Install a package by name (not yet implemented)
     * @param packageName Name of package to install
     * @return 0 on success, non-zero on error
     */
    int collectDependencies(const std::string &packageName, std::vector<PackageInfo> &collectedPackages);
    int subCollectDependencies(const std::string &packageName, std::vector<PackageInfo> &collectedPackages, std::vector<PackageInfo> packages);
    
    int collectPackages(std::vector<PackageInfo> packages,std::vector<std::string> &collectedPackages);
    int askInstallationConfirmation(std::vector<PackageInfo> packages);
    int installCollectedPackages(const std::vector<std::string> &packageNames);
    /**
     * @brief Remove an installed package (not yet implemented)
     * @param packageName Name of package to remove
     * @return 0 on success, non-zero on error
     */
    int removePackage(const std::string &packageName);

    /**
     * @brief List all packages from local index
     * @param outPackages Vector to populate with package information
     * @return 0 on success, non-zero on error
     */
    int listPackages(std::vector<PackageInfo> &outPackages);
} // namespace openspm