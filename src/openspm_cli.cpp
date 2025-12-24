#include <openspm_cli.hpp>
#include <repository_manager.hpp>
#include <package_manager.hpp>
#include <filesystem>
#include <iostream>
#include <logger.hpp>
#include <config.hpp>
#include <utils.hpp>
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
                else
                {
                    error("Unknown flag: " + flag);
                    return 1;
                }
                return 0;
            }
            for (const auto &flag : flagsWithoutValues)
            {
                if (flag == "--no-color" || flag == "-nc")
                {
                    Config *config = getConfig();
                    config->colorOutput = false;
                }
                else
                {
                    error("Unknown flag: " + flag);
                    return 1;
                }
            }
            return 0;
        }
        int configure()
        {
            log("Starting configuration...");
            Config *config = getConfig();
            config->colorOutput = true;
            printVersion();
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
            std::filesystem::path configPath("/etc/openspm/config.yaml");
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

        int processCommandLine(std::string command,
                               const std::vector<std::string> &commandArgs,
                               const std::vector<std::pair<std::string, std::string>> &flagsWithValues,
                               const std::vector<std::string> &flagsWithoutValues)
        {
            try
            {
                if (processFlags(flagsWithValues, flagsWithoutValues) != 0)
                {
                    return 1;
                }
                if (command == "configure" || command == "config" || command == "cfg")
                {
                    return configure();
                }
                else if (command == "version" || command == "--version" || command == "-v")
                {
                    printVersion();
                    return 0;
                }
                loadConfig("/etc/openspm/config.yaml");
                Config *config = getConfig();
                int status = initDataArchive();
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
                        error("\033[1;31mFailed to remove repository: " + repoUrl);
                        return 1;
                    }
                    log("\033[1;32mSuccessfully removed repository: " + repoUrl);
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
                            log("  \033[1;34m" + repoUrl);
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
                else if (command == "list-packages" || command == "lp")
                {
                    listPackages();
                }
                else if (command == "help" || command == "--help" || command == "-h")
                {
                    log("\033[1;32mOpenSPM Help:");
                    log("  \033[1;34mconfigure                 \033[1;35mStart interactive configuration");
                    log("  \033[1;34madd-repo \033[1;37m<url>            \033[1;35mAdd a package repository");
                    log("  \033[1;34mrm-repo \033[1;37m<url>             \033[1;35mRemove a package repository");
                    log("  \033[1;34mlist-repos                \033[1;35mList all package repositories");
                    log("  \033[1;34mhelp                      \033[1;35mShow this help message");
                    log("  \033[1;34mversion                   \033[1;35mShow OpenSPM version");
                    log("  \033[1;34mupdate-repos              \033[1;35mUpdate all package repositories");
                    log("  \033[1;34mupdate                    \033[1;35mUpdate all");
                    log("");
                    log("\033[1;32mFlags (can be passed to any command):");
                    log("  \033[1;34m--data-dir \033[1;37m<dir>          \033[1;35mUse alternate data directory");
                    log("  \033[1;34m--target-dir \033[1;37m<dir>        \033[1;35mUse alternate installation target directory");
                    log("  \033[1;34m--tags \033[1;37m<tags>             \033[1;35mProvide supported tags (semicolon separated)");
                    log("  \033[1;34m--no-color, -nc           \033[1;35mDisable colored output in logs");
                }
                else
                {
                    error("\033[1;31mUnknown command: " + command);
                    return 1;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error("\033[1;31mError: " + std::string(e.what()));
                return 1;
            }
        }
        int updateAll()
        {
            log("\033[1;35mUpdating all repositories...");
            int status = openspm::updateAllRepositories();
            if (status != 0)
            {
                error("\033[1;31mFailed to update repositories.");
                return 1;
            }
            log("\033[1;32mSuccessfully updated all repositories.");
            log("\033[1;35mUpdating packages...");
            status = openspm::updatePackages();
            if (status != 0)
            {
                error("\033[1;31mFailed to update packages.");
                return 1;
            }
            return 0;
        }

        int addRepository(const std::string &repoUrl, bool skipUpdate)
        {

            RepositoryInfo repoInfo;
            bool status = getRepositoryInfo(repoUrl, repoInfo);
            if (!status)
            {
                error("\033[1;31mFailed to get repository info from URL: " + repoUrl);
                return 1;
            }
            log("\033[1;32mRepository Info:");
            log("  \033[1;34mName: \033[1;37m" + repoInfo.name);
            log("  \033[1;34mDescription: \033[1;37m" + repoInfo.description);
            log("  \033[1;34mMantainer: \033[1;37m" + repoInfo.mantainer);
            log("Are you sure you want to add this repository? (y/n): ");
            std::string response;
            std::getline(std::cin, response);
            if (response != "y" && response != "Y")
            {
                return 0;
            }
            status = openspm::addRepository(repoInfo);
            if (!status)
            {
                error("\033[1;31mFailed to add repository: " + repoUrl);
                return 1;
            }
            if (!skipUpdate)
            {
                log("\033[0;36mUpdating...");
                return updatePackages();
            }
            log("\033[1;32mSuccessfully added repository: " + repoUrl);
            return 0;
        }
        int updatePackages()
        {
            return openspm::updatePackages();
        }
        int updateRepositories()
        {
            int status = updateAllRepositories();
            log("\033[1;32mSuccessfully updated all repositories.");
            return status;
        }

        int listPackages()
        {
            std::vector<PackageInfo> packages;
            int status = listInstalledPackages(packages);
            if (status != 0)
            {
                error("\033[1;31mFailed to get packages list");
                return status;
            }
            std::string tags = getConfig()->supported_tags;
            log("\033[1;32mCompatible packages:");
            log("\033[1;32m────────────────────────────────────────────");
            for (auto &package : packages)
            {
                bool result = areTagsCompatible(tags, package.tags);
                if (result)
                {

                    log("  \033[1;36mName:        \033[1;33m" + package.name);
                    log("  \033[1;36mVersion:     \033[1;35m" + package.version);

                    if (!package.description.empty())
                        log("  \033[1;36mDescription: \033[1;33m" + package.description);
                    else
                        log("  \033[1;36mDescription: \033[1;31m<none>");

                    if (!package.maintainer.empty())
                        log("  \033[1;36mMaintainer:  \033[1;33m" + package.maintainer);
                    else
                        log("  \033[1;36mMaintainer:  \033[1;31m<unknown>");

                    if (!package.tags.empty())
                        log("  \033[1;36mTags:        \033[1;34m" + package.tags);
                    else
                        log("  \033[1;36mTags:        \033[1;31m<none>");

                    if (!package.url.empty())
                        log("  \033[1;36mURL:         \033[1;34m" + package.url);
                    else
                        log("  \033[1;36mURL:         \033[1;31m<none>");

                    log("\033[1;32m────────────────────────────────────────────\033[0m");
                }
            }
            return 0;
        }
    } // namespace cli
} // namespace openspm