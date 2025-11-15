#include <utils.hpp>
#include <string>
#include <logger.hpp>
#include <global.h>
#include <httplib.h>
#include <regex>
#include <filesystem>
#include <repository_store.hpp>
#include <package_info.h>
#include <vector>
#include <fstream>
#include <yaml-cpp/yaml.h>
namespace openspm::frontend::utils
{
    int verifyRepository(const std::string &url)
    {
        try
        {
            std::regex url_regex(
                R"(^(https?)://([^/:]+)(?::(\d+))?(.*)$)");

            std::smatch match;

            if (!std::regex_match(url, match, url_regex))
            {
                throw std::runtime_error("Invalid URL: " + url);
            }

            std::string scheme = match[1];
            std::string host = match[2];
            std::string port = match[3].matched ? match[3].str()
                                                : (scheme == "https" ? "443" : "80");
            std::string path = match[4].matched && !match[4].str().empty()
                                   ? match[4].str()
                                   : "/";
            if (scheme == "http")
            {
                logMessage(color::yellow + "Warning: The repository URL uses HTTP which is not secure. Consider using HTTPS." + color::reset, true);
                httplib::Client cli(host.c_str(), std::stoi(port));
                auto res = cli.Get("/repository.yml");
                if (res && res->status == 200)
                {
                    std::string yml = res->body;
                    try
                    {
                        YAML::Node repoNode = YAML::Load(yml);
                        if (repoNode["name"] && repoNode["key"])
                        {
                            logMessage(color::cyan + "Repository Name: " + repoNode["name"].as<std::string>() + color::reset, false);
                            logMessage(color::cyan + "Repository mantainer: " + repoNode["mantainer"].as<std::string>() + color::reset);
                            logMessage(color::cyan + "Repository Key: " + repoNode["key"].as<std::string>() + color::reset, false);
                        }
                    }
                    catch (const std::exception &e)
                    {
                        return 1; // Verification failed
                    }
                    return 1; // Warning for insecure URL
                }
                else
                {
                    if (res)
                        logMessage(color::red + "Failed to verify repository: " + url + " " + std::to_string(res->status) + color::reset, true);
                    else
                        logMessage(color::red + "Failed to verify repository: " + url + " No response" + color::reset, true);
                    return -1; // Verification failed
                }
            }

            httplib::SSLClient cli(host.c_str(), std::stoi(port));
            auto res = cli.Get("/repository.yml");
            if (res && res->status == 200)
            {
                std::string yml = res->body;
                try
                {
                    YAML::Node repoNode = YAML::Load(yml);
                    if (repoNode["name"] && repoNode["key"])
                    {
                        logMessage(color::cyan + "Repository Name: " + repoNode["name"].as<std::string>() + color::reset, false);
                        logMessage(color::cyan + "Repository mantainer: " + repoNode["mantainer"].as<std::string>() + color::reset, false);
                        logMessage(color::cyan + "Repository Key: " + repoNode["key"].as<std::string>() + color::reset, false);
                        return 0; // Success
                    }
                }
                catch (const std::exception &e)
                {
                    logMessage(color::red + "Error parsing repository YAML: " + std::string(e.what()) + color::reset, true);
                    return 1; // Verification failed
                }
                logMessage(color::green + "Repository verified successfully: " + url + color::reset, false);
                return 0; // Success
            }
            else
            {
                if (res)
                    logMessage(color::red + "Failed to verify repository: " + url + " responsed with status: " + std::to_string(res->status) + color::reset, true);
                else
                    logMessage(color::red + "Failed to verify repository: " + url + " responsed with status: No response" + color::reset, true);
                return -1; // Verification failed
            }
        }
        catch (const std::exception &e)
        {
            logMessage(color::red + "Error verifying repository: " + std::string(e.what()) + color::reset, true);
            return -1; // Verification failed
        }
    }
    std::vector<std::string> getAllRepositoryUrls()
    {
        try
        {
            std::filesystem::path configPath(getRuntimeConfig().configDirectory);
            std::filesystem::path mirrorsFilePath = configPath / "mirrors.txt";
            std::vector<std::string> mirrors = openspm::getRepositoriesFromFile(mirrorsFilePath.string());
            return mirrors;
        }
        catch (const std::exception &e)
        {
            logMessage(color::red + "Error retrieving repository URLs: " + std::string(e.what()) + color::reset, true);
            return {};
        }
        return {};
    }
    int fetch()
    {
        std::vector<std::string> repos = utils::getAllRepositoryUrls();
        std::vector<PackageInfo> allPackages;
        for (const auto &repo : repos)
        {
            try
            {
                std::regex url_regex(
                    R"(^(https?)://([^/:]+)(?::(\d+))?(.*)$)");

                std::smatch match;

                if (!std::regex_match(repo, match, url_regex))
                {
                    throw std::runtime_error("Invalid URL: " + repo);
                }

                std::string scheme = match[1];
                std::string host = match[2];
                std::string port = match[3].matched ? match[3].str()
                                                    : (scheme == "https" ? "443" : "80");
                std::string path = match[4].matched && !match[4].str().empty()
                                       ? match[4].str()
                                       : "/";
                std::string response;
                if (scheme == "http")
                {
                    logMessage(color::yellow + "Warning: The repository URL uses HTTP which is not secure. Consider using HTTPS." + color::reset, true);
                    httplib::Client cli(host.c_str(), std::stoi(port));
                    logMessage(color::magenta + "GET " + (scheme + "://" + host + ":" + port + "/repository.yml") + color::reset, false);
                    auto res = cli.Get("/repository.yml");
                    if (res && res->status == 200)
                    {
                        response = res->body;
                    }
                    else
                    {
                        if (res)
                            throw std::runtime_error("Failed to fetch repository data: " + repo + " " + std::to_string(res->status));
                        else
                            throw std::runtime_error("Failed to fetch repository data: " + repo + " No response");
                    }
                }
                else
                {
                    httplib::SSLClient cli(host.c_str(), std::stoi(port));
                    logMessage(color::magenta + "GET " + (scheme + "://" + host + ":" + port + "/index.yml") + color::reset, false);
                    auto res = cli.Get("/index.yml");
                    if (res && res->status == 200)
                    {
                        response = res->body;
                    }
                    else
                    {
                        if (res)
                            throw std::runtime_error("Failed to fetch repository data: " + repo + " " + std::to_string(res->status));
                        else
                            throw std::runtime_error("Failed to fetch repository data: " + repo + " No response");
                    }
                }
                std::vector<PackageInfo> packages = deserializeRepository(response);
                allPackages.insert(allPackages.end(), packages.begin(), packages.end());
            }
            catch (const std::exception &e)
            {
                logMessage(color::red + std::string("Failed to fetch from repository: ") + repo + " Error: " + e.what() + color::reset, true);
                continue;
            }
        }
        if (allPackages.empty())
        {
            logMessage(color::yellow + std::string("No packages found in the configured repositories.") + color::reset, false);
            return 0;
        }
        logMessage(color::cyan + std::string("Fetched ") + std::to_string(allPackages.size()) + " packages" + color::reset);
        std::string serializedData = serializeRepository(allPackages);
        std::filesystem::path configPath(getRuntimeConfig().configDirectory);
        std::filesystem::path cacheFilePath = configPath / "repository_cache.yml";
        try
        {
            std::ofstream cacheFile(cacheFilePath.string());
            if (!cacheFile.is_open())
            {
                throw std::runtime_error("Failed to open cache file for writing: " + cacheFilePath.string());
            }
            cacheFile << serializedData;
            cacheFile.close();
            logMessage(color::green + std::string("Repository cache updated: ") + cacheFilePath.string() + color::reset, false);
        }
        catch (const std::exception &e)
        {
            logMessage(color::red + std::string("Error writing repository cache: ") + e.what() + color::reset, true);
            return 1;
        }
        return 0;
    }

}