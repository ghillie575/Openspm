#include <openspm-frontend.hpp>
#include <runtimeConfig.hpp>
#include <config.hpp>
#include <global.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <repository_store.hpp>
#include <logger.hpp>
#include <regex>
#include <utils.hpp>
#include <httplib.h>
#include <package_info.h>
#if defined(_WIN32)
#include <windows.h>
#endif

namespace openspm::frontend
{
    void fetch()
    {
        logMessage(color::cyan + std::string("Fetching updates from all repositories...") + color::reset, false);
        if(utils::fetch() != 0){
            logMessage(color::red + std::string("Failed to fetch updates from repositories.") + color::reset, true);
        }
    }
    // ============================================================
    // Add Repository
    // ============================================================
    void addRepository(const std::string &url)
    {
        bool useColor = getConfig().enable_color_output;
        color::init(useColor);

        if (url.empty())
        {
            logMessage(color::red + std::string("Error: Repository URL cannot be empty.") + color::reset, true);
            return;
        }
        int verificationResult = utils::verifyRepository(url);
        if (verificationResult == -1)
        {
            logMessage(color::red + std::string("Repository verification failed due to an error. Repository not added.") + color::reset, true);
            return; // Verification failed, error already logged
        }
        if (verificationResult != 0)
        {
            logMessage(color::red + std::string("Repository verification failed. This repository may be unsecure.") + color::reset, true);
            logMessage(
                (useColor ? color::yellow : "") + std::string("Are you sure want to continue? (") +
                    (useColor ? color::green + "y" + color::yellow + "/" + color::red + "n" + color::yellow : "y/n") +
                    "): " + color::reset,
                false);

            char response;
            std::cin >> response;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (response == 'y' || response == 'Y')
            {
                try
                {
                    int result = openspm::addRepository(url);
                    if (result == 0)
                    {
                        logMessage(color::green + std::string("Repository added: ") + url + color::reset, false);
                    }
                    else if (result == 3)
                    {
                        logMessage(color::yellow + std::string("Repository already exists: ") + url + color::reset, false);
                    }
                    else
                    {
                        logMessage(color::red + std::string("Failed to add repository: ") + url + color::reset, true);
                    }
                }
                catch (const std::exception &e)
                {
                    logMessage(color::red + std::string("Error adding repository: ") + e.what() + color::reset, true);
                }
                return;
            }
            else
            {
                logMessage(color::yellow + std::string("Operation cancelled. Repository not added.") + color::reset, false);
                return;
            }
        }

        debugLog(getRuntimeConfig(), "Prompting user for confirmation to add repository: " + url);

        logMessage(
            (useColor ? color::yellow : "") + std::string("This action will add a software repository:\n  ") +
                color::bold + url + color::reset,
            false);

        logMessage(
            (useColor ? color::yellow : "") + std::string("Are you sure? (") +
                (useColor ? color::green + "y" + color::yellow + "/" + color::red + "n" + color::yellow : "y/n") +
                "): " + color::reset,
            false);

        char response;
        std::cin >> response;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (response == 'y' || response == 'Y')
        {
            try
            {
                int result = openspm::addRepository(url);
                if (result == 0)
                {
                    logMessage(color::green + std::string("Repository added: ") + url + color::reset, false);
                }
                else if (result == 3)
                {
                    logMessage(color::yellow + std::string("Repository already exists: ") + url + color::reset, false);
                }
                else
                {
                    logMessage(color::red + std::string("Failed to add repository: ") + url + color::reset, true);
                }
            }
            catch (const std::exception &e)
            {
                logMessage(color::red + std::string("Error adding repository: ") + e.what() + color::reset, true);
            }
        }
        else
        {
            logMessage(color::yellow + std::string("Operation cancelled. Repository not added.") + color::reset, false);
        }
    }

    // ============================================================
    // Command-Line Processor
    // ============================================================
    void processCommandLine(
        const std::vector<std::pair<std::string, std::string>> &textFlags,
        const std::vector<std::string> &simpleFlags,
        const std::string &command,
        const std::vector<std::string> &commandArgs)
    {

        runtimeConfig rtCfg;
        config cfg;

        // Parse simple flags
        for (const auto &flag : simpleFlags)
        {
            if (flag == "--debug" || flag == "-d")
                rtCfg.isDebugMode = true;
            else if (flag == "--verbose" || flag == "-v")
                rtCfg.verboseLogging = true;
            else if (flag == "--no-color")
                cfg.enable_color_output = false;
            else if (flag == "--no-progress-bar")
                cfg.enable_progress_bar = false;
            else
            {
                logMessage(color::red + std::string("Unknown flag: ") + flag + color::reset, true);
                return;
            }
        }

        // Parse text flags
        for (const auto &flag : textFlags)
        {
            if (flag.first == "--config-dir")
                rtCfg.configDirectory = flag.second;
            else if (flag.first == "--target-dir")
                rtCfg.targetDirectory = flag.second;
            else if (flag.first == "--target-os")
                rtCfg.targetOs = flag.second;
            else if (flag.first == "--target-compiler")
                rtCfg.targetCompiler = flag.second;

            else
            {
                logMessage(color::red + std::string("Unknown flag: ") + flag.first + color::reset, true);
                return;
            }
        }

        // Detect platform
#if defined(_WIN32)
        rtCfg.targetOs = "Windows";
#elif defined(__APPLE__)
        rtCfg.targetOs = "macOS";
#elif defined(__linux__)
        rtCfg.targetOs = "Linux";
#else
        rtCfg.targetOs = "Unknown";
#endif

        color::init(cfg.enable_color_output);
        debugLog(rtCfg, "Detected OS: " + rtCfg.targetOs);

        // Prepare default config
        std::filesystem::path defaultConfigPath("openspm_config/openspm-config.yaml");
        try
        {
            if (!std::filesystem::exists(defaultConfigPath.parent_path()))
            {
                std::filesystem::create_directories(defaultConfigPath.parent_path());
            }
        }
        catch (const std::exception &e)
        {
            logMessage(color::red + std::string("Error creating config directory: ") + e.what() + color::reset, true);
            return;
        }

        try
        {
            loadConfig(defaultConfigPath, cfg);
            debugLog(rtCfg, "Loaded configuration from " + defaultConfigPath.string());
        }
        catch (const std::exception &e)
        {
            logMessage(color::red + std::string("Error loading config: ") + e.what() + color::reset, true);
            return;
        }
        debugLog(rtCfg, "Configuration loaded successfully.");
        debugLog(rtCfg, "Config Directory: " + (rtCfg.configDirectory.empty() ? cfg.configDirectory : rtCfg.configDirectory));
        if (rtCfg.configDirectory.empty())
        {
            rtCfg.configDirectory = cfg.configDirectory;
        }
        setRuntimeConfig(rtCfg);
        setConfig(cfg);

        if (cfg.enable_debug_mode || rtCfg.isDebugMode)
            logMessage(color::cyan + std::string("Debug mode enabled.") + color::reset, false);

        // Execute command
        if (command == "add-repository")
        {
            if (commandArgs.empty())
            {
                logMessage(color::red + std::string("Error: 'add-repository' requires at least one URL argument.") + color::reset, true);
                return;
            }

            for (const auto &url : commandArgs)
                addRepository(url);
        }
        else if (command == "fetch")
        {
            fetch();
        }
        else
        {
            logMessage(color::red + std::string("Unknown command: ") + command + color::reset, true);
        }
        
    }

} // namespace openspm::frontend
