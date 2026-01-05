/**
 * @file utils.hpp
 * @brief Utility functions for URL parsing and tag comparison
 * 
 * Provides helper functions for parsing URLs and comparing
 * package tags for compatibility checking.
 */
#pragma once
#include <string>
#include <vector>
namespace openspm
{
    /**
     * @brief Parsed components of a URL
     */
    struct ParsedUrl
    {
        std::string scheme;  ///< URL scheme (http, https)
        std::string host;    ///< Hostname or IP address
        int port;            ///< Port number (-1 if not specified)
        std::string path;    ///< URL path component
    };
    
    /**
     * @brief Split semicolon-separated tag string into vector
     * @param tags Semicolon-separated tag string (e.g., "bin;linux-x86_64")
     * @return Vector of individual tags
     */
    std::vector<std::string> splitTags(const std::string &tags);
    
    /**
     * @brief Check if package tags are compatible with supported tags
     * @param supported Semicolon-separated supported tags
     * @param packageTags Semicolon-separated package tags
     * @return true if all package tags are in the supported set
     */
    bool areTagsCompatible(const std::string &supported,
                           const std::string &packageTags);
    
    /**
     * @brief Parse a URL into its components
     * @param url URL string to parse
     * @return Parsed URL components
     */
    ParsedUrl parse_url(const std::string &url);
}