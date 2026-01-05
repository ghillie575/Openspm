#ifndef OPENSPM_LOGGER_HPP
#define OPENSPM_LOGGER_HPP

#include <string>
#include <vector>
#define CLR_RESET "\033[0m"
#define CLR_CYAN "\033[0;36m"
#define CLR_GREEN "\033[0;32m"
#define CLR_RED "\033[0;31m"
#define CLR_YELLOW "\033[0;33m"
#define CL_GRAY "\033[0;30m"
#define CL_PURPLE "\033[0;35m"
#define CLEAR_LINE "\033[2K\r"
namespace openspm::logger
{
    /**
     * @brief Initializes logging to a physical file.
     */
    void initFileLogging();
    // --- Standard Logging Functions ---

    void log(const std::string &m);
    void warn(const std::string &m);
    void error(const std::string &m);
    void debug(const std::string &m);

    /**
     * @brief Specialized logger for HTTP interactions.
     */
    void logHttpRequest(const std::string &method, const std::string &url, int status);
    void logHttpRequest(const std::string &method, const std::string &url);

    /**
     * @brief Logs the current version of OpenSPM defined in config.hpp.
     */
    void printVersion();

} // namespace openspm::logger

#endif // OPENSPM_LOGGER_HPP