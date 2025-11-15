#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>
#include <vector>
namespace openspm::frontend::utils
{
    int verifyRepository(const std::string &url);
    int fetch();
    std::vector<std::string> getAllRepositoryUrls();
}
#endif