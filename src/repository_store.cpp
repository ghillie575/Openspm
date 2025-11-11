#include <repository_store.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
namespace openspm
{
    void addRepository(const std::string &url)
    {
        std::vector<std::string> mirrors = getRepositoriesFromFile("mirrors.txt");
        mirrors.push_back(url);
        writeRepositoriesToFile("mirrors.txt", mirrors);
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
