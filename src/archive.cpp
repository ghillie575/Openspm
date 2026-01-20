/**
 * @file archive.cpp
 * @brief Implementation of compressed archive management
 *
 * Uses libarchive to provide read/write operations for tar.gz archives
 * used to store OpenSPM metadata files.
 */
#ifdef _WIN32
#include <BaseTsd.h>
using ssize_t = SSIZE_T;
#endif
#include <archive.hpp>
#include <archive.h>
#include <archive_entry.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <filesystem>
#include <sys/stat.h>
#include <map>
#include <vector>
#include <logger.hpp>

namespace openspm
{
    using namespace logger;
    Archive::Archive(const std::string &path) : archivePath(path)
    {
        debug("[DEBUG Archive::Archive] Created archive object for: " + path);
    }

    int Archive::createArchive()
    {
        debug("[DEBUG Archive::createArchive] Starting archive creation: " + archivePath);
        std::filesystem::path pathObj(archivePath);
        if (std::filesystem::exists(pathObj))
        {
            debug("[DEBUG Archive::createArchive] Archive already exists, skipping creation");
            return 0; // Archive already exists
        }
        if (!std::filesystem::exists(pathObj.parent_path()))
        {
            debug("[DEBUG Archive::createArchive] Parent directory doesn't exist, creating: " + pathObj.parent_path().string());
            try
            {
                std::filesystem::create_directories(pathObj.parent_path());
                debug("[DEBUG Archive::createArchive] Successfully created parent directory");
            }
            catch (const std::exception &e)
            {
                error("Failed to create parent directory: " + std::string(e.what()));
                return 1;
            }
        }
        debug("[DEBUG Archive::createArchive] Creating new archive writer");
        struct archive *a = archive_write_new();
        if (!a)
        {
            error("Failed to create archive writer");
            return 2;
        }

        debug("[DEBUG Archive::createArchive] Setting archive format and compression");
        archive_write_set_format_pax_restricted(a);
        archive_write_add_filter_gzip(a);

        debug("[DEBUG Archive::createArchive] Opening archive file: " + pathObj.string());
#ifdef _WIN32
        int ret = archive_write_open_filename(a, pathObj.string().c_str());
#else
        int ret = archive_write_open_filename(a, pathObj.string().c_str());
#endif
        if (ret != ARCHIVE_OK)
        {
            error("Failed to open archive file");
            archive_write_free(a);
            return 3;
        }

        debug("[DEBUG Archive::createArchive] Closing and freeing archive");
        archive_write_close(a);
        archive_write_free(a);
        debug("[DEBUG Archive::createArchive] Archive creation complete");
        return 0;
    }

    int Archive::writeFile(const std::string &filePath, std::string &data)
    {
        debug("[DEBUG Archive::writeFile] Writing to archive: " + archivePath);
        debug("[DEBUG Archive::writeFile] Target file: " + filePath);
        debug("[DEBUG Archive::writeFile] Data size: " + std::to_string(data.size()) + " bytes");

        std::map<std::string, std::string> files;
        std::vector<std::string> fileList;

        debug("[DEBUG Archive::writeFile] Reading existing files...");
        if (listFiles(fileList) == 0)
        {
            debug("[DEBUG Archive::writeFile] Found " + std::to_string(fileList.size()) + " existing files");
            for (const auto &file : fileList)
            {
                debug("[DEBUG Archive::writeFile] Existing file: " + file);
                std::string content;
                if (readFile(file, content) == 0)
                {
                    files[file] = content;
                    debug("[DEBUG Archive::writeFile] Read " + std::to_string(content.size()) + " bytes from: " + file);
                }
                else
                {
                    debug("[DEBUG Archive::writeFile] Failed to read existing file: " + file);
                }
            }
        }
        else
        {
            debug("[DEBUG Archive::writeFile] No existing files or error listing");
        }

        // Add or update the file
        debug("[DEBUG Archive::writeFile] Adding/updating file: " + filePath);
        files[filePath] = data;

        debug("[DEBUG Archive::writeFile] Total files to write: " + std::to_string(files.size()));
        for (const auto &[path, content] : files)
        {
            debug("[DEBUG Archive::writeFile] Will write: " + path + " (" + std::to_string(content.size()) + " bytes)");
        }

        // Recreate archive with all files
        debug("[DEBUG Archive::writeFile] Creating archive writer");
        struct archive *a = archive_write_new();
        if (!a)
        {
            error("ERROR: Failed to create archive writer");
            return -1;
        }

        archive_write_set_format_pax_restricted(a);
        archive_write_add_filter_gzip(a);

        debug("[DEBUG Archive::writeFile] Opening archive for writing: " + archivePath);
        if (archive_write_open_filename(a, archivePath.c_str()) != ARCHIVE_OK)
        {
            error("ERROR: Failed to open archive for writing");
            archive_write_free(a);
            return -1;
        }

        for (const auto &[path, content] : files)
        {
            debug("[DEBUG Archive::writeFile] Writing entry: " + path);
            struct archive_entry *entry = archive_entry_new();
            archive_entry_set_pathname(entry, path.c_str());
            archive_entry_set_size(entry, content.size());
            archive_entry_set_filetype(entry, AE_IFREG);
            archive_entry_set_perm(entry, 0644);

            int header_ret = archive_write_header(a, entry);
            if (header_ret != ARCHIVE_OK)
            {
                error("ERROR: Failed to write header for: " + path);
            }

            ssize_t written = archive_write_data(a, content.c_str(), content.size());
            if (written < 0 || (size_t)written != content.size())
            {
                error("ERROR: Failed to write data for: " + path + " (wrote " + std::to_string(written) + " of " + std::to_string(content.size()) + " bytes)");
            }
            else
            {
                debug("[DEBUG Archive::writeFile] Successfully wrote " + std::to_string(written) + " bytes for: " + path);
            }

            archive_entry_free(entry);
        }

        debug("[DEBUG Archive::writeFile] Closing archive...");
        archive_write_close(a);
        archive_write_free(a);

        debug("[DEBUG Archive::writeFile] Write complete!");
        return 0;
    }

    int Archive::readFile(const std::string &filePath, std::string &outData)
    {
        debug("[DEBUG Archive::readFile] Reading file: " + filePath + " from archive: " + archivePath);
        struct archive *a = archive_read_new();
        if (!a)
        {
            error("Failed to create archive reader");
            return -1;
        }

        archive_read_support_filter_gzip(a);
        archive_read_support_format_all(a);

        debug("[DEBUG Archive::readFile] Opening archive: " + archivePath);
        if (archive_read_open_filename(a, archivePath.c_str(), 10240) != ARCHIVE_OK)
        {
            error("Failed to open archive");
            archive_read_free(a);
            return -1;
        }

        struct archive_entry *entry;
        int found = -1;

        debug("[DEBUG Archive::readFile] Searching for file in archive...");
        while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
        {
            const char *pathname = archive_entry_pathname(entry);
            if (pathname)
            {
                debug("[DEBUG Archive::readFile] Found entry: " + std::string(pathname));
                if (filePath == pathname)
                {
                    debug("[DEBUG Archive::readFile] Match found! Reading data...");
                    size_t size = archive_entry_size(entry);
                    debug("[DEBUG Archive::readFile] Entry size: " + std::to_string(size) + " bytes");
                    outData.resize(size);

                    ssize_t readSize = archive_read_data(a, &outData[0], size);
                    if (readSize < 0)
                    {
                        error("Failed to read data from entry");
                        archive_read_free(a);
                        return -1;
                    }

                    debug("[DEBUG Archive::readFile] Successfully read " + std::to_string(readSize) + " bytes");
                    found = 0;
                    break;
                }
            }
            archive_read_data_skip(a);
        }

        if (found != 0)
        {
            debug("[DEBUG Archive::readFile] File not found in archive: " + filePath);
        }

        archive_read_close(a);
        archive_read_free(a);
        debug("[DEBUG Archive::readFile] Archive closed, returning status: " + std::to_string(found));
        return found;
    }

    int Archive::deleteFile(const std::string &filePath)
    {
        debug("[DEBUG Archive::deleteFile] Deleting file: " + filePath);
        std::map<std::string, std::string> files;
        std::vector<std::string> fileList;

        debug("[DEBUG Archive::deleteFile] Listing files in archive");
        if (listFiles(fileList) != 0)
        {
            error("Failed to list files");
            return -1;
        }

        debug("[DEBUG Archive::deleteFile] Found " + std::to_string(fileList.size()) + " files");
        bool found = false;
        for (const auto &file : fileList)
        {
            if (file == filePath)
            {
                debug("[DEBUG Archive::deleteFile] Found target file, skipping in rebuild");
                found = true;
                continue;
            }
            debug("[DEBUG Archive::deleteFile] Preserving file: " + file);
            std::string content;
            if (readFile(file, content) == 0)
            {
                files[file] = content;
            }
        }

        if (!found)
        {
            warn("[DEBUG Archive::deleteFile] File not found in archive: " + filePath);
            return -1;
        }

        // Recreate archive without the deleted file
        debug("[DEBUG Archive::deleteFile] Recreating archive without deleted file");
        struct archive *a = archive_write_new();
        if (!a)
        {
            error("Failed to create archive writer");
            return -1;
        }

        archive_write_set_format_pax_restricted(a);
        archive_write_add_filter_gzip(a);

        if (archive_write_open_filename(a, archivePath.c_str()) != ARCHIVE_OK)
        {
            error("Failed to open archive for writing");
            archive_write_free(a);
            return -1;
        }

        for (const auto &[path, content] : files)
        {
            debug("[DEBUG Archive::deleteFile] Writing preserved file: " + path);
            struct archive_entry *entry = archive_entry_new();
            archive_entry_set_pathname(entry, path.c_str());
            archive_entry_set_size(entry, content.size());
            archive_entry_set_filetype(entry, AE_IFREG);
            archive_entry_set_perm(entry, 0644);

            archive_write_header(a, entry);
            archive_write_data(a, content.c_str(), content.size());
            archive_entry_free(entry);
        }

        debug("[DEBUG Archive::deleteFile] Closing archive");
        archive_write_close(a);
        archive_write_free(a);
        debug("[DEBUG Archive::deleteFile] Delete operation complete");
        return 0;
    }

    int Archive::listFiles(std::vector<std::string> &outFileList)
    {
        debug("[DEBUG Archive::listFiles] Listing files in archive: " + archivePath);
        struct archive *a = archive_read_new();
        if (!a)
        {
            error("Failed to create archive reader");
            return -1;
        }

        archive_read_support_filter_gzip(a);
        archive_read_support_format_all(a);

        if (archive_read_open_filename(a, archivePath.c_str(), 10240) != ARCHIVE_OK)
        {
            debug("[DEBUG Archive::listFiles] Failed to open archive (may not exist yet)");
            archive_read_free(a);
            return -1;
        }

        struct archive_entry *entry;
        outFileList.clear();

        while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
        {
            const char *pathname = archive_entry_pathname(entry);
            if (pathname)
            {
                debug("[DEBUG Archive::listFiles] Found file: " + std::string(pathname));
                outFileList.push_back(pathname);
            }
            archive_read_data_skip(a);
        }

        debug("[DEBUG Archive::listFiles] Total files found: " + std::to_string(outFileList.size()));
        archive_read_close(a);
        archive_read_free(a);
        return 0;
    }
} // namespace openspm