#include <config.hpp>
#include <fstream>
#include <yaml-cpp/yaml.h>
void loadConfig(const std::string &filepath, config &cfg)
{
    try
    {
        YAML::Node configNode = YAML::LoadFile(filepath);
        if (configNode["enable_color_output"])
            cfg.enable_color_output = configNode["enable_color_output"].as<bool>();
        if (configNode["enable_verbose_logging"])
            cfg.enable_verbose_logging = configNode["enable_verbose_logging"].as<bool>();
        if (configNode["enable_debug_mode"])
            cfg.enable_debug_mode = configNode["enable_debug_mode"].as<bool>();
        if (configNode["enable_progress_bar"])
            cfg.enable_progress_bar = configNode["enable_progress_bar"].as<bool>();
        if (configNode["enable_file_logging"])
            cfg.enable_file_logging = configNode["enable_file_logging"].as<bool>();
        if (configNode["enable_multithreading"])
            cfg.enable_multithreading = configNode["enable_multithreading"].as<bool>();
        if (configNode["enable_cache"])
            cfg.enable_cache = configNode["enable_cache"].as<bool>();
        if (configNode["allow_insecure_repositories"])
            cfg.allow_insecure_repositories = configNode["allow_insecure_repositories"].as<bool>();
    }
    catch (const std::exception &e)
    {
    }
}
void saveConfig(const std::string &filepath, const config &cfg){
    YAML::Node configNode;
    configNode["enable_color_output"] = cfg.enable_color_output;
    configNode["enable_verbose_logging"] = cfg.enable_verbose_logging;
    configNode["enable_debug_mode"] = cfg.enable_debug_mode;
    configNode["enable_progress_bar"] = cfg.enable_progress_bar;
    configNode["enable_file_logging"] = cfg.enable_file_logging;
    configNode["enable_multithreading"] = cfg.enable_multithreading;
    configNode["enable_cache"] = cfg.enable_cache;
    configNode["allow_insecure_repositories"] = cfg.allow_insecure_repositories;

    std::ofstream fout(filepath);
    fout << configNode;
    fout.close();
}
