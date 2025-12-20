#include <openspm_cli.hpp>
#include <repository_manager.hpp>
#include <filesystem>
#include <iostream>
#include <logger.hpp>
#include <config.hpp>
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
            if(!std::filesystem::exists(configPath.parent_path()))
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
                    return addRepository(repoUrl, false) ? 0 : 1;
                }
                else if (command == "help" || command == "--help" || command == "-h")
                {
                    log("OpenSPM Help:");
                    log("  configure                 Start interactive configuration");
                    log("  add-repo <url>            Add a package repository");
                    log("  list-repos                List all package repositories");
                    log("  help                      Show this help message");
                    log("  version                   Show OpenSPM version");
                    log("");
                    log("Flags (can be passed to any command):");
                    log("  --data-dir <dir>          Use alternate data directory");
                    log("  --target-dir <dir>        Use alternate installation target directory");
                    log("  --tags <tags>             Provide supported tags (semicolon separated)");
                    log("  --no-color, -nc           Disable colored output in logs");
                }
                else
                {
                    error("Unknown command: " + command);
                    return 1;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error("Error: " + std::string(e.what()));
                return 1;
            }
        }

        int addRepository(const std::string &repoUrl, bool skipUpdate)
        {
            return openspm::addRepository(repoUrl) ? 0 : 1;
        }
    } // namespace cli
} // namespace openspm