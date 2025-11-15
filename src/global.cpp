#include <global.h>
runtimeConfig g_runtimeConfig;
config g_config;
runtimeConfig& getRuntimeConfig()
{
    return g_runtimeConfig;
}
config& getConfig()
{
    return g_config;
}
void setRuntimeConfig(const runtimeConfig &rtCfg)
{
    g_runtimeConfig = rtCfg;
}
void setConfig(const config &cfg)
{
    g_config = cfg;
}

