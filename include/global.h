#ifndef GLOBAL_H
#define GLOBAL_H
#include <string>
#include <runtimeConfig.hpp>
#include <config.hpp>
runtimeConfig getRuntimeConfig();
config getConfig();
void setRuntimeConfig(const runtimeConfig &rtCfg);
void setConfig(const config &cfg);
#endif