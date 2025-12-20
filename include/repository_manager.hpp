#pragma once
#include <string>
#include <vector>
namespace openspm
{
    struct RepositoryInfo
    {
        std::string url;
        std::string name;
        std::string description;
        std::string mantainer;
    };
    std::vector<std::string> getRepositoryList();
    bool addRepository(const std::string &repoUrl);
    bool removeRepository(const std::string &repoUrl);
    bool verifyRepository(const std::string &repoUrl);
};