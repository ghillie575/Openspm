#include <openspm-frontend.hpp>
#include <repository_store.hpp>
#include <iostream>
namespace openspm
{
    namespace frontend
    {
        void addRepository(const std::string &url)
        {
            std::cout << "This action will add a software repository: " << url << '\n'
                      << "Are you sure? (y/n): ";
            char response;
            std::cin >> response;
            if (response == 'y' || response == 'Y')
            {
                openspm::addRepository(url);
                std::cout << "Repository added: " << url << '\n';
            }
            else
            {
                std::cout << "Operation cancelled. Mirror not added.\n";
            }
        }

        void processCommandLine(
            const std::vector<std::pair<std::string, std::string>> &textFlags,
            const std::vector<std::string> &simpleFlags,
            const std::string &command,
            const std::vector<std::string> &commandArgs)
        {
            if (command == "add-repository")
            {
                if (commandArgs.size() < 1)
                {
                    std::cerr << "Error: 'add-repository' requires a URL argument.\n";
                    return;
                }
                for (size_t i = 0; i < commandArgs.size(); i++)
                {
                    std::string url = commandArgs[i];
                    addRepository(url);
                }
            }
        }
    } // namespace frontend
} // namespace openspm