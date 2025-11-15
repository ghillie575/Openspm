#ifndef PACKAGE_INFO_H
#define PACKAGE_INFO_H

#include <string>
#include <vector>

struct PackageInfo {
    std::string package;
    std::string version;
    std::string description;
    std::string req;
    std::string path;
    std::string checksum;
};
std::string serializeRepository(const std::vector<PackageInfo> &packages);
std::vector<PackageInfo> deserializeRepository(const std::string& yamlString);

#endif
