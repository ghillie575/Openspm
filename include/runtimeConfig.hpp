#include <string>
#ifndef SPM_RUNTIME_CONFIG_H
#define SPM_RUNTIME_CONFIG_H
struct runtimeConfig
{
    bool isDebugMode = false;
    bool verboseLogging = false;
    std::string configDirectory = "";
    std::string targetDirectory = "";
    std::string targetOs = "";
    std::string targetCompiler = "none";
};
#endif
