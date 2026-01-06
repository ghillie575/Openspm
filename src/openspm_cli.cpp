/**
 * @file openspm_cli.cpp
 * @brief Implementation of command-line interface for OpenSPM
 * 
 * Provides user-facing commands for repository management, package listing,
 * configuration, and updates.
 */
#include <openspm_cli.hpp>
#include <repository_manager.hpp>
#include <package_manager.hpp>
#include <filesystem>
#include <iostream>
#include <logger.hpp>
#include <config.hpp>
#include <utils.hpp>
#include <thread>
namespace openspm
{
    using namespace logger;
    namespace cli
    {
        int processFlags(const std::vector<std::pair<std::string, std::string>> &flagsWithValues,
                         const std::vector<std::string> &flagsWithoutValues)
        {
            for (const auto &flagPair : flagsWithValues)
            {
                const std::string &flag = flagPair.first;
                const std::string &value = flagPair.second;
                if (flag == "--data-dir")
                {
                    Config *config = getConfig();
                    config->dataDir = value;
                }
                else if (flag == "--target-dir")
                {
                    Config *config = getConfig();
                    config->targetDir = value;
                }
                else if (flag == "--tags")
                {
                    Config *config = getConfig();
                    config->supported_tags = value;
                }
                else if (flag == "--logfile")
                {
                    Config *config = getConfig();
                    config->logsFile = value;
                }
                else
                {
                    error("Unknown flag: " + flag);
                    return 1;
                }
            }
            for (const auto &flag : flagsWithoutValues)
            {
                if (flag == "--no-color" || flag == "-nc")
                {
                    Config *config = getConfig();
                    config->colorOutput = false;
                }
                else if (flag == "--debug")
                {
                    Config *config = getConfig();
                    config->debug = true;
                }
                else
                {
                    error("Unknown flag: " + flag);
                    return 1;
                }
            }
            return 0;
        }
        int createDefaultConfig()
        {
            Config *config = getConfig();
            config->colorOutput = true;
            printVersion();
#ifndef _WIN32
            if (system("uname -a") != 0)
            {
                error("Terminal does not support system calls. Cannot proceed with configuration.");
                config->supported = false;
                config->unsupported_msg = "System calls are not supported in this terminal.";
                return 1;
            }
            if (system("gcc --version > /dev/null 2>&1") == 0)
            {
                log("Found GCC");
                config->supported_tags += "gcc;";
            }
#else
            // On Windows, check for MSVC or gcc
            if (system("gcc --version > nul 2>&1") == 0)
            {
                log("Found GCC");
                config->supported_tags += "gcc;";
            }
            else if (system("cl > nul 2>&1") == 0)
            {
                log("Found MSVC");
                config->supported_tags += "msvc;";
            }
#endif
            config->dataDir = ".spm";
            config->targetDir = ".spm";
            config->supported = true;
            config->colorOutput = false;
#ifdef _WIN32
            std::filesystem::path configPath("C:\\ProgramData\\openspm\\config.yaml");
#else
            std::filesystem::path configPath("/etc/openspm/config.yaml");
#endif
            saveConfig(configPath.string(), *config);
            return 0;
        }
        int configure()
        {
            log("Starting configuration...");
            Config *config = getConfig();
            config->colorOutput = true;
            printVersion();
#ifndef _WIN32
            if (system("uname -a") != 0)
            {
                error("Terminal does not support system calls. Cannot proceed with configuration.");
                config->supported = false;
                config->unsupported_msg = "System calls are not supported in this terminal.";
                return 1;
            }
            if (system("gcc --version > /dev/null 2>&1") == 0)
            {
                log("Found GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__));
                config->supported_tags += "gcc;";
                config->supported_tags += "gcc-" + std::to_string(__GNUC__) + ";";
                config->supported_tags += "gcc-" + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__) + ";";
                config->supported_tags += "non-bin;";
            }
            log("Please follow the prompts to configure OpenSPM.");
            log("Enter the data directory (default: /etc/openspm/): ");
            std::string dataDir;
            std::getline(std::cin, dataDir);
            if (dataDir.empty())
            {
                dataDir = "/etc/openspm/";
            }
            log("Enter the target installation directory (default: /usr/local/): ");
            std::string targetDir;
            std::getline(std::cin, targetDir);
            if (targetDir.empty())
            {
                targetDir = "/usr/local/";
            }
#else
            // On Windows, check for MSVC or gcc
            if (system("gcc --version > nul 2>&1") == 0)
            {
                log("Found GCC");
                config->supported_tags += "gcc;";
                config->supported_tags += "non-bin;";
            }
            else if (system("cl > nul 2>&1") == 0)
            {
                log("Found MSVC");
                config->supported_tags += "msvc;";
                config->supported_tags += "non-bin;";
            }
            log("Please follow the prompts to configure OpenSPM.");
            log("Enter the data directory (default: C:\\ProgramData\\openspm\\): ");
            std::string dataDir;
            std::getline(std::cin, dataDir);
            if (dataDir.empty())
            {
                dataDir = "C:\\ProgramData\\openspm\\";
            }
            log("Enter the target installation directory (default: C:\\Program Files\\openspm\\): ");
            std::string targetDir;
            std::getline(std::cin, targetDir);
            if (targetDir.empty())
            {
                targetDir = "C:\\Program Files\\openspm\\";
            }
#endif
            config->dataDir = dataDir;
            config->targetDir = targetDir;
            log("Do you see colored text below? \n \e[32mThis is green text.\e[0m \n \e[31mThis is red text.\e[0m \n (y/n):");
            std::string colorTestResponse;
            std::getline(std::cin, colorTestResponse);
            if (colorTestResponse == "n" || colorTestResponse == "N")
            {
                config->colorOutput = false;
            }
            else
            {
                log("Do you want to enable colored output? (y/n, default: y): ");
                std::string colorOutputStr;
                std::getline(std::cin, colorOutputStr);
                if (colorOutputStr.empty() || colorOutputStr == "y" || colorOutputStr == "Y")
                {
                    config->colorOutput = true;
                }
                else
                {
                    config->colorOutput = false;
                }
            }
#ifdef _WIN32
            std::filesystem::path configPath("C:\\ProgramData\\openspm\\config.yaml");
#else
            std::filesystem::path configPath("/etc/openspm/config.yaml");
#endif
            if (!std::filesystem::exists(configPath.parent_path()))
            {
                try
                {
                    std::filesystem::create_directories(configPath.parent_path());
                }
                catch (const std::exception &e)
                {
                    error("Failed to create directories for config file: " + std::string(e.what()));
                    return 1;
                }
            }
            saveConfig(configPath.string(), *config);
            log("Configuration completed.");
            return 0;
        }
        int installPackage(const std::string &packageName){
            std::vector<std::string> collectedPackages;
            std::vector<openspm::PackageInfo> packages;
            int status = openspm::collectDependencies(packageName, packages);
            if(status !=0){
                return status;
            }
            status = openspm::askInstallationConfirmation(packages);
            if(status !=0){
                return status;
            }
            status = openspm::collectPackages(packages, collectedPackages);
            if(status !=0){
                return status;
            }
            status = openspm::installCollectedPackages(collectedPackages);
            return status;
        }
        int processCommandLine(std::string command,
                               const std::vector<std::string> &commandArgs,
                               const std::vector<std::pair<std::string, std::string>> &flagsWithValues,
                               const std::vector<std::string> &flagsWithoutValues)
        {
            try
            {

                if (command == "configure" || command == "config" || command == "cfg")
                {
                    return configure();
                }
                else if (command == "version" || command == "--version" || command == "-v")
                {
                    printVersion();
                    return 0;
                }
#ifdef _WIN32
                loadConfig("C:\\ProgramData\\openspm\\config.yaml");
#else
                loadConfig("/etc/openspm/config.yaml");
#endif
                Config *config = getConfig();
                int status = processFlags(flagsWithValues, flagsWithoutValues);
                if (status != 0)
                {
                    return 1;
                }
                status = initDataArchive();

                if (status != 0)
                {
                    error("Failed to initialize data archive.");
                    return 1;
                }
                if (!config->supported)
                {
                    error("This platform is not officially supported: " + config->unsupported_msg);
                    return 1;
                }
                initFileLogging();
                if (processFlags(flagsWithValues, flagsWithoutValues) != 0)
                {
                    return 1;
                }
                if (command == "add-repo" || command == "add-repository" || command == "ar")
                {
                    if (commandArgs.size() < 1)
                    {
                        error("Repository URL is required.");
                        return 1;
                    }
                    std::string repoUrl = commandArgs[0];
                    return addRepository(repoUrl, false);
                }
                else if (command == "rm-repo" || command == "remove-repository" || command == "rr")
                {
                    if (commandArgs.size() < 1)
                    {
                        error("Repository URL is required.");
                        return 1;
                    }
                    std::string repoUrl = commandArgs[0];
                    RepositoryInfo repoInfo;
                    repoInfo.url = repoUrl;
                    bool result = removeRepository(repoInfo);
                    if (!result)
                    {
                        error("\033[0;31mFailed to remove repository: " + repoUrl);
                        return 1;
                    }
                    log("\033[0;32mSuccessfully removed repository: " + repoUrl);
                }
                else if (command == "list-repos" || command == "list-repositories" || command == "lr")
                {
                    std::vector<std::string> repoList = getRepositoryList();
                    if (repoList.empty())
                    {
                        log("No repositories found.");
                    }
                    else
                    {
                        log("Configured Repositories:");
                        for (const auto &repoUrl : repoList)
                        {
                            log("  \033[0;34m" + repoUrl);
                        }
                    }
                }
                else if (command == "update-repos" || command == "update-repositories" || command == "ur")
                {
                    return updateRepositories();
                }
                else if (command == "update" || command == "up")
                {
                    return updateAll();
                }
                else if (command == "install" || command == "i")
                {
                    if (commandArgs.size() < 1)
                    {
                        error("Package name is required.");
                        return 1;
                    }
                    std::string packageName = commandArgs[0];
                    return installPackage(packageName);
                }
                else if (command == "list-packages" || command == "lp")
                {
                    listPackages();
                }
                else if (command == "help" || command == "--help" || command == "-h")
                {
                    log("\033[0;32mOpenSPM - Open Source Package Manager\033[0m");
                    log("\033[0;32mUsage: openspm <command> [args] [flags]\033[0m");
                    log("");
                    log("\033[0;32mCommands:");
                    log("  \033[0;34mconfigure                 \033[0;35mStart interactive configuration");
                    log("  \033[0;34mversion, -v               \033[0;35mShow version information");
                    log("  \033[0;34mhelp, -h                  \033[0;35mShow this help message");
                    log("");
                    log("\033[0;32mRepository Management:");
                    log("  \033[0;34madd-repo \033[0;37m<url>            \033[0;35mAdd a new package repository");
                    log("  \033[0;34mrm-repo \033[0;37m<url>             \033[0;35mRemove a package repository");
                    log("  \033[0;34mlist-repos                \033[0;35mList all configured repositories");
                    log("  \033[0;34mupdate-repos              \033[0;35mSync repository metadata");
                    log("");
                    log("\033[0;32mPackage Management:");
                    log("  \033[0;34mlist-packages, lp         \033[0;35mList packages compatible with this system");
                    log("  \033[0;34mupdate, up                \033[0;35mUpdate all installed packages");
                    log("");
                    log("\033[0;32mGlobal Flags:");
                    log("  \033[0;34m--logfile \033[0;37m<file>          \033[0;35mPath to save log output");
                    log("  \033[0;34m--data-dir \033[0;37m<dir>          \033[0;35mSet custom metadata directory");
                    log("  \033[0;34m--target-dir \033[0;37m<dir>        \033[0;35mSet custom installation target");
                    log("  \033[0;34m--tags \033[0;37m<tags>             \033[0;35mOverride system tags (e.g. \"gcc;bin\")");
                    log("  \033[0;34m--no-color, -nc           \033[0;35mDisable colored output");
                    log("  \033[0;34m--debug                   \033[0;35mShow verbose debugging information");
                }
                else
                {
                    error("\033[0;31mUnknown command: " + command);
                    return 1;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error("\033[0;31mError: " + std::string(e.what()));
                return 1;
            }
        }
        int updateAll()
        {
            int status = openspm::updateAllRepositories();
            if (status != 0)
            {
                error("\033[0;31mFailed to update repositories.");
                return 1;
            }
            status = openspm::updatePackages();
            if (status != 0)
            {
                return 1;
            }
            log(CLR_GREEN + std::string("Updated repository and package indexes"));
            return 0;
        }

        int addRepository(const std::string &repoUrl, bool skipUpdate, bool interactive)
        {

            RepositoryInfo repoInfo;
            bool status = getRepositoryInfo(repoUrl, repoInfo);
            if (!status)
            {
                error("\033[0;31mFailed to get repository info from URL: " + repoUrl);
                return 1;
            }
            log("\033[0;32mRepository Info:");
            log("  \033[0;34mName: \033[0;37m" + repoInfo.name);
            log("  \033[0;34mDescription: \033[0;37m" + repoInfo.description);
            log("  \033[0;34mMantainer: \033[0;37m" + repoInfo.mantainer);
            if (interactive)
            {
                log("Are you sure you want to add this repository? (y/n): ");
                std::string response;
                std::getline(std::cin, response);
                if (response != "y" && response != "Y")
                {
                    return 0;
                }
            }
            status = openspm::addRepository(repoInfo);
            if (!status)
            {
                error("\033[0;31mFailed to add repository: " + repoUrl);
                return 1;
            }
            if (!skipUpdate)
            {
                log("\033[0;36mUpdating...");
                return updatePackages();
            }
            log("\033[0;32mSuccessfully added repository: " + repoUrl);
            return 0;
        }
        int updatePackages()
        {
            return openspm::updatePackages();
        }
        int updateRepositories()
        {
            int status = updateAllRepositories();
            return status;
        }

        int listPackages()
        {
            std::vector<PackageInfo> packages;
            int status = listPackages(packages);
            if (status != 0)
            {
                error("\033[0;31mFailed to get packages list");
                return status;
            }
            std::string tags = getConfig()->supported_tags;
            log("\033[0;32mCompatible packages:");
            log("\033[0;32m────────────────────────────────────────────");
            for (auto &package : packages)
            {
                bool result = areTagsCompatible(tags, package.tags);
                if (result)
                {

                    log("  \033[0;36mName:        \033[0;33m" + package.name);
                    log("  \033[0;36mVersion:     \033[0;35m" + package.version);

                    if (!package.description.empty())
                        log("  \033[0;36mDescription: \033[0;33m" + package.description);
                    else
                        log("  \033[0;36mDescription: \033[0;31m<none>");

                    if (!package.maintainer.empty())
                        log("  \033[0;36mMaintainer:  \033[0;33m" + package.maintainer);
                    else
                        log("  \033[0;36mMaintainer:  \033[0;31m<unknown>");

                    if (!package.tags.empty())
                        log("  \033[0;36mTags:        \033[0;34m" + package.tags);
                    else
                        log("  \033[0;36mTags:        \033[0;31m<none>");
                    if (package.dependencies.size() != 0)
                    {
                        log("  \033[0;36mDependencies:");
                        for (auto dep : package.dependencies)
                        {
                            log("   \033[0;31m" + dep);
                        }
                        
                    }
                    if (!package.url.empty())
                        log("  \033[0;36mURL:         \033[0;34m" + package.url);
                    else
                        log("  \033[0;36mURL:         \033[0;31m<none>");

                    log("\033[0;32m────────────────────────────────────────────\033[0m");
                }
            }
            return 0;
        }
    } // namespace cli
} // namespace openspm