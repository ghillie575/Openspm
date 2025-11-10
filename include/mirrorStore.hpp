#ifndef MIRRORSTORE_H
#define MIRRORSTORE_H
#include <string>
#include <vector>
void addMirror(const std::string& url);
std::vector<std::string> getMirrors();
void writeMirrorsToFile(const std::string& filename);
#endif