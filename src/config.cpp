#ifndef OPENSPM_VERSION
#error "OPENSPM_VERSION is not defined"
#endif
#ifndef OPENSPM_BUILD_DATE
#error "OPENSPM_BUILD_DATE is not defined"
#endif
#include <iostream>
#include <vector>
#include <string>
#include <logger.hpp>
#include <config.hpp>
#include <yaml-cpp/yaml.h>
#include <archive.hpp>
#include <fstream>
#include <filesystem>
namespace openspm
{
    static Config globalConfig;
    static Archive *globalArchive = nullptr;
    using namespace logger;
    void loadConfig(std::string configPath)
    {
        std::filesystem::path pathObj(configPath);
        if (!std::filesystem::exists(pathObj))
        {
            warn("\033[1;33mConfig file does not exist. Using default configuration.");
            return;
        }
        std::ifstream file(configPath);
        if (!file.is_open())
        {
            warn("\033[1;33mFailed to open config file. Using default configuration.");
            return;
        }
        std::string yamlStr((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        globalConfig = fromYaml(yamlStr);
        file.close();
    }
    Config *getConfig()
    {
        return &globalConfig;
    }
    void saveConfig(std::string configPath, const Config &config)
    {
        log("Saving config to " + configPath);
        std::filesystem::path pathObj(configPath);
        if (!std::filesystem::exists(pathObj.parent_path()))
        {
            try
            {
                std::filesystem::create_directories(pathObj.parent_path());
            }
            catch (const std::exception &e)
            {
                error("\033[1;31mFailed to create directories for config file: " + std::string(e.what()));
                return;
            }
        }
        std::ofstream file(configPath);
        if (!file.is_open())
        {
            error("\033[1;31mFailed to open config file for writing.");
            return;
        }
        std::string yamlStr = toYaml(config);
        file << yamlStr;
        file.close();
    }
    std::string toYaml(const Config &config)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "dataDir" << YAML::Value << config.dataDir;
        out << YAML::Key << "targetDir" << YAML::Value << config.targetDir;
        out << YAML::Key << "colorOutput" << YAML::Value << config.colorOutput;
        out << YAML::Key << "platform" << YAML::Value << config.platform;
        out << YAML::Key << "supported_tags" << YAML::Value << config.supported_tags;
        out << YAML::Key << "supported" << YAML::Value << config.supported;
        out << YAML::Key << "unsupported_msg" << YAML::Value << config.unsupported_msg;
        out << YAML::EndMap;
        return std::string(out.c_str());
    }
    Config fromYaml(const std::string &yamlStr)
    {
        Config config;
        YAML::Node node = YAML::Load(yamlStr);
        if (node["dataDir"])
            config.dataDir = node["dataDir"].as<std::string>();
        if (node["targetDir"])
            config.targetDir = node["targetDir"].as<std::string>();
        if (node["colorOutput"])
            config.colorOutput = node["colorOutput"].as<bool>();
        if (node["platform"])
            config.platform = node["platform"].as<std::string>();
        if (node["supported_tags"])
            config.supported_tags = node["supported_tags"].as<std::string>();
        if (node["supported"])
            config.supported = node["supported"].as<bool>();
        if (node["unsupported_msg"])
            config.unsupported_msg = node["unsupported_msg"].as<std::string>();
        return config;
    }
    Archive *getDataArchive()
    {
        return globalArchive;
    }
    int initDataArchive()
    {
        if (globalArchive != nullptr)
        {
            return 0; // Already initialized
        }
        Config *config = getConfig();
        std::filesystem::path dataDirPath(config->dataDir);
        if (!std::filesystem::exists(dataDirPath))
        {
            try
            {
                std::filesystem::create_directories(dataDirPath);
            }
            catch (const std::exception &e)
            {
                error("\033[1;31mFailed to create data directory: " + std::string(e.what()));
                return 1;
            }
        }
        std::string archivePath = dataDirPath.append("data.bin").string();
        globalArchive = new Archive(archivePath);
        return globalArchive->createArchive();
    }

} // namespace openspm