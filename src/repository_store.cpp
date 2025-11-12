#include <repository_store.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <global.h>
namespace openspm
{
    void addRepository(const std::string &url)
    {
        std::filesystem::path configPath(getRuntimeConfig().configDirectory);
        if (!std::filesystem::exists(configPath))
        {
            std::filesystem::create_directories(configPath);
        }
        std::filesystem::path mirrorsFilePath = configPath / "mirrors.txt";
        std::vector<std::string> mirrors = getRepositoriesFromFile(mirrorsFilePath.string());
        mirrors.push_back(url);
        
        writeRepositoriesToFile(mirrorsFilePath.string(), mirrors);
    }
    std::vector<std::string> getRepositories()
    {
        return {};
    }

    void writeRepositoriesToFile(const std::string &filename, const std::vector<std::string> &mirrors)
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file for writing: " << filename << '\n';
            return;
        }

        for (const auto &mirror : mirrors)
        {
            file << mirror << '\n';
        }
    }

    std::vector<std::string> getRepositoriesFromFile(const std::string &filename)
    {
        std::vector<std::string> mirrors;
        std::ifstream file(filename);

        if (!file.is_open())
        {
            return mirrors;
        }

        std::string line;
        while (std::getline(file, line))
        {
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty() || line[0] == '#')
                continue;

            mirrors.push_back(line);
        }

        return mirrors;
    }
} // namespace name
