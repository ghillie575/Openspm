/**
 * @file archive.hpp
 * @brief Compressed archive management for metadata storage
 * 
 * Provides a simple interface for reading and writing files within
 * a compressed tar.gz archive used to store OpenSPM metadata.
 */
#pragma once
#include <string>
#include <vector>
namespace openspm
{
    /**
     * @brief Manages a compressed tar.gz archive for file storage
     * 
     * The Archive class provides methods to read, write, and delete files
     * within a compressed archive. It's used by OpenSPM to store repository
     * and package metadata in a single compressed file.
     */
    class Archive
    {
    public:
        /**
         * @brief Construct an archive manager
         * @param path Path to the archive file
         */
        Archive(const std::string &path);
        
        /**
         * @brief Write or update a file in the archive
         * @param filePath Path/name of the file within the archive
         * @param data Content to write
         * @return 0 on success, non-zero on error
         */
        int writeFile(const std::string &filePath, std::string &data);
        
        /**
         * @brief Read a file from the archive
         * @param filePath Path/name of the file within the archive
         * @param outData String to populate with file content
         * @return 0 on success, non-zero if file not found or error
         */
        int readFile(const std::string &filePath, std::string &outData);
        
        /**
         * @brief Delete a file from the archive
         * @param filePath Path/name of the file to delete
         * @return 0 on success, non-zero on error
         */
        int deleteFile(const std::string &filePath);
        
        /**
         * @brief List all files in the archive
         * @param outFileList Vector to populate with file paths
         * @return 0 on success, non-zero on error
         */
        int listFiles(std::vector<std::string> &outFileList);
        
        /**
         * @brief Create an empty archive if it doesn't exist
         * @return 0 on success, non-zero on error
         */
        int createArchive();

    private:
        std::string archivePath;  ///< Path to the archive file
    };
}