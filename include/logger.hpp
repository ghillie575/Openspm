/**
 * @file logger.hpp
 * @brief Logging system with file and console output
 * 
 * Provides logging functions with multiple levels (log, warn, error, debug)
 * and ANSI color support for console output.
 */
#ifndef OPENSPM_LOGGER_HPP
#define OPENSPM_LOGGER_HPP

#include <string>
#include <vector>

/// ANSI color code for resetting terminal color
#define CLR_RESET "\033[0m"
/// ANSI color code for cyan text
#define CLR_CYAN "\033[0;36m"
/// ANSI color code for green text
#define CLR_GREEN "\033[0;32m"
/// ANSI color code for red text
#define CLR_RED "\033[0;31m"
/// ANSI color code for yellow text
#define CLR_YELLOW "\033[0;33m"
/// ANSI color code for gray text
#define CL_GRAY "\033[0;30m"
/// ANSI color code for purple text
#define CL_PURPLE "\033[0;35m"
/// ANSI escape sequence to clear current line
#define CLEAR_LINE "\033[2K\r"
namespace openspm::logger
{
    /**
     * @brief Initialize logging to a physical file
     * 
     * Opens the log file specified in configuration and writes a session start message.
     * If the log directory doesn't exist, it will be created.
     */
    void initFileLogging();
    
    /**
     * @brief Log an informational message
     * @param m Message to log
     */
    void log(const std::string &m);
    
    /**
     * @brief Log a warning message (displayed in yellow)
     * @param m Warning message to log
     */
    void warn(const std::string &m);
    
    /**
     * @brief Log an error message (displayed in red)
     * @param m Error message to log
     */
    void error(const std::string &m);
    
    /**
     * @brief Log a debug message (only shown when debug mode is enabled)
     * @param m Debug message to log
     */
    void debug(const std::string &m);

    /**
     * @brief Log an HTTP request with status code
     * @param method HTTP method (GET, POST, etc.)
     * @param url Request URL
     * @param status HTTP status code
     */
    void logHttpRequest(const std::string &method, const std::string &url, int status);
    
    /**
     * @brief Log an HTTP request without status code
     * @param method HTTP method (GET, POST, etc.)
     * @param url Request URL
     */
    void logHttpRequest(const std::string &method, const std::string &url);

    /**
     * @brief Print the OpenSPM version to console
     * 
     * Displays the version string defined during compilation.
     */
    void printVersion();

} // namespace openspm::logger

#endif // OPENSPM_LOGGER_HPP