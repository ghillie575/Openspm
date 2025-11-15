#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <iostream>
#include <string>
#include <global.h>
#include <regex>
namespace openspm::frontend
{
    namespace color
    {
        inline std::string reset, red, green, yellow, blue, magenta, cyan, bold;

        inline void init(bool enableColor)
        {
            if (!enableColor)
            {
                reset = red = green = yellow = blue = magenta = cyan = bold = "";
                return;
            }

#if defined(_WIN32)
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hOut != INVALID_HANDLE_VALUE)
            {
                DWORD dwMode = 0;
                if (GetConsoleMode(hOut, &dwMode))
                {
                    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
                }
            }
#endif

            reset = "\033[0m";
            red = "\033[31m";
            green = "\033[32m";
            yellow = "\033[33m";
            blue = "\033[34m";
            magenta = "\033[35m";
            cyan = "\033[36m";
            bold = "\033[1m";
        }
    }

    inline std::string stripAnsiColors(const std::string &input)
    {
        static const std::regex ansi_escape("\033\\[[0-9;]*m");
        return std::regex_replace(input, ansi_escape, "");
    }
    inline void logMessage(const std::string &msg, bool isError = false)
    {
        bool useColor = getConfig().enable_color_output;
        std::string output = useColor ? msg : stripAnsiColors(msg);
        auto &out = isError ? std::cerr : std::cout;
        out << output << '\n';
    }

    inline void debugLog(const runtimeConfig &rtCfg, const std::string &msg)
    {
        if (rtCfg.isDebugMode)
        {
            logMessage("[DEBUG] " + msg, true);
        }
    }
} // namespace openspm

#endif