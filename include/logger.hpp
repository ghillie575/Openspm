#pragma once
#include <string>
namespace openspm
{
    namespace logger
    {
        void log(const std::string &message);
        void warn(const std::string &message);
        void error(const std::string &message);
        void printVersion();
    } // namespace logger
} // namespace openspm