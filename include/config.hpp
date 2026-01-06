/**
 * @file config.hpp
 * @brief Configuration management and data archive access
 * 
 * Handles loading and saving configuration, managing global config state,
 * and providing access to the data archive for metadata storage.
 */
#pragma once
#include <string>
#include <archive.hpp>
namespace openspm
{
    /**
     * @brief OpenSPM configuration settings
     */
    struct Config
    {
#ifdef _WIN32
        std::string dataDir = "C:\\ProgramData\\openspm\\";      ///< Directory for metadata storage
        std::string targetDir = "C:\\Program Files\\openspm\\";  ///< Target directory for package installation
#else
        std::string dataDir = "/etc/openspm/";      ///< Directory for metadata storage
        std::string targetDir = "/usr/local/";      ///< Target directory for package installation
#endif
        bool colorOutput = true;                     ///< Enable colored console output
#ifdef _WIN32
        std::string platform = "windows-x86_64";    ///< Platform identifier
#elif __APPLE__
        std::string platform = "macos-x86_64";      ///< Platform identifier
#else
        std::string platform = "linux-x86_64";      ///< Platform identifier
#endif
        std::string supported_tags = "bin;" + platform + ";";  ///< Semicolon-separated supported tags
        bool supported = true;                       ///< Whether this platform is supported
        bool debug = false;                          ///< Enable debug logging
        bool scrollingText = true;                   ///< Enable scrolling text (unused)
#ifdef _WIN32
        std::string logsFile = "C:\\ProgramData\\openspm\\logs\\openspm.log";  ///< Log file path
#else
        std::string logsFile = "/var/log/openspm/openspm.log";  ///< Log file path
#endif
        std::string unsupported_msg = "";            ///< Message if platform unsupported
    };
    
    /**
     * @brief Serialize configuration to YAML string
     * @param config Configuration to serialize
     * @return YAML string representation
     */
    std::string toYaml(const Config &config);
    
    /**
     * @brief Parse configuration from YAML string
     * @param yamlStr YAML string to parse
     * @return Parsed configuration
     */
    Config fromYaml(const std::string &yamlStr);
    
    /**
     * @brief Load configuration from file
     * @param configPath Path to config file
     */
    void loadConfig(std::string configPath);
    
    /**
     * @brief Get pointer to global configuration
     * @return Pointer to global config instance
     */
    Config *getConfig();
    
    /**
     * @brief Get pointer to global data archive
     * @return Pointer to archive instance (may be nullptr if not initialized)
     */
    Archive *getDataArchive();
    
    /**
     * @brief Save configuration to file
     * @param configPath Path to save config file
     * @param config Configuration to save
     */
    void saveConfig(std::string configPath, const Config &config);
    
    /**
     * @brief Initialize the global data archive
     * @return 0 on success, non-zero on error
     */
    int initDataArchive();
} // namespace openspm