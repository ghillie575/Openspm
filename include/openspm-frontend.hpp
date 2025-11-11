#ifndef SPM_FRONTEND_H
#define SPM_FRONTEND_H
#include <string>
#include <unordered_set>
#include <vector>
namespace openspm
{
    namespace frontend
    {
        void addRepository(const std::string &url);
        void processCommandLine(
        const std::vector<std::pair<std::string, std::string>> &textFlags,
        const std::vector<std::string> &simpleFlags,
        const std::string &command,
        const std::vector<std::string> &commandArgs);
    } // namespace frontend
} // namespace openspm
#endif