#include <repository_store.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <stdexcept>
#include <global.h>

namespace openspm
{

    int addRepository(const std::string &url)
    {
        namespace fs = std::filesystem;
        fs::path configPath(getRuntimeConfig().configDirectory);

        try
        {
            if (!fs::exists(configPath))
            {
                fs::create_directories(configPath);
            }

            fs::path mirrorsFilePath = configPath / "mirrors.txt";
            std::vector<std::string> mirrors = getRepositoriesFromFile(mirrorsFilePath.string());

            // Create file if missing
            if (!fs::exists(mirrorsFilePath))
            {
                std::ofstream newFile(mirrorsFilePath.string());
                if (!newFile.is_open())
                    throw std::runtime_error("Failed to create mirrors file: " + mirrorsFilePath.string());
            }

            // Avoid duplicates
            if (std::find(mirrors.begin(), mirrors.end(), url) != mirrors.end())
            {
                return 3; // Repository already exists
            }

            mirrors.push_back(url);
            return writeRepositoriesToFile(mirrorsFilePath.string(), mirrors);
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error(std::string(e.what()));
            return 1;
        }
    }
    int writeRepositoriesToFile(const std::string &filename, const std::vector<std::string> &mirrors)
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open mirrors file for writing: " + filename);
            return 1;
        }

        for (const auto &mirror : mirrors)
        {
            file << mirror << '\n';
        }

        return 0;
    }

    std::vector<std::string> getRepositoriesFromFile(const std::string &filename)
    {
        std::vector<std::string> mirrors;
        std::ifstream file(filename);

        if (!file.is_open())
        {
            // Non-fatal: file might not exist yet
            return mirrors;
        }

        std::string line;
        while (std::getline(file, line))
        {
            // Trim
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty() || line[0] == '#')
                continue;
            mirrors.push_back(line);
        }

        return mirrors;
    }

} // namespace openspm