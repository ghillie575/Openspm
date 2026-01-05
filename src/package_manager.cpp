/**
 * @file package_manager.cpp
 * @brief Implementation of package management functionality
 */
#include <package_manager.hpp>
#include <logger.hpp>
#include <archive.hpp>
#include <repository_manager.hpp>
#include <config.hpp>
#include <yaml-cpp/yaml.h>
#include <httplib.h>
#include <utils.hpp>
namespace openspm
{
    using namespace logger;
    int updatePackages()
    {

        std::vector<std::string> repoList = getRepositoryList();
        if (repoList.empty())
        {
            warn("\033[1;33mNo repositories found. Cannot update packages.");
            return 1;
        }
        std::vector<PackageInfo> allPackages;
        for (const auto &repoUrl : repoList)
        {

            RepositoryInfo repoInfo;
            bool infoStatus = getRepositoryInfo(repoUrl, repoInfo);
            if (!infoStatus)
            {
                error("\033[1;31mFailed to get repository info: " + repoUrl);
                continue;
            }

            std::vector<PackageInfo> repoPackages;
            int fetchStatus = fetchPackageListFromRepository(repoUrl, repoPackages);
            if (fetchStatus != 0)
            {
                warn("\033[1;33mFailed to fetch packages from repository: " + repoUrl + ". Skipping.");
                continue;
            }

            allPackages.insert(allPackages.end(), repoPackages.begin(), repoPackages.end());
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
                out << YAML::Key << "tags" << YAML::Value << pkg.tags;
                out << YAML::Key << "url" << YAML::Value << pkg.url;

                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
            out << YAML::EndMap;
            Archive *dataArchive = getDataArchive();
            std::string outData = out.c_str();
            dataArchive->writeFile("packages.yaml", outData);
        }
        log("\033[1;32mSuccessfully updated packages list.");
        return 0;
    }
    int fetchPackageListFromRepository(const std::string &repoUrl, std::vector<PackageInfo> &outPackages)
    {
        auto parsed = parse_url(repoUrl);

        httplib::Client *cli = nullptr;
        httplib::SSLClient *sslCli = nullptr;
        bool useSSL = false;
        if (parsed.scheme == "https")
        {
            if (parsed.port > 0)
                sslCli = new httplib::SSLClient(parsed.host, parsed.port);
            else
                sslCli = new httplib::SSLClient(parsed.host);
            useSSL = true;
        }
        else
        {
            if (parsed.port > 0)
                cli = new httplib::Client(parsed.host, parsed.port);
            else
                cli = new httplib::Client(parsed.host);
        }

        if (getConfig()->colorOutput)
        {
            std::cout << "\033[1;36mGET \033[1;35m" << parsed.scheme << "://" << parsed.host << (parsed.port > 0 ? (":" + std::to_string(parsed.port)) : "") << parsed.path << "/pkg-list.yaml ";
        }
        else
        {
            std::cout << "GET " << parsed.scheme << "://" << parsed.host << (parsed.port > 0 ? (":" + std::to_string(parsed.port)) : "") << parsed.path << "/pkg-list.yaml ";
        }
        if (useSSL)
        {
            auto res = sslCli->Get((parsed.path + "/pkg-list.yaml").c_str());
            if (res && res->status == 200)
            {

                std::cout << "\033[1;32m" << res->status << "\033[0m" << std::endl;

                YAML::Node root = YAML::Load(res->body);
                const YAML::Node &dependNode = root["depend"];
                if (dependNode && dependNode.IsSequence())
                {
                    for (const auto &dep : dependNode)
                    {
                        std::vector<PackageInfo> depPackages;
                        int depStatus = fetchPackageListFromRepository(dep.as<std::string>(), depPackages);
                        if (depStatus != 0)
                        {
                            warn("\033[1;33mFailed to fetch dependent repository: " + dep.as<std::string>() + ". Skipping.");
                            continue;
                        }
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
                for (const auto &node : packages)
                {
                    PackageInfo pkg;

                    pkg.name = node["name"] ? node["name"].as<std::string>() : "";
                    pkg.version = node["version"] ? node["version"].as<std::string>() : "";
                    pkg.description = node["description"] ? node["description"].as<std::string>() : "";
                    pkg.maintainer = node["maintainer"] ? node["maintainer"].as<std::string>() : "";
                    pkg.tags = node["tags"] ? node["tags"].as<std::string>() : "";
                    pkg.url = node["url"] ? node["url"].as<std::string>() : "";

                    outPackages.push_back(std::move(pkg));
                }
                delete sslCli;
                return 0;
            }
            else
            {

                std::cout << "\033[1;31m" << (res ? std::to_string(res->status) : "no response") << "\033[0m" << std::endl;
                delete sslCli;
                return 1;
            }
        }
        else
        {

            auto res = cli->Get((parsed.path + "/pkg-list.yaml").c_str());
            if (res && res->status == 200)
            {

                std::cout << "\033[1;32m" << res->status << "\033[0m" << std::endl;

                YAML::Node root = YAML::Load(res->body);

                const YAML::Node &packages = root["packages"];
                if (!packages || !packages.IsSequence())
                {
                    error("\033[1;31mInvalid package index format in repository: " + repoUrl);
                    delete sslCli;
                    return 1;
                }
                for (const auto &node : packages)
                {
                    PackageInfo pkg;

                    pkg.name = node["name"] ? node["name"].as<std::string>() : "";
                    pkg.version = node["version"] ? node["version"].as<std::string>() : "";
                    pkg.description = node["description"] ? node["description"].as<std::string>() : "";
                    pkg.maintainer = node["maintainer"] ? node["maintainer"].as<std::string>() : "";
                    pkg.tags = node["tags"] ? node["tags"].as<std::string>() : "";
                    pkg.url = node["url"] ? node["url"].as<std::string>() : "";

                    outPackages.push_back(std::move(pkg));
                }

                delete cli;
                return 0;
            }
            else
            {

                std::cout << "\033[1;31m" << (res ? std::to_string(res->status) : "no response") << "\033[0m" << std::endl;
                delete cli;
                return 1;
            }
        }
    }
    int listInstalledPackages(std::vector<PackageInfo> &outPackages)
    {
        Archive *dataArchive = getDataArchive();
        std::string packagesFileContent;
        int status = dataArchive->readFile("packages.yaml", packagesFileContent);
        if (status != 0)
        {
            error("\033[1;31mFailed to read installed packages list.");
            return 1;
        }
        YAML::Node root = YAML::Load(packagesFileContent);
        const YAML::Node &packages = root["packages"];
        if (!packages || !packages.IsSequence())
        {
            error("\033[1;31mInvalid installed packages list format.");
            return 1;
        }
        for (const auto &node : packages)
        {
            PackageInfo pkg;
            pkg.name = node["name"] ? node["name"].as<std::string>() : "";
            pkg.version = node["version"] ? node["version"].as<std::string>() : "";
            pkg.description = node["description"] ? node["description"].as<std::string>() : "";
            pkg.maintainer = node["maintainer"] ? node["maintainer"].as<std::string>() : "";
            pkg.tags = node["tags"] ? node["tags"].as<std::string>() : "";
            pkg.url = node["url"] ? node["url"].as<std::string>() : "";
            outPackages.push_back(std::move(pkg));
        }
        return 0;
    }
}