#include <logger.hpp>
#include <config.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <regex>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace openspm::logger
{
#define CLR_RESET "\033[0m"
#define CLR_CYAN "\033[1;36m"
#define CLR_GREEN "\033[1;32m"
#define CLR_RED "\033[1;31m"
#define CLR_YELLOW "\033[1;33m"
#define CL_GRAY "\033[1;30m"
#define CL_PURPLE "\033[1;35m"
#define CLEAR_LINE "\033[2K\r"

    inline std::string MOVE_UP(size_t n) { return "\033[" + std::to_string(n) + "A"; }

#ifdef _WIN32
    static void enableVT()
    {
        static bool enabled = false;
        if (!enabled)
        {
            HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD mode;
            if (GetConsoleMode(h, &mode))
                SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            enabled = true;
        }
    }
#endif

    struct LogEntry
    {
        std::string text;
        bool isStep;
        bool isError;
    };

    static bool fixedMode = false;
    static bool usingStep = false;
    static std::string stepName;
    static std::vector<LogEntry> visible;
    static std::vector<LogEntry> stepBuffer;
    static size_t anchorHeight = 0;
    static const size_t MAX_LINES = 5;
    static std::ofstream logFile;

    static std::string getTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    static std::string stripAnsi(std::string str)
    {
        static const std::regex ansi_regex("\x1b\\[[0-9;]*[a-zA-Z]");
        return std::regex_replace(str, ansi_regex, "");
    }

    void initFileLogging()
    {
        logFile.open(getConfig()->logsFile, std::ios::app);
        if (logFile.is_open())
        {
            logFile << "\n--- Session Started: " << getTimestamp() << " ---\n";
        }else{
            std::cout << "Log File Failure: " << getConfig()->logsFile << std::endl;
        }
    }
    static void redraw()
    {
        if (fixedMode)
            return;
        if (anchorHeight)
            std::cout << MOVE_UP(anchorHeight);

        for (const auto &e : visible)
        {
            std::cout << CLEAR_LINE;
            if (!e.isStep && !fixedMode)
                std::cout << CLR_RESET << "|  ";
            std::cout << e.text << std::endl;
        }
        anchorHeight = visible.size();
    }

    static void pushVisible(const LogEntry &e)
    {
        visible.push_back(e);
        while (visible.size() > MAX_LINES)
        {
            if (visible.front().isStep && visible.size() > 1)
                visible.erase(visible.begin() + 1);
            else
                visible.erase(visible.begin());
        }
    }

    void setFixedMode(bool v) { fixedMode = v; }

    void startStep(const std::string &name, bool progress)
    {
#ifdef _WIN32
        enableVT();
#endif
        usingStep = true;
        stepName = name;
        visible.clear();
        stepBuffer.clear();

        if (!fixedMode)
        {
            pushVisible({CLR_CYAN "⟳ " + stepName + CLR_RESET, true, false});
            redraw();
        }
        else
        {
            std::cout << CLR_CYAN "⟳ " << stepName << CLR_RESET << std::endl;
        }
    }

    void finishStep(bool success)
    {
        if (!usingStep)
            return;
        usingStep = false;

        if (!fixedMode && anchorHeight)
        {
            std::cout << MOVE_UP(anchorHeight);
            for (size_t i = 0; i < anchorHeight; ++i)
                std::cout << CLEAR_LINE << (i + 1 < anchorHeight ? "\n" : "");
            if (anchorHeight > 1)
                std::cout << MOVE_UP(anchorHeight - 1);
            visible.clear();
            anchorHeight = 0;
        }

        if (success)
        {
            std::cout << CLR_GREEN "✓ " << stepName << CLR_RESET << std::endl;
        }
        else
        {
            std::cout << CLR_RED "✗ " << stepName << " failed" << CLR_RESET << std::endl;
            for (const auto &e : stepBuffer)
                if (e.isError)
                    std::cout << "|  " << e.text << std::endl;
        }
        stepBuffer.clear();
    }

    void updateProgress(size_t cur, size_t total) {}

    void endStepMode()
    {
        if (usingStep)
            finishStep(true);
    }

    static void emit(const std::string &txt, bool isError)
    {
        if (usingStep)
            stepBuffer.push_back({txt, isError});

        if (!fixedMode)
        {
            pushVisible({txt, false, isError});
            redraw();
        }
        else
        {
            if (!fixedMode)
                std::cout << CLR_RESET << "|  ";
            std::cout << txt << std::endl;
        }
    }

    void log(const std::string &m) { emit(m, false); }
    void warn(const std::string &m) { emit(CLR_YELLOW "⚠ " + m + CLR_RESET, true); }
    void error(const std::string &m) { emit(CLR_RED "✗ " + m + CLR_RESET, true); }

    void debug(const std::string &m)
    {
        if (getConfig()->debug)
            emit(CL_GRAY "🔍 " + m + CLR_RESET, false);
    }

    void logHttpRequest(const std::string &method, const std::string &url, int status)
    {
        std::string clr = status >= 400 ? CLR_RED : CLR_GREEN;
        log(std::string(CLR_CYAN) + method + " " + CL_PURPLE + url + " " + clr + "[" + std::to_string(status) + "]" + CLR_RESET);
    }

    void printVersion() { log("OpenSPM v" + std::string(OPENSPM_VERSION)); }

}