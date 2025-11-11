#ifndef MIRRORSTORE_H
#define MIRRORSTORE_H
#include <string>
#include <vector>
namespace openspm {
void addRepository(const std::string& url);
std::vector<std::string> getRepositories();
std::vector<std::string> getRepositoriesFromFile(const std::string& filename);
void writeRepositoriesToFile(const std::string& filename, const std::vector<std::string>& mirrors);
}
#endif