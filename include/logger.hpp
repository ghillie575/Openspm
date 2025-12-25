#ifndef OPENSPM_LOGGER_HPP
#define OPENSPM_LOGGER_HPP

#include <string>
#include <vector>

namespace openspm::logger
{
    /**
     * @brief Initializes logging to a physical file.
     */
    void initFileLogging();

    /**
     * @brief Toggles "Fixed Mode" (standard scrolling) vs "Interactive Mode" (overwriting lines).
     * @param v True for fixed/scrolling, False for interactive/fancy.
     */
    void setFixedMode(bool v);

    /**
     * @brief Starts a visual step in the terminal.
     * @param name The description of the task being performed.
     * @param progress Whether to track numerical progress (reserved for future use).
     */
    void startStep(const std::string &name, bool progress = false);

    /**
     * @brief Completes the current step.
     * @param success If true, shows a checkmark; if false, shows an X and error buffer.
     */
    void finishStep(bool success);

    /**
     * @brief Utility to close the current step context safely.
     */
    void endStepMode();

    /**
     * @brief Updates a progress bar or counter (implementation pending).
     */
    void updateProgress(size_t cur, size_t total);

    // --- Standard Logging Functions ---

    void log(const std::string &m);
    void warn(const std::string &m);
    void error(const std::string &m);
    void debug(const std::string &m);

    /**
     * @brief Specialized logger for HTTP interactions.
     */
    void logHttpRequest(const std::string &method, const std::string &url, int status);

    /**
     * @brief Logs the current version of OpenSPM defined in config.hpp.
     */
    void printVersion();

} // namespace openspm::logger

#endif // OPENSPM_LOGGER_HPP