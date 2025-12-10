#include <string>
#include <vector>
namespace openspm{
    namespace archive{
        class Archive{
        public:
            Archive(const std::string &path);
            ~Archive();
            int writeFile(const std::string &filePath, std::string &data);
            int readFile(const std::string &filePath, std::string &outData);
            int deleteFile(const std::string &filePath);
            int listFiles(std::vector<std::string> &outFileList);
            int close();
        private:
            std::string archivePath;
        };
    } // namespace archive
}