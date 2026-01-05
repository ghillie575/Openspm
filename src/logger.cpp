/**
 * @file logger.cpp
 * @brief Implementation of logging system
 * 
 * Provides console and file logging with ANSI color support,
 * timestamp generation, and multiple log levels.
 */
#include <logger.hpp>
#include <config.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <chrono>
#include <iomanip>
#include <filesystem>

namespace openspm::logger
{
    static std::ofstream logFile;

    /// Generate timestamp for log entries
    static std::string getTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    /// Remove ANSI color codes from string
    static std::string stripAnsi(std::string str)
    {
        static const std::regex ansi_regex("\x1b\\[[0-9;]*[a-zA-Z]");
        return std::regex_replace(str, ansi_regex, "");
    }

    void initFileLogging()
    {
        std::filesystem::path logPath(getConfig()->logsFile);
        if (!std::filesystem::exists(logPath.parent_path()))
        {
            std::filesystem::create_directories(logPath.parent_path());
        }

        logFile.open(logPath, std::ios::app);
        if (logFile.is_open())
        {
            logFile << "\n--- Session Started: " << getTimestamp() << " ---\n";
        }
        else
        {
            std::cerr << "Log File Failure: " << getConfig()->logsFile << std::endl;
        }
    }

    /// Emit message to console and log file
    static void emit(const std::string &txt)
    {
        bool useColor = getConfig()->colorOutput;
        std::string processedText = useColor ? txt : stripAnsi(txt);

        // Output to Console
        std::cout << CLR_RESET << processedText << CLR_RESET << std::endl;

        // Output to File (always stripped of ANSI codes)
        if (logFile.is_open())
        {
            logFile << getTimestamp() << ": " << stripAnsi(txt) << std::endl;
        }
    }

    void log(const std::string &m) { emit(m); }
    
    void warn(const std::string &m) { emit(CLR_YELLOW "W: " + m + CLR_RESET); }
    
    void error(const std::string &m) { emit(CLR_RED "E: " + m + CLR_RESET); }

    void debug(const std::string &m)
    {
        if (getConfig()->debug)
            emit(CL_GRAY "D: " + m + CLR_RESET);
    }

    void logHttpRequest(const std::string &method, const std::string &url, int status)
    {
        std::string clr = status >= 400 ? CLR_RED : CLR_GREEN;
        std::string msg = std::string(CLR_CYAN) + method + " " + 
                          CL_PURPLE + url + " " + 
                          clr + "[" + std::to_string(status) + "]" + CLR_RESET;
        emit(msg);
    }
    void logHttpRequest(const std::string &method, const std::string &url)
    {
        std::string clr = CLR_GREEN;
        std::string msg = std::string(CLR_CYAN) + method + " " + 
                          CL_PURPLE + url + " " + 
                          clr + CLR_RESET;
        emit(msg);
    }

    void printVersion() {
    const std::string name = "OpenSPM";
    const std::string version = "v" + std::string(OPENSPM_VERSION);

    // Calculate box width
    size_t inner_width = std::max(name.size(), version.size()) + 4; // padding
    std::string horizontal(inner_width, '-');

    // ANSI colors
    const std::string BLUE = "\033[1;34m";
    const std::string CYAN = "\033[1;36m";
    const std::string RESET = "\033[0m";

    // Top border
    log(BLUE + "+" + horizontal + "+" + CLR_RESET);

    // Name line
    log(BLUE + "|" + CLR_RESET +
        "  " + CYAN + name +
        std::string(inner_width - name.size() - 2, ' ') +
        BLUE + "|" + CLR_RESET);

    // Version line
    log(BLUE + "|" + CLR_RESET +
        "  " + CLR_GREEN + version +
        std::string(inner_width - version.size() - 2, ' ') +
        BLUE + "|" + CLR_RESET);

    // Bottom border
    log(BLUE + "+" + horizontal + "+" + CLR_RESET);
}

}