/**
 * @file logger.hpp
 * @brief Logging utilities for OpenSPM
 */
#pragma once
#include <string>
namespace openspm
{
    namespace logger
    {
        /**
         * @brief Log an informational message to stdout
         * @param message The message to log
         */
        void log(const std::string &message);
        
        /**
         * @brief Log a warning message to stderr
         * @param message The warning message to log
         */
        void warn(const std::string &message);
        
        /**
         * @brief Log an error message to stderr
         * @param message The error message to log
         */
        void error(const std::string &message);
        
        /**
         * @brief Print the OpenSPM version and build date
         */
        void printVersion();
    } // namespace logger
} // namespace openspm