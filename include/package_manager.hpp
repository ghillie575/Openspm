#pragma once
#include <string>
#include <vector>
namespace openspm
{
    struct PackageInfo
    {
        std::string name;
        std::string version;
        std::string description;
        std::string maintainer;
        std::string tags;
        std::string url;
    };
    int updatePackages();
    int fetchPackageListFromRepository(const std::string &repoUrl, std::vector<PackageInfo> &outPackages);
    int installPackage(const std::string &packageName);
    int removePackage(const std::string &packageName);
    int listPackages(std::vector<PackageInfo> &outPackages);
} // namespace openspm