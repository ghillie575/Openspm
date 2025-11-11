#include <string>
#ifndef SPM_CONFIG_H
#define SPM_CONFIG_H
struct config
{
    bool enable_color_output = true;
    bool enable_verbose_logging = false;
    bool enable_debug_mode = false;
    bool enable_progress_bar = true;
    bool enable_file_logging = false;
    bool enable_multithreading = true;
    bool enable_cache = true;
    bool allow_insecure_repositories = false;
};
void loadConfig(const std::string &filepath, config &cfg);
void saveConfig(const std::string &filepath, const config &cfg);
#endif
