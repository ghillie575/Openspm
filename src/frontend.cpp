#include <openspm-frontend.hpp>
#include <repository_store.hpp>
#include <runtimeConfig.hpp>
#include <config.hpp>
#include <iostream>
#include <global.h>
#include <filesystem>
namespace openspm
{
    namespace frontend
    {
        void addRepository(const std::string &url)
        {
            if (getConfig().enable_color_output)
            {
                std::cout << "\e[33m This action will add a software repository: " << url << '\n'
                          << "Are you sure? (\e[32my\e[33m/\e[31mn\e[33m): \e[0m";
            }
            else
            {
                std::cout << "This action will add a software repository: " << url << '\n'
                          << "Are you sure? (y/n): ";
            }
            char response;
            std::cin >> response;
            if (response == 'y' || response == 'Y')
            {
                openspm::addRepository(url);
                if (getConfig().enable_color_output)
                {
                    std::cout << "\e[32mRepository added: " << url << "\e[0m\n";
                }
                else
                {
                    std::cout << "Repository added: " << url << '\n';
                }
            }
            else
            {
                if (getConfig().enable_color_output)
                {
                    std::cout << "\e[31mOperation cancelled. Mirror not added.\e[0m\n";
                }
                else
                {
                    std::cout << "Operation cancelled. Mirror not added.\n";
                }
            }
        }

        void processCommandLine(
            const std::vector<std::pair<std::string, std::string>> &textFlags,
            const std::vector<std::string> &simpleFlags,
            const std::string &command,
            const std::vector<std::string> &commandArgs)
        {
            runtimeConfig rtCfg;
            config cfg;

            for (const auto &flag : simpleFlags)
            {
                if (flag == "--debug" || flag == "-d")
                {
                    rtCfg.isDebugMode = true;
                }
                else if (flag == "--verbose" || flag == "-v")
                {
                    rtCfg.verboseLogging = true;
                }
                else
                {
                    std::cerr << "Unknown flag: " << flag << "\n";
                    return;
                }
            }
            for (const auto &flag : textFlags)
            {
                if (flag.first == "--config-dir")
                {
                    rtCfg.configDirectory = flag.second;
                }
                else if (flag.first == "--target-dir")
                {
                    rtCfg.targetDirectory = flag.second;
                }
                else if (flag.first == "--target-os")
                {
                    rtCfg.targetOs = flag.second;
                }
                else if (flag.first == "--target-compiler")
                {
                    rtCfg.targetCompiler = flag.second;
                }
                else
                {
                    std::cerr << "Unknown flag: " << flag.first << "\n";
                    return;
                }
            }
#if defined(_WIN32)
            rtCfg.targetOs = "Windows";
#elif defined(__APPLE__)
            rtCfg.targetOs = "macOS";
#elif defined(__linux__)
            rtCfg.targetOs = "Linux";
#else
#endif
            std::filesystem::path defaultConfigPath = std::filesystem::path("openspm_config") / "openspm-config.yaml";
            if (!std::filesystem::exists(defaultConfigPath))
            {
                std::filesystem::create_directories(defaultConfigPath.parent_path());
            }
            loadConfig(defaultConfigPath, cfg);
            rtCfg.configDirectory = rtCfg.configDirectory == "DEFAULT" ? cfg.configDirectory : rtCfg.configDirectory;
            rtCfg.targetCompiler = rtCfg.targetCompiler == "none" ? cfg.compiler : rtCfg.targetCompiler;
            setRuntimeConfig(rtCfg);
            setConfig(cfg);
            if (command == "add-repository")
            {
                if (commandArgs.size() < 1)
                {
                    std::cerr << "Error: 'add-repository' requires a URL argument.\n";
                    return;
                }
                for (size_t i = 0; i < commandArgs.size(); i++)
                {
                    std::string url = commandArgs[i];
                    addRepository(url);
                }
            }
        }
    } // namespace frontend
} // namespace openspm