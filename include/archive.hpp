/**
 * @file archive.hpp
 * @brief Archive management for OpenSPM data storage
 */
#pragma once
#include <string>
#include <vector>
namespace openspm
{
    /**
     * @brief Class for managing compressed archive files
     * 
     * Provides methods to read, write, and manipulate files within
     * a compressed archive using libarchive.
     */
    class Archive
    {
    public:
        /**
         * @brief Construct an Archive object
         * @param path Path to the archive file
         */
        Archive(const std::string &path);
        
        /**
         * @brief Write data to a file within the archive
         * @param filePath Path of the file within the archive
         * @param data Data to write to the file
         * @return 0 on success, non-zero on error
         */
        int writeFile(const std::string &filePath, std::string &data);
        
        /**
         * @brief Read data from a file within the archive
         * @param filePath Path of the file within the archive
         * @param outData Output parameter for the file data
         * @return 0 on success, non-zero on error
         */
        int readFile(const std::string &filePath, std::string &outData);
        
        /**
         * @brief Delete a file from the archive
         * @param filePath Path of the file to delete
         * @return 0 on success, non-zero on error
         */
        int deleteFile(const std::string &filePath);
        
        /**
         * @brief List all files in the archive
         * @param outFileList Output parameter for the list of file paths
         * @return 0 on success, non-zero on error
         */
        int listFiles(std::vector<std::string> &outFileList);
        
        /**
         * @brief Create a new archive file
         * @return 0 on success, non-zero on error
         */
        int createArchive();

    private:
        std::string archivePath;  ///< Path to the archive file
    };
}