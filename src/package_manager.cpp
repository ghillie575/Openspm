/**
 * @file package_manager.cpp
 * @brief Implementation of package management operations
 * 
 * Handles fetching package lists from repositories, updating the local
 * package index, and listing available packages.
 */
#include <package_manager.hpp>
#include <logger.hpp>
#include <archive.hpp>
#include <repository_manager.hpp>
#include <config.hpp>
#include <yaml-cpp/yaml.h>
#include <httplib.h>
#include <utils.hpp>
#include <indicators/progress_bar.hpp>
#include <fstream>
#include <filesystem>
namespace openspm
{
    using namespace logger;
    int updatePackages()
    {
        debug("[DEBUG updatePackages] Starting package update");
        Archive *dataArchive = getDataArchive();
        debug("[DEBUG updatePackages] Archive pointer: " + std::to_string((long)dataArchive));

        std::vector<std::string> repoList = getRepositoryList();
        debug("[DEBUG updatePackages] Found " + std::to_string(repoList.size()) + " repositories");

        if (repoList.empty())
        {
            warn("No repositories found. Cannot update packages.");
            return 1;
        }

        std::unordered_map<std::string, PackageInfo> packageMap;
        std::vector<PackageInfo> allPackages;

        size_t repoIndex = 0;
        for (const auto &repoUrl : repoList)
        {
            debug("[DEBUG updatePackages] Repository: " + repoUrl);

            RepositoryInfo repoInfo;
            bool infoStatus = getRepositoryInfo(repoUrl, repoInfo);
            if (!infoStatus)
            {
                error("Failed to get repository info: " + repoUrl);
                continue;
            }
            debug("[DEBUG updatePackages] Repository info retrieved: " + repoInfo.name);

            std::vector<PackageInfo> repoPackages;
            int fetchStatus = fetchPackageListFromRepository(repoUrl, repoPackages);
            if (fetchStatus != 0)
            {
                warn("\033[1;33mFailed to fetch packages from repository: " + repoUrl + ". Skipping.");
                continue;
            }

            debug("[DEBUG updatePackages] Fetched " + std::to_string(repoPackages.size()) + " packages from this repository");

            for (const auto &pkg : repoPackages)
            {
                debug("[DEBUG updatePackages] Adding package to map: " + pkg.name);
                packageMap[pkg.name] = pkg;
            }
            repoIndex++;
        }

        debug("[DEBUG updatePackages] Total unique packages in map: " + std::to_string(packageMap.size()));

        for (const auto &[_, pkg] : packageMap)
        {
            debug("[DEBUG updatePackages] Package in final list: " + pkg.name);
            allPackages.push_back(pkg);
        }
        log("\033[1;32mFound " + std::to_string(allPackages.size()) + " packages");
        log("\033[1;36mBuilding package database...");
        debug("[DEBUG updatePackages] Building YAML...");
        YAML::Emitter out;

        out << YAML::BeginMap;
        out << YAML::Key << "packages";
        out << YAML::Value << YAML::BeginSeq;

        for (const auto &pkg : allPackages)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "name" << YAML::Value << pkg.name;
            out << YAML::Key << "version" << YAML::Value << pkg.version;
            out << YAML::Key << "description" << YAML::Value << pkg.description;
            out << YAML::Key << "maintainer" << YAML::Value << pkg.maintainer;

            out << YAML::Key << "dependencies" << YAML::Value << YAML::BeginSeq;
            for (const auto &dep : pkg.dependencies)
            {
                out << dep;
            }
            out << YAML::EndSeq;

            out << YAML::Key << "tags" << YAML::Value << pkg.tags;
            out << YAML::Key << "url" << YAML::Value << pkg.url;
            out << YAML::EndMap;
        }

        out << YAML::EndSeq;
        out << YAML::EndMap;
        std::string data = out.c_str();

        debug("[DEBUG updatePackages] YAML data length: " + std::to_string(data.length()) + " bytes");
        debug("[DEBUG updatePackages] Writing to archive...");

        int writeStatus = dataArchive->writeFile("packages.yaml", data);
        if (writeStatus != 0)
        {
            error("\033[1;31mFailed to write to archive! Status: " + std::to_string(writeStatus));
            return 1;
        }

        debug("[DEBUG updatePackages] Write successful!");
        log("\033[1;32mSuccessfully updated packages list");
        return 0;
    }

    int fetchPackageListFromRepository(const std::string &repoUrl, std::vector<PackageInfo> &outPackages)
    {
        debug("[DEBUG fetchPackageListFromRepository] Fetching from: " + repoUrl);
        auto parsed = parse_url(repoUrl);
        debug("[DEBUG fetchPackageListFromRepository] Parsed URL - scheme: " + parsed.scheme + ", host: " + parsed.host + ", path: " + parsed.path);

        httplib::Client *cli = nullptr;
        httplib::SSLClient *sslCli = nullptr;
        bool useSSL = false;
        if (parsed.scheme == "https")
        {
            debug("[DEBUG fetchPackageListFromRepository] Using HTTPS");
            if (parsed.port > 0)
                sslCli = new httplib::SSLClient(parsed.host, parsed.port);
            else
                sslCli = new httplib::SSLClient(parsed.host);
            useSSL = true;
        }
        else
        {
            debug("[DEBUG fetchPackageListFromRepository] Using HTTP");
            if (parsed.port > 0)
                cli = new httplib::Client(parsed.host, parsed.port);
            else
                cli = new httplib::Client(parsed.host);
        }

        std::string fullUrl = parsed.scheme + "://" + parsed.host;
        if (parsed.port > 0)
        {
            fullUrl += ":" + std::to_string(parsed.port);
        }
        fullUrl += parsed.path + "/pkg-list.yaml";

        if (useSSL)
        {
            debug("[DEBUG fetchPackageListFromRepository] Making HTTPS request");
            auto res = sslCli->Get((parsed.path + "/pkg-list.yaml").c_str());
            if (res && res->status == 200)
            {
                logHttpRequest("GET", fullUrl, res->status);
                debug("[DEBUG fetchPackageListFromRepository] Request successful, response size: " + std::to_string(res->body.size()) + " bytes");

                debug("[DEBUG fetchPackageListFromRepository] Parsing YAML");
                YAML::Node root = YAML::Load(res->body);

                const YAML::Node &dependNode = root["depend"];
                if (dependNode && dependNode.IsSequence())
                {
                    debug("[DEBUG fetchPackageListFromRepository] Found " + std::to_string(dependNode.size()) + " dependencies");
                    for (const auto &dep : dependNode)
                    {
                        std::string depUrl = dep.as<std::string>();
                        log("\033[1;36mProcessing dependency: " + depUrl);
                        debug("[DEBUG fetchPackageListFromRepository] Processing dependency: " + depUrl);
                        std::vector<PackageInfo> depPackages;
                        int depStatus = fetchPackageListFromRepository(depUrl, depPackages);
                        if (depStatus != 0)
                        {
                            warn("\033[1;33mFailed to fetch dependent repository: " + depUrl + ". Skipping.");
                            continue;
                        }
                        debug("[DEBUG fetchPackageListFromRepository] Added " + std::to_string(depPackages.size()) + " packages from dependency");
                        outPackages.insert(outPackages.end(), depPackages.begin(), depPackages.end());
                    }
                }

                const YAML::Node &packages = root["packages"];
                if (!packages || !packages.IsSequence())
                {
                    error("\033[1;31mInvalid package index format in repository: " + repoUrl);
                    delete sslCli;
                    return 1;
                }

                debug("[DEBUG fetchPackageListFromRepository] Found " + std::to_string(packages.size()) + " packages");
                for (const auto &node : packages)
                {
                    PackageInfo pkg;
                    pkg.name = node["name"] ? node["name"].as<std::string>() : "";
                    pkg.version = node["version"] ? node["version"].as<std::string>() : "";
                    pkg.description = node["description"] ? node["description"].as<std::string>() : "";
                    pkg.maintainer = node["maintainer"] ? node["maintainer"].as<std::string>() : "";

                    if (node["dependencies"] && node["dependencies"].IsSequence())
                    {
                        for (const auto &depNode : node["dependencies"])
                        {
                            pkg.dependencies.push_back(depNode.as<std::string>());
                        }
                    }

                    pkg.tags = node["tags"] ? node["tags"].as<std::string>() : "";
                    pkg.url = node["url"] ? node["url"].as<std::string>() : "";

                    debug("[DEBUG fetchPackageListFromRepository] Package: " + pkg.name + " v" + pkg.version);
                    outPackages.push_back(std::move(pkg));
                }
                delete sslCli;
                debug("[DEBUG fetchPackageListFromRepository] Successfully fetched " + std::to_string(outPackages.size()) + " total packages");
                return 0;
            }
            else
            {
                logHttpRequest("GET", fullUrl, res ? res->status : 0);
                debug("[DEBUG fetchPackageListFromRepository] Request failed");
                delete sslCli;
                return 1;
            }
        }
        else
        {
            debug("[DEBUG fetchPackageListFromRepository] Making HTTP request");
            auto res = cli->Get((parsed.path + "/pkg-list.yaml").c_str());
            if (res && res->status == 200)
            {
                logHttpRequest("GET", fullUrl, res->status);
                debug("[DEBUG fetchPackageListFromRepository] Request successful, response size: " + std::to_string(res->body.size()) + " bytes");

                debug("[DEBUG fetchPackageListFromRepository] Parsing YAML");
                YAML::Node root = YAML::Load(res->body);

                const YAML::Node &dependNode = root["depend"];
                if (dependNode && dependNode.IsSequence())
                {
                    debug("[DEBUG fetchPackageListFromRepository] Found " + std::to_string(dependNode.size()) + " dependencies");
                    for (const auto &dep : dependNode)
                    {
                        std::string depUrl = dep.as<std::string>();
                        log("Processing dependency: " + depUrl);
                        debug("[DEBUG fetchPackageListFromRepository] Processing dependency: " + depUrl);
                        std::vector<PackageInfo> depPackages;
                        int depStatus = fetchPackageListFromRepository(depUrl, depPackages);
                        if (depStatus != 0)
                        {
                            warn("Failed to fetch dependent repository: " + depUrl + ". Skipping.");
                            continue;
                        }
                        debug("[DEBUG fetchPackageListFromRepository] Added " + std::to_string(depPackages.size()) + " packages from dependency");
                        outPackages.insert(outPackages.end(), depPackages.begin(), depPackages.end());
                    }
                }

                const YAML::Node &packages = root["packages"];
                if (!packages || !packages.IsSequence())
                {
                    error("Invalid package index format in repository: " + repoUrl);
                    delete cli;
                    return 1;
                }

                debug("[DEBUG fetchPackageListFromRepository] Found " + std::to_string(packages.size()) + " packages");
                for (const auto &node : packages)
                {
                    PackageInfo pkg;
                    pkg.name = node["name"] ? node["name"].as<std::string>() : "";
                    pkg.version = node["version"] ? node["version"].as<std::string>() : "";
                    pkg.description = node["description"] ? node["description"].as<std::string>() : "";
                    pkg.maintainer = node["maintainer"] ? node["maintainer"].as<std::string>() : "";

                    if (node["dependencies"] && node["dependencies"].IsSequence())
                    {
                        for (const auto &depNode : node["dependencies"])
                        {
                            pkg.dependencies.push_back(depNode.as<std::string>());
                        }
                    }

                    pkg.tags = node["tags"] ? node["tags"].as<std::string>() : "";
                    pkg.url = node["url"] ? node["url"].as<std::string>() : "";

                    debug("[DEBUG fetchPackageListFromRepository] Package: " + pkg.name + " v" + pkg.version);
                    outPackages.push_back(std::move(pkg));
                }
                delete cli;
                debug("[DEBUG fetchPackageListFromRepository] Successfully fetched " + std::to_string(outPackages.size()) + " total packages");
                return 0;
            }
            else
            {
                logHttpRequest("GET", fullUrl, res ? res->status : 0);
                debug("[DEBUG fetchPackageListFromRepository] Request failed");
                delete cli;
                return 1;
            }
        }
    }

    int listPackages(std::vector<PackageInfo> &outPackages)
    {
        debug("[DEBUG listPackages] Starting package list");
        Archive *dataArchive = getDataArchive();
        debug("[DEBUG listPackages] Getting archive pointer: " + std::to_string((long)dataArchive));

        std::string packagesFileContent;
        debug("[DEBUG listPackages] Reading packages.yaml from archive...");
        int status = dataArchive->readFile("packages.yaml", packagesFileContent);
        if (status != 0)
        {
            error("Failed to read installed packages list.");
            return 1;
        }

        debug("[DEBUG listPackages] Read " + std::to_string(packagesFileContent.length()) + " bytes");
        debug("[DEBUG listPackages] Parsing YAML");

        YAML::Node root = YAML::Load(packagesFileContent);
        const YAML::Node &packages = root["packages"];
        if (!packages || !packages.IsSequence())
        {
            error("Invalid installed packages list format.");
            return 1;
        }

        debug("[DEBUG listPackages] Found " + std::to_string(packages.size()) + " packages in YAML");

        for (const auto &node : packages)
        {
            PackageInfo pkg;
            pkg.name = node["name"] ? node["name"].as<std::string>() : "";
            pkg.version = node["version"] ? node["version"].as<std::string>() : "";
            pkg.description = node["description"] ? node["description"].as<std::string>() : "";
            pkg.maintainer = node["maintainer"] ? node["maintainer"].as<std::string>() : "";

            if (node["dependencies"] && node["dependencies"].IsSequence())
            {
                for (const auto &depNode : node["dependencies"])
                {
                    pkg.dependencies.push_back(depNode.as<std::string>());
                }
            }

            pkg.tags = node["tags"] ? node["tags"].as<std::string>() : "";
            pkg.url = node["url"] ? node["url"].as<std::string>() : "";

            debug("[DEBUG listPackages] Package: " + pkg.name + " v" + pkg.version);
            outPackages.push_back(std::move(pkg));
        }

        debug("[DEBUG listPackages] Total packages added to output: " + std::to_string(outPackages.size()));
        return 0;
    }
    int installPackage(const std::string &packageName)
    {
        int status;
        std::vector<PackageInfo> packages;
        debug("[DEBUG installPackage] Installing package: " + packageName);
        status = listPackages(packages);
        if (status != 0)
        {
            error("Failed to get installed packages list.");
            return status;
        }
        PackageInfo targetPackage;
        for (const auto &pkg : packages)
        {
            if (pkg.name == packageName)
            {
                targetPackage = pkg;
                break;
            }
        }
        if (targetPackage.name.empty())
        {
            error("Package not found: " + packageName);
            return 1;
        }
        debug("[DEBUG installPackage] Found package: " + targetPackage.name + " v" + targetPackage.version);
        auto parsed = parse_url(targetPackage.url);
        debug("[DEBUG installPackage] Parsed URL - scheme: " + parsed.scheme + ", host: " + parsed.host + ", path: " + parsed.path);
        httplib::Client *cli = nullptr;
        httplib::SSLClient *sslCli = nullptr;
        bool useSSL = false;
        if (parsed.scheme == "https")
        {
            debug("[DEBUG installPackage] Using HTTPS");
            if (parsed.port > 0)
                sslCli = new httplib::SSLClient(parsed.host, parsed.port);
            else
                sslCli = new httplib::SSLClient(parsed.host);
            useSSL = true;
        }
        else
        {
            debug("[DEBUG installPackage] Using HTTP");
            if (parsed.port > 0)
                cli = new httplib::Client(parsed.host, parsed.port);
            else
                cli = new httplib::Client(parsed.host);
        }
        std::filesystem::path downloadPath = std::filesystem::temp_directory_path() / (targetPackage.name + ".pkg");
        debug("[DEBUG installPackage] Download path: " + downloadPath.string());
        indicators::ProgressBar bar{
            indicators::option::BarWidth{50},
            indicators::option::Start{"["},
            indicators::option::End{"]"},
            indicators::option::PrefixText{"Downloading " + targetPackage.name + ": "},
            indicators::option::ForegroundColor{indicators::Color::cyan},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true},
            indicators::option::MaxProgress{100}};
        if (useSSL)
        {
            std::ofstream outFile(downloadPath, std::ios::binary);
            sslCli->Get((parsed.path).c_str(),
                        [&](size_t len, size_t total)
                        {
                            if (total > 0)
                            {
                                size_t progress = (len * 100) / total;
                                bar.set_progress(progress);
                            }
                            return true;
                        });
        }
        log("\033[1;32mSuccessfully installed package: " + targetPackage.name);
        return 0;
    }
}