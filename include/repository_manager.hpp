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
    bool addRepository(RepositoryInfo repoInfo);
    bool getRepositoryInfo(const std::string &repoUrl, RepositoryInfo &outInfo);
    bool fetchRepositoryInfo(const std::string &repoUrl, RepositoryInfo &outInfo);
    bool removeRepository(RepositoryInfo repoInfo);
    bool verifyRepository(const std::string &repoUrl);
    bool validateRepositoryInfo(const RepositoryInfo &repoInfo);
    int updateAllRepositories();
};