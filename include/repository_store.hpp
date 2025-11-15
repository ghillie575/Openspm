#ifndef MIRRORSTORE_H
#define MIRRORSTORE_H

#include <string>
#include <vector>

namespace openspm
{

    int addRepository(const std::string &url);
    std::vector<std::string> getRepositoriesFromFile(const std::string &filename);
    int writeRepositoriesToFile(const std::string &filename, const std::vector<std::string> &mirrors);

} // namespace openspm

#endif // MIRRORSTORE_H