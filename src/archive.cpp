#include <archive.hpp>
#include <archive.h>
#include <archive_entry.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <filesystem>
#include <sys/stat.h>
#include <map>

namespace openspm {

Archive::Archive(const std::string &path) : archivePath(path) {}

int Archive::createArchive() {
    std::filesystem::path pathObj(archivePath);
    if (std::filesystem::exists(pathObj)) {
        return 0; // Archive already exists
    }
    if( !std::filesystem::exists(pathObj.parent_path())) {
        try {
            std::filesystem::create_directories(pathObj.parent_path());
        } catch (const std::exception &e) {
            return 1;
        }
    }
    struct archive *a = archive_write_new();
    if (!a) return 2;
    
    archive_write_set_format_pax_restricted(a);
    archive_write_add_filter_gzip(a);
    
    int ret = archive_write_open_filename(a, pathObj.c_str());
    if (ret != ARCHIVE_OK) {
        archive_write_free(a);
        return 3;
    }
    
    archive_write_close(a);
    archive_write_free(a);
    return 0;
}

int Archive::writeFile(const std::string &filePath, std::string &data) {
    std::map<std::string, std::string> files;
    std::vector<std::string> fileList;
    
    if (listFiles(fileList) == 0) {
        for (const auto &file : fileList) {
            std::string content;
            if (readFile(file, content) == 0) {
                files[file] = content;
            }
        }
    }
    
    // Add or update the file
    files[filePath] = data;
    
    // Recreate archive with all files
    struct archive *a = archive_write_new();
    if (!a) return -1;
    
    archive_write_set_format_pax_restricted(a);
    archive_write_add_filter_gzip(a);
    
    if (archive_write_open_filename(a, archivePath.c_str()) != ARCHIVE_OK) {
        archive_write_free(a);
        return -1;
    }
    
    for (const auto &[path, content] : files) {
        struct archive_entry *entry = archive_entry_new();
        archive_entry_set_pathname(entry, path.c_str());
        archive_entry_set_size(entry, content.size());
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        
        archive_write_header(a, entry);
        archive_write_data(a, content.c_str(), content.size());
        archive_entry_free(entry);
    }
    
    archive_write_close(a);
    archive_write_free(a);
    return 0;
}

int Archive::readFile(const std::string &filePath, std::string &outData) {
    struct archive *a = archive_read_new();
    if (!a) return -1;
    
    archive_read_support_filter_gzip(a);
    archive_read_support_format_all(a);
    
    if (archive_read_open_filename(a, archivePath.c_str(), 10240) != ARCHIVE_OK) {
        archive_read_free(a);
        return -1;
    }
    
    struct archive_entry *entry;
    int found = -1;
    
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *pathname = archive_entry_pathname(entry);
        if (pathname && filePath == pathname) {
            size_t size = archive_entry_size(entry);
            outData.resize(size);
            
            ssize_t readSize = archive_read_data(a, &outData[0], size);
            if (readSize < 0) {
                archive_read_free(a);
                return -1;
            }
            
            found = 0;
            break;
        }
        archive_read_data_skip(a);
    }
    
    archive_read_close(a);
    archive_read_free(a);
    return found;
}

int Archive::deleteFile(const std::string &filePath) {
    std::map<std::string, std::string> files;
    std::vector<std::string> fileList;
    
    if (listFiles(fileList) != 0) {
        return -1;
    }
    
    bool found = false;
    for (const auto &file : fileList) {
        if (file == filePath) {
            found = true;
            continue;
        }
        std::string content;
        if (readFile(file, content) == 0) {
            files[file] = content;
        }
    }
    
    if (!found) return -1;
    
    // Recreate archive without the deleted file
    struct archive *a = archive_write_new();
    if (!a) return -1;
    
    archive_write_set_format_pax_restricted(a);
    archive_write_add_filter_gzip(a);
    
    if (archive_write_open_filename(a, archivePath.c_str()) != ARCHIVE_OK) {
        archive_write_free(a);
        return -1;
    }
    
    for (const auto &[path, content] : files) {
        struct archive_entry *entry = archive_entry_new();
        archive_entry_set_pathname(entry, path.c_str());
        archive_entry_set_size(entry, content.size());
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        
        archive_write_header(a, entry);
        archive_write_data(a, content.c_str(), content.size());
        archive_entry_free(entry);
    }
    
    archive_write_close(a);
    archive_write_free(a);
    return 0;
}

int Archive::listFiles(std::vector<std::string> &outFileList) {
    struct archive *a = archive_read_new();
    if (!a) return -1;
    
    archive_read_support_filter_gzip(a);
    archive_read_support_format_all(a);
    
    if (archive_read_open_filename(a, archivePath.c_str(), 10240) != ARCHIVE_OK) {
        archive_read_free(a);
        return -1;
    }
    
    struct archive_entry *entry;
    outFileList.clear();
    
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *pathname = archive_entry_pathname(entry);
        if (pathname) {
            outFileList.push_back(pathname);
        }
        archive_read_data_skip(a);
    }
    
    archive_read_close(a);
    archive_read_free(a);
    return 0;
}
} // namespace openspm