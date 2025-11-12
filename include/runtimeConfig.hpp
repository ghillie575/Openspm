#include <string>
#ifndef SPM_RUNTIME_CONFIG_H
#define SPM_RUNTIME_CONFIG_H
struct runtimeConfig
{
    bool isDebugMode = false;
    bool verboseLogging = false;
    std::string configDirectory = "DEFAULT";
    std::string targetDirectory = "DEFAULT";
    std::string targetOs = "";
    std::string targetCompiler = "none";
};
#endif
