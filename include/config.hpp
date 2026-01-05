/**
 * @file config.hpp
 * @brief Configuration management for OpenSPM
 */
#pragma once
#include <string>
#include <archive.hpp>
namespace openspm
{
    /**
     * @brief Configuration structure for OpenSPM
     * 
     * Holds all configuration settings including directories,
     * platform information, and supported tags.
     */
    struct Config
    {
        std::string dataDir = "/etc/openspm/";      ///< Directory for OpenSPM data storage
        std::string targetDir = "/usr/local/";      ///< Target directory for package installation
        bool colorOutput = true;                     ///< Enable/disable colored terminal output
#ifdef _windows_
        std::string platform = "windows-x86_64";    ///< Current platform identifier
#elif __APPLE__
        std::string platform = "macos-x86_64";      ///< Current platform identifier
#else
        std::string platform = "linux-x86_64";      ///< Current platform identifier
#endif
        std::string supported_tags = "bin;" + platform + ";";  ///< Semicolon-separated list of supported tags
        bool supported = true;                       ///< Whether the platform is supported
        std::string unsupported_msg = "";           ///< Message displayed if platform is unsupported
    };
    
    /**
     * @brief Convert a Config object to YAML string
     * @param config The configuration to convert
     * @return YAML string representation of the config
     */
    std::string toYaml(const Config &config);
    
    /**
     * @brief Parse a YAML string into a Config object
     * @param yamlStr The YAML string to parse
     * @return Config object parsed from YAML
     */
    Config fromYaml(const std::string &yamlStr);
    
    /**
     * @brief Load configuration from a file
     * @param configPath Path to the configuration file
     */
    void loadConfig(std::string configPath);
    
    /**
     * @brief Get the global configuration object
     * @return Pointer to the global Config object
     */
    Config *getConfig();
    
    /**
     * @brief Get the global data archive object
     * @return Pointer to the global Archive object
     */
    Archive *getDataArchive();
    
    /**
     * @brief Save configuration to a file
     * @param configPath Path to save the configuration file
     * @param config The configuration to save
     */
    void saveConfig(std::string configPath, const Config &config);
    
    /**
     * @brief Initialize the data archive
     * @return 0 on success, non-zero on error
     */
    int initDataArchive();
} // namespace openspm