/**
 * @file package_manager.cpp
 * @brief Implementation of package management operations
 *
 * Handles fetching package lists from repositories, updating the local
 * package index, and listing available packages.
 */
#include <package_manager.hpp>
#include <logger.hpp>
#ifdef _WIN32
#include <BaseTsd.h>
using ssize_t = SSIZE_T;
#endif
#include <archive.h>
#include <archive.hpp>
#include <repository_manager.hpp>
#include <config.hpp>
#include <yaml-cpp/yaml.h>
#include <httplib.h>
#include <utils.hpp>
#include <indicators/progress_bar.hpp>
#include <fstream>
#include <filesystem>
#include <archive_entry.h>
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
                warn("\033[0;33mFailed to fetch packages from repository: " + repoUrl + ". Skipping.");
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
        log("\033[0;32mFound " + std::to_string(allPackages.size()) + " packages");
        log("\033[0;36mBuilding package database...");
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
            error("\033[0;31mFailed to write to archive! Status: " + std::to_string(writeStatus));
            return 1;
        }

        debug("[DEBUG updatePackages] Write successful!");
        log("\033[0;32mSuccessfully updated packages list");
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
                        log("\033[0;36mProcessing dependency: " + depUrl);
                        debug("[DEBUG fetchPackageListFromRepository] Processing dependency: " + depUrl);
                        std::vector<PackageInfo> depPackages;
                        int depStatus = fetchPackageListFromRepository(depUrl, depPackages);
                        if (depStatus != 0)
                        {
                            warn("\033[0;33mFailed to fetch dependent repository: " + depUrl + ". Skipping.");
                            continue;
                        }
                        debug("[DEBUG fetchPackageListFromRepository] Added " + std::to_string(depPackages.size()) + " packages from dependency");
                        outPackages.insert(outPackages.end(), depPackages.begin(), depPackages.end());
                    }
                }

                const YAML::Node &packages = root["packages"];
                if (!packages || !packages.IsSequence())
                {
                    error("\033[0;31mInvalid package index format in repository: " + repoUrl);
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

    int collectDependencies(const std::string &packageName, std::vector<PackageInfo> &collectedPackages)
    {
        std::vector<PackageInfo> packages;
        int status = listPackages(packages);
        if (status != 0)
        {
            error("Failed to list packages for dependency collection.");
            return 1;
        }
        return subCollectDependencies(packageName, collectedPackages, packages);
    }
    int subCollectDependencies(const std::string &packageName, std::vector<PackageInfo> &collectedPackages, std::vector<PackageInfo> packages)
    {
        for (const auto &pkg : packages)
        {
            if (pkg.name == packageName)
            {
                debug("[DEBUG subCollectDependencies] Found package: " + pkg.name);
                bool compatible = areTagsCompatible(getConfig()->supported_tags, pkg.tags);
                if (!compatible)
                {
                    error("Package " + pkg.name + " is not compatible with the system tags.");
                    return 1;
                }
                debug("[DEBUG subCollectDependencies] Package is compatible");
                for (const auto &depName : pkg.dependencies)
                {
                    bool alreadyCollected = false;
                    for (const auto &collectedPkg : collectedPackages)
                    {
                        if (collectedPkg.name == depName)
                        {
                            alreadyCollected = true;
                            break;
                        }
                    }
                    if (!alreadyCollected)
                    {
                        debug("[DEBUG subCollectDependencies] Collecting dependency: " + depName);
                        int status = subCollectDependencies(depName, collectedPackages, packages);
                        if (status != 0)
                        {
                            return status;
                        }
                    }
                }
                collectedPackages.push_back(pkg);
                debug("[DEBUG subCollectDependencies] Added package to collected list: " + pkg.name);
                return 0;
            }
        }
        error("Package not found: " + packageName);
        return 1;
    }
    int askInstallationConfirmation(std::vector<PackageInfo> packages)
    {
        log("The following packages will be installed:");
        for (const auto &pkg : packages)
        {
            log("\033[0;34m - " + pkg.name + " v" + pkg.version);
        }
        log("\033[0;33mDo you want to proceed? (\033[0;32my\033[0;33m/\033[0;31mn\033[0;33m): ");
        char response;
        std::cin >> response;
        if (response == 'y' || response == 'Y')
        {
            return 0;
        }
        else
        {
            error("Installation cancelled by user.");
            return 1;
        }
    }
    int collectPackages(std::vector<PackageInfo> packages, std::vector<std::string> &collectedPackages)
    {
        int status;
        httplib::Client *cli = nullptr;
        httplib::SSLClient *sslCli = nullptr;
        for (const auto &targetPackage : packages)
        {
            debug("[DEBUG collectPackages] Collecting packages");
            debug("[DEBUG collectPackages] Found package: " + targetPackage.name + " v" + targetPackage.version);
            auto parsed = parse_url(targetPackage.url);
            debug("[DEBUG collectPackages] Parsed URL - scheme: " + parsed.scheme + ", host: " + parsed.host + ", path: " + parsed.path);

            bool useSSL = false;
            if (parsed.scheme == "https")
            {
                debug("[DEBUG collectPackages] Using HTTPS");
                if (parsed.port > 0)
                    sslCli = new httplib::SSLClient(parsed.host, parsed.port);
                else
                    sslCli = new httplib::SSLClient(parsed.host);
                useSSL = true;
            }
            else
            {
                debug("[DEBUG collectPackages] Using HTTP");
                if (parsed.port > 0)
                    cli = new httplib::Client(parsed.host, parsed.port);
                else
                    cli = new httplib::Client(parsed.host);
            }
            std::filesystem::path downloadPath = std::filesystem::temp_directory_path() / (targetPackage.name + ".pkg");
            if(std::filesystem::exists(downloadPath))
            {
                debug("[DEBUG collectPackages] Temporary file exists. Removing: " + downloadPath.string());
                std::filesystem::remove(downloadPath);
            }
            debug("[DEBUG collectPackages] Download path: " + downloadPath.string());
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
                logHttpRequest("GET", targetPackage.url);
                std::ofstream outFile(downloadPath, std::ios::binary);
                auto res = sslCli->Get((parsed.path).c_str(),
                                       [&](const char *data, size_t len)
                                       {
                                           outFile.write(data, len);
                                           return true;
                                       },
                                       [&](size_t current, size_t total)
                                       {
                                           if (total > 0)
                                           {
                                               bar.set_progress(static_cast<size_t>((current * 100) / total));
                                           }
                                           return true;
                                       });
                if (res && res->status == 200)
                {
                    outFile.close();
                    debug("[DEBUG collectPackages] Download successful");
                    delete sslCli;
                }
                else
                {

                    error("Failed to download package. HTTP Status: " + std::to_string(res ? res->status : 0));
                    delete sslCli;
                    return 1;
                }
            }
            else
            {
                logHttpRequest("GET", targetPackage.url);
                std::ofstream outFile(downloadPath, std::ios::binary);
                auto res = cli->Get((parsed.path).c_str(),
                                    [&](const char *data, size_t len)
                                    {
                                        outFile.write(data, len);
                                        return true;
                                    },
                                    [&](size_t current, size_t total)
                                    {
                                        if (total > 0)
                                        {
                                            bar.set_progress(static_cast<size_t>((current * 100) / total));
                                        }
                                        return true;
                                    });
                if (res && res->status == 200)
                {
                    outFile.close();
                    debug("[DEBUG collectPackages] Download successful");
                    delete cli;
                }
                else
                {

                    error("Failed to download package. HTTP Status: " + std::to_string(res ? res->status : 0));
                    delete cli;
                    return 1;
                }
            }
            collectedPackages.push_back(targetPackage.name);
        }
        return 0;
    }
    int installCollectedPackages(const std::vector<std::string> &packageNames)
    {
        log("Installing packages...");
        indicators::ProgressBar bar{
            indicators::option::BarWidth{50},
            indicators::option::Start{"["},
            indicators::option::End{"]"},
            indicators::option::PrefixText{"Installing "},
            indicators::option::ForegroundColor{indicators::Color::green},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true},
            indicators::option::MaxProgress{static_cast<size_t>(packageNames.size())}};
        for (const auto &pkgName : packageNames)
        {
            bar.set_option(indicators::option::PrefixText{"Installing " + pkgName + ": "});
            bar.print_progress();
            std::filesystem::path downloadPath = std::filesystem::temp_directory_path() / (pkgName + ".pkg");
            std::filesystem::path extractPath = std::filesystem::temp_directory_path() / "openspm" / pkgName;
            std::filesystem::create_directories(extractPath);

            debug("[DEBUG installCollectedPackages] Extracting " + downloadPath.string() + " to " + extractPath.string());

            struct archive *a = archive_read_new();
            struct archive *ext = archive_write_disk_new();
            archive_read_support_format_all(a);
            archive_read_support_filter_all(a);
            archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);

            if (archive_read_open_filename(a, downloadPath.string().c_str(), 10240) != ARCHIVE_OK)
            {
                error("Failed to open archive: " + downloadPath.string());
                archive_read_free(a);
                archive_write_free(ext);
                return 1;
            }

            struct archive_entry *entry;
            while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
            {
                std::filesystem::path fullPath = extractPath / archive_entry_pathname(entry);
                archive_entry_set_pathname(entry, fullPath.string().c_str());

                if (archive_write_header(ext, entry) != ARCHIVE_OK)
                {
                    error("Failed to write header for: " + fullPath.string());
                }
                else
                {
                    const void *buff;
                    size_t size;
                    la_int64_t offset;

                    while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK)
                    {
                        archive_write_data_block(ext, buff, size, offset);
                    }
                    archive_write_finish_entry(ext);
                }
            }

            archive_read_close(a);
            archive_read_free(a);
            archive_write_close(ext);
            archive_write_free(ext);

            debug("[DEBUG installCollectedPackages] Extraction complete for " + pkgName);
            debug("[DEBUG installCollectedPackages] Moving files to system directories");
            for (const auto &dirEntry : std::filesystem::recursive_directory_iterator(extractPath / "TARGET"))
            {
                std::filesystem::path relativePath = std::filesystem::relative(dirEntry.path(), extractPath / "TARGET");
                std::filesystem::path targetPath = getConfig()->targetDir / relativePath;

                try
                {
                    if (dirEntry.is_directory())
                    {
                        std::filesystem::create_directories(targetPath);
                    }
                    else if (dirEntry.is_regular_file())
                    {
                        std::filesystem::create_directories(targetPath.parent_path());
                        std::filesystem::copy_file(dirEntry.path(), targetPath, std::filesystem::copy_options::overwrite_existing);
                    }
                }
                catch (const std::filesystem::filesystem_error &e)
                {
                    error("Filesystem error: " + std::string(e.what()));
                    return 1;
                }
            }
            debug("Executing post-install scripts if any");
            std::filesystem::path postInstallScript = extractPath / "install.sh";
            if (std::filesystem::exists(postInstallScript) && std::filesystem::is_regular_file(postInstallScript))
            {
                debug("[DEBUG installCollectedPackages] Found post-install script for " + pkgName);

                // Get package info for environment variables
                std::vector<PackageInfo> packages;
                listPackages(packages);
                PackageInfo pkgInfo;
                for (const auto &pkg : packages)
                {
                    if (pkg.name == pkgName)
                    {
                        pkgInfo = pkg;
                        break;
                    }
                }

#ifdef _WIN32
                // On Windows, try to execute install.bat if available
                std::filesystem::path postInstallBat = extractPath / "install.bat";
                if (std::filesystem::exists(postInstallBat))
                {
                    std::string command = "set PKG_NAME=" + pkgInfo.name + " && " + 
                              "set PKG_VERSION=" + pkgInfo.version + " && " + 
                              "set PKG_MAINTAINER=" + pkgInfo.maintainer + " && " + 
                              "set PKG_DESCRIPTION=" + pkgInfo.description + " && " + 
                              "set PKG_TAGS=" + pkgInfo.tags + " && " + 
                              "set PKG_INSTALL_DIR=" + getConfig()->targetDir + " && " + 
                              "set PKG_SOURCE_DIR=" + extractPath.string() + " && " + 
                              "cmd /c \"" + postInstallBat.string() + "\" > nul";
                    
                    int ret = system(command.c_str());
                    if (ret != 0)
                    {
                        error("Post-install script failed for package: " + pkgName);
                        return 1;
                    }
                    debug("[DEBUG installCollectedPackages] Post-install script executed successfully for " + pkgName);
                }
                else
                {
                    // No post-install script available for Windows
                    debug("[DEBUG installCollectedPackages] No Windows post-install script found (install.bat) for " + pkgName);
                }
#else
                std::string command = "PKG_NAME=" + pkgInfo.name + " " + "PKG_VERSION=" + pkgInfo.version + " " + "PKG_MAINTAINER=\"" + pkgInfo.maintainer + "\" " + "PKG_DESCRIPTION=\"" + pkgInfo.description + "\" " + "PKG_TAGS=\"" + pkgInfo.tags + "\" " + "PKG_INSTALL_DIR=" + getConfig()->targetDir + " " + "PKG_SOURCE_DIR=" + extractPath.string() + " " + "sh " + postInstallScript.string() + " > /dev/null";

                int ret = system(command.c_str());
                if (ret != 0)
                {
                    error("Post-install script failed for package: " + pkgName);
                    return 1;
                }
                debug("[DEBUG installCollectedPackages] Post-install script executed successfully for " + pkgName);
#endif
            }
            else
            {
                debug("[DEBUG installCollectedPackages] No post-install script found for " + pkgName);
            }
            debug("[DEBUG installCollectedPackages] Installation complete for " + pkgName);
            bar.tick();
        }

        log("\033[0;32mAll packages installed successfully.\033[0m");
        return 0;
    }
}