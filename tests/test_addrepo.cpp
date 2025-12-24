#include <openspm_cli.hpp>
#include <iostream>
#include <config.hpp>
using namespace openspm;
int main()
{
    int status = openspm::cli::createDefaultConfig();
    if (status != 0)
    {
        std::cout << "test failed" << std::endl;
        return 1;
    }
    loadConfig("/etc/openspm/config.yaml");
    Config *config = getConfig();
    status = initDataArchive();
    if (status != 0)
    {
        std::cout << "test failed" << std::endl;
        return 1;
    }
    status = openspm::cli::addRepository("https://testing.openspm.org", false, false);
    if (status != 0)
    {
        std::cout << "test failed" << std::endl;
        return 1;
    }
    status = openspm::cli::updateAll();
    if (status != 0)
    {
        std::cout << "test failed" << std::endl;
        return 1;
    }
    status = openspm::cli::listPackages();
    if (status != 0)
    {
        std::cout << "test failed" << std::endl;
        return 1;
    }
    return 0;
}