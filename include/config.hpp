#pragma once
#include <string>
#include <archive.hpp>
namespace openspm
{
    struct Config
    {
        std::string dataDir = "/etc/openspm/";
        std::string targetDir = "/usr/local/";
        bool colorOutput = true;
#ifdef _windows_
        std::string platform = "windows-x86_64";
#elif __APPLE__
        std::string platform = "macos-x86_64";
#else
        std::string platform = "linux-x86_64";
#endif
        std::string supported_tags = "bin;" + platform + ";";
        bool supported = true;
        bool debug = false;
        std::string logsFile = "/var/log/openspm/latest.log";
        std::string unsupported_msg = "";
    };
    std::string toYaml(const Config &config);
    Config fromYaml(const std::string &yamlStr);
    void loadConfig(std::string configPath);
    Config *getConfig();
    Archive *getDataArchive();
    void saveConfig(std::string configPath, const Config &config);
    int initDataArchive();
} // namespace openspm