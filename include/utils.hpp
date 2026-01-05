/**
 * @file utils.hpp
 * @brief Utility functions for OpenSPM
 */
#pragma once
#include <string>
#include <vector>
namespace openspm
{
    /**
     * @brief Structure to hold parsed URL components
     */
    struct ParsedUrl
    {
        std::string scheme;  ///< URL scheme (e.g., "http", "https")
        std::string host;    ///< Hostname or IP address
        int port;            ///< Port number (-1 if not specified)
        std::string path;    ///< URL path component
    };
    
    /**
     * @brief Split a semicolon-separated tag string into individual tags
     * @param tags The semicolon-separated tag string
     * @return Vector of individual tag strings
     */
    std::vector<std::string> splitTags(const std::string &tags);
    
    /**
     * @brief Check if package tags are compatible with supported tags
     * @param supported The supported tags (semicolon-separated)
     * @param packageTags The package tags (semicolon-separated)
     * @return true if all package tags are in the supported tags list
     */
    bool areTagsCompatible(const std::string &supported,
                           const std::string &packageTags);
    
    /**
     * @brief Parse a URL string into its components
     * @param url The URL string to parse
     * @return ParsedUrl structure containing the URL components
     */
    ParsedUrl parse_url(const std::string &url);
}