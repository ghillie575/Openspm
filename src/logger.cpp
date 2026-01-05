/**
 * @file logger.cpp
 * @brief Implementation of logging utilities
 */
#include <logger.hpp>
#include <config.hpp>
#include <iostream>

/**
 * @brief Remove ANSI color codes from a message string
 * @param message The message to strip color codes from (modified in place)
 */
void removeColorCodes(std::string &message)
{
    const std::string colorCodeStart = "\033[";
    size_t pos = 0;
    while ((pos = message.find(colorCodeStart, pos)) != std::string::npos)
    {
        size_t endPos = message.find('m', pos);
        if (endPos != std::string::npos)
        {
            message.erase(pos, endPos - pos + 1);
        }
        else
        {
            break;
        }
    }
}

namespace openspm
{
    namespace logger
    {
        void log(const std::string &message)
        {
            if (getConfig()->colorOutput == false)
                removeColorCodes(const_cast<std::string &>(message));
            std::cout << message << "\033[0m" << std::endl;
        }

        void warn(const std::string &message)
        {
            if (getConfig()->colorOutput == false)
                removeColorCodes(const_cast<std::string &>(message));
            std::cerr << message << "\033[0m" << std::endl;
        }

        void error(const std::string &message)
        {
            if (getConfig()->colorOutput == false)
                removeColorCodes(const_cast<std::string &>(message));
            std::cerr << message << "\033[0m" << std::endl;
        }
        void printVersion()
        {
            log("OpenSPM v" + std::string(OPENSPM_VERSION) + " build " + std::string(OPENSPM_BUILD_DATE));
        }
    } // namespace logger
} // namespace openspm