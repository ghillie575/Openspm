/**
 * @file config.cpp
 * @brief Implementation of configuration management
 * 
 * Handles loading and saving configuration files, YAML serialization,
 * and managing global configuration state and data archive.
 */
#ifndef OPENSPM_VERSION
#error "OPENSPM_VERSION is not defined"
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
            std::cout <<"\033[0;33mConfig file does not exist. Using default configuration." << std::endl;
            return;
        }
        std::ifstream file(configPath);
        if (!file.is_open())
        {
            std::cout << "\033[0;33mFailed to open config file. Using default configuration." << std::endl;
            return;
        }
        std::string yamlStr((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        globalConfig = fromYaml(yamlStr);
        file.close();
        debug("[DEBUG loadConfig] Config loaded successfully");
        debug("[DEBUG loadConfig] dataDir: " + globalConfig.dataDir);
        debug("[DEBUG loadConfig] targetDir: " + globalConfig.targetDir);
        debug("[DEBUG loadConfig] supported_tags: " + globalConfig.supported_tags);
    }
    Config *getConfig()
    {
        return &globalConfig;
    }
    void saveConfig(std::string configPath, const Config &config)
    {
        debug("[DEBUG saveConfig] Saving config to: " + configPath);
        log("Saving config to " + configPath);
        std::filesystem::path pathObj(configPath);
        if (!std::filesystem::exists(pathObj.parent_path()))
        {
            debug("[DEBUG saveConfig] Parent directory doesn't exist, creating");
            try
            {
                std::filesystem::create_directories(pathObj.parent_path());
                debug("[DEBUG saveConfig] Created directories: " + pathObj.parent_path().string());
            }
            catch (const std::exception &e)
            {
                error("\033[0;31mFailed to create directories for config file: " + std::string(e.what()));
                return;
            }
        }
        debug("[DEBUG saveConfig] Opening config file for writing");
        std::ofstream file(configPath);
        if (!file.is_open())
        {
            error("\033[0;31mFailed to open config file for writing.");
            return;
        }
        debug("[DEBUG saveConfig] Converting config to YAML");
        std::string yamlStr = toYaml(config);
        debug("[DEBUG saveConfig] YAML size: " + std::to_string(yamlStr.size()) + " bytes");
        file << yamlStr;
        file.close();
        debug("[DEBUG saveConfig] Config saved successfully");
    }
    std::string toYaml(const Config &config)
    {
        debug("[DEBUG toYaml] Converting config to YAML");
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
        debug("[DEBUG toYaml] Conversion complete");
        return std::string(out.c_str());
    }
    Config fromYaml(const std::string &yamlStr)
    {
        debug("[DEBUG fromYaml] Parsing YAML string");
        Config config;
        YAML::Node node = YAML::Load(yamlStr);
        if (node["dataDir"]) {
            config.dataDir = node["dataDir"].as<std::string>();
            debug("[DEBUG fromYaml] dataDir: " + config.dataDir);
        }
        if (node["targetDir"]) {
            config.targetDir = node["targetDir"].as<std::string>();
            debug("[DEBUG fromYaml] targetDir: " + config.targetDir);
        }
        if (node["colorOutput"]) {
            config.colorOutput = node["colorOutput"].as<bool>();
            debug("[DEBUG fromYaml] colorOutput: " + std::to_string(config.colorOutput));
        }
        if (node["platform"]) {
            config.platform = node["platform"].as<std::string>();
            debug("[DEBUG fromYaml] platform: " + config.platform);
        }
        if (node["supported_tags"]) {
            config.supported_tags = node["supported_tags"].as<std::string>();
            debug("[DEBUG fromYaml] supported_tags: " + config.supported_tags);
        }
        if (node["supported"]) {
            config.supported = node["supported"].as<bool>();
            debug("[DEBUG fromYaml] supported: " + std::to_string(config.supported));
        }
        if (node["unsupported_msg"]) {
            config.unsupported_msg = node["unsupported_msg"].as<std::string>();
            debug("[DEBUG fromYaml] unsupported_msg: " + config.unsupported_msg);
        }
        debug("[DEBUG fromYaml] Parse complete");
        return config;
    }
    Archive *getDataArchive()
    {
        debug("[DEBUG getDataArchive] Returning global archive pointer: " + std::to_string((long)globalArchive));
        return globalArchive;
    }
    int initDataArchive()
    {
        debug("[DEBUG initDataArchive] Initializing data archive");
        if (globalArchive != nullptr)
        {
            debug("[DEBUG initDataArchive] Archive already initialized");
            return 0; // Already initialized
        }
        Config *config = getConfig();
        std::filesystem::path dataDirPath(config->dataDir);
        debug("[DEBUG initDataArchive] Data directory: " + dataDirPath.string());
        if (!std::filesystem::exists(dataDirPath))
        {
            debug("[DEBUG initDataArchive] Data directory doesn't exist, creating");
            try
            {
                std::filesystem::create_directories(dataDirPath);
                debug("[DEBUG initDataArchive] Created data directory");
            }
            catch (const std::exception &e)
            {
                error("\033[0;31mFailed to create data directory: " + std::string(e.what()));
                return 1;
            }
        }
        std::string archivePath = dataDirPath.append("data.bin").string();
        debug("[DEBUG initDataArchive] Archive path: " + archivePath);
        globalArchive = new Archive(archivePath);
        debug("[DEBUG initDataArchive] Archive object created at: " + std::to_string((long)globalArchive));
        int result = globalArchive->createArchive();
        debug("[DEBUG initDataArchive] createArchive returned: " + std::to_string(result));
        return result;
    }

} // namespace openspm