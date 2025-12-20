#pragma once
#include <string>
#include <vector>
namespace openspm
{
    class Archive
    {
    public:
        Archive(const std::string &path);
        int writeFile(const std::string &filePath, std::string &data);
        int readFile(const std::string &filePath, std::string &outData);
        int deleteFile(const std::string &filePath);
        int listFiles(std::vector<std::string> &outFileList);
        int createArchive();

    private:
        std::string archivePath;
    };
}