#include <config.hpp>
#include <iostream>
#include <openspm-frontend.hpp>
#include <sstream>
#include <global.h>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <sys/utsname.h>
#endif
#include <filesystem>

std::string getCPUArchitecture()
{
#if defined(_WIN32)
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);

    switch (sysInfo.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64:
        return "x86_64";
    case PROCESSOR_ARCHITECTURE_ARM:
        return "ARM";
    case PROCESSOR_ARCHITECTURE_ARM64:
        return "ARM64";
    case PROCESSOR_ARCHITECTURE_INTEL:
        return "x86";
    default:
        return "Unknown";
    }

#elif defined(__APPLE__)
    char buffer[256];
    size_t size = sizeof(buffer);
    if (sysctlbyname("hw.machine", &buffer, &size, nullptr, 0) == 0)
    {
        std::string arch(buffer);
        return arch;
    }
    return "Unknown";

#elif defined(__linux__)
    struct utsname uts;
    if (uname(&uts) == 0)
    {
        return std::string(uts.machine);
    }
    return "Unknown";

#else
    return "Unsupported Platform";
#endif
}
int main(int argc, char *argv[])
{
    config cfg;
    std::string configFile = "openspm_config.yaml";
    loadConfig(configFile, cfg);
    if (system("gcc --version > /dev/null 2>&1") == 0)
    {
        cfg.compiler = "gcc";
        cfg.allow_noncompiled_binaries = true;
    }
    else if (system("clang --version > /dev/null 2>&1") == 0)
    {
        cfg.compiler = "clang";
        cfg.allow_noncompiled_binaries = true;
    }
    else
    {
        cfg.compiler = "none";
        cfg.allow_noncompiled_binaries = false;
    }
    std::cout << "Openspm config utility\n";
    if (!cfg.allow_noncompiled_binaries)
    {
        std::cout << "No suitable compiler found (gcc or clang). Non-compiled binaries will be disallowed.\n";
    }
    else
    {
        std::cout << "Detected compiler: " << cfg.compiler << "\n";
    }
    cfg.architecture = getCPUArchitecture();
    std::cout << "Detected architecture: " << cfg.architecture << "\n\n";
    std::cout << "\e[41m \e[42m \e[43m \e[44m \e[45m \e[46m \e[0m \e[41m \e[42m \e[43m \e[44m \e[45m \e[46m \e[0m \e[41m \e[42m \e[43m \e[44m \e[45m \e[46m \e[0m\n\n";
    std::cout << "Select loggin option:\n"
              << "1. Enable colored output\n"
              << "2. Disable colored output\n";
    int choice;
    std::cin >> choice;
    if (choice == 1)
    {
        cfg.enable_color_output = true;
    }
    else if (choice == 2)
    {
        cfg.enable_color_output = false;
    }
    else
    {
        std::cout << "Invalid choice. Exiting.\n";
        return 1;
    }
    std::cout << std::endl;
    if (cfg.enable_color_output)
    {
        std::cout << "\e[36mSelect progress bar option:\n\e[0m"
                  << "1. \e[91mEnable progress bar\n\e[0m"
                  << "2. \e[92mDisable progress bar\n\e[0m";
    }
    else
    {
        std::cout << "Select progress bar option:\n"
                  << "1. Enable progress bar"
                  << "2. Disable progress bar\n";
    }
    std::cin >> choice;
    if (choice == 1)
    {
        cfg.enable_progress_bar = true;
    }
    else if (choice == 2)
    {
        cfg.enable_progress_bar = false;
    }
    else
    {
        std::cout << "Invalid choice. Exiting.\n";
        return 1;
    }
    std::cout << std::endl;
    if (cfg.enable_color_output)
    {
        std::cout << "\e[36mSelect multithreading option:\n\e[0m"
                  << "1. \e[91mEnable multithreading\n\e[0m"
                  << "2. \e[92mDisable multithreading\n\e[0m";
    }
    else
    {
        std::cout << "Select multithreading option:\n"
                  << "1. Enable multithreading\n"
                  << "2. Disable multithreading\n";
    }
    std::cin >> choice;
    if (choice == 1)
    {
        cfg.enable_multithreading = true;
    }
    else if (choice == 2)
    {
        cfg.enable_multithreading = false;
    }
    else
    {
        std::cout << "Invalid choice. Exiting.\n";
        return 1;
    }
    std::cout << "Select configuration path (default: openspm_config): ";
    std::string configPath;
    //std::getline(std::cin, configPath);
    if(configPath.empty()){
        configPath="openspm_config";
    }
    cfg.configDirectory = configPath;
    std::filesystem::path configDir = std::filesystem::path("openspm_config");
    std::cout << "Saving configuration to " << (configDir / "openspm-config.yml").string() << "...\n";
    std::filesystem::path filePath = configDir / "openspm-config.yml";
    cfg.configDirectory = configPath;
    if (!std::filesystem::exists(configDir))
    {
        std::filesystem::create_directories(configDir);
    }
    saveConfig(filePath.string(), cfg);
    runtimeConfig rtCfg = getRuntimeConfig();
    rtCfg.configDirectory = cfg.configDirectory;
    setRuntimeConfig(rtCfg);
    setConfig(cfg);
    std::cout << "\nConfiguration saved" << std::endl;
    std::cout << "Please select default repositories to add:\n"
              << "1. Official Openspm Repository (https://repo.openspm.org)\n"
              << "2. Community Mirrors (https://mirrors.openspm.org)\n"
              << "3. Custom Repository URL\n"
              << "Enter choices separated by spaces (e.g., 1 3): ";
    std::cin.ignore();
    std::string line;
    std::getline(std::cin, line);
    std::istringstream iss(line);
    int repoChoice;
    while (iss >> repoChoice)
    {
        switch (repoChoice)
        {
        case 1:
            std::cout << "Adding Official Openspm Repository...\n";
            openspm::frontend::addRepository("https://repo.openspm.org");
            break;
        case 2:
            std::cout << "Adding Community Mirrors...\n";
            openspm::frontend::addRepository("https://mirrors.openspm.org");
            break;
        case 3:
        {
            std::cout << "Enter custom repository URL: ";
            std::string customUrl;
            std::getline(std::cin, customUrl);
            openspm::frontend::addRepository(customUrl);
            break;
        }
        default:
            std::cout << "Invalid choice: " << repoChoice << "\n";
            break;
        }
    }
}