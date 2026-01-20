/**
 * @file utils.cpp
 * @brief Implementation of utility functions
 * 
 * Provides URL parsing and tag comparison functionality for
 * package compatibility checking.
 */
#include <utils.hpp>
#include <sstream>
#include <unordered_set>
#include <logger.hpp>

namespace openspm
{
    using namespace logger;
    
    ParsedUrl parse_url(const std::string &url)
    {
        debug("[DEBUG parse_url] Parsing URL: " + url);
        ParsedUrl result;
        result.port = -1;
        result.path = "";
        std::string s = url;

        auto scheme_pos = s.find("://");
        if (scheme_pos != std::string::npos)
        {
            result.scheme = s.substr(0, scheme_pos);
            debug("[DEBUG parse_url] Scheme: " + result.scheme);
            s = s.substr(scheme_pos + 3);
        }

        auto path_pos = s.find('/');
        if (path_pos != std::string::npos)
        {
            result.path = s.substr(path_pos);
            debug("[DEBUG parse_url] Path: " + result.path);
            s = s.substr(0, path_pos);
        }

        if (result.path.size() > 1 && result.path.back() == '/')
        {
            result.path.pop_back();
            debug("[DEBUG parse_url] Removed trailing slash from path");
        }

        auto port_pos = s.find(':');
        if (port_pos != std::string::npos)
        {
            result.host = s.substr(0, port_pos);
            result.port = std::stoi(s.substr(port_pos + 1));
            debug("[DEBUG parse_url] Host: " + result.host + ", Port: " + std::to_string(result.port));
        }
        else
        {
            result.host = s;
            debug("[DEBUG parse_url] Host: " + result.host + " (no explicit port)");
        }

        debug("[DEBUG parse_url] Parse complete");
        return result;
    }
    
    std::vector<std::string> splitTags(const std::string &tags)
    {
        debug("[DEBUG splitTags] Splitting tags: " + tags);
        std::vector<std::string> result;
        std::stringstream ss(tags);
        std::string tag;

        while (std::getline(ss, tag, ';'))
        {
            if (!tag.empty()) {
                debug("[DEBUG splitTags] Found tag: " + tag);
                result.push_back(tag);
            }
        }
        debug("[DEBUG splitTags] Total tags found: " + std::to_string(result.size()));
        return result;
    }
    
    bool areTagsCompatible(const std::string &supported,
                           const std::string &packageTags)
    {
        debug("[DEBUG areTagsCompatible] Checking compatibility");
        debug("[DEBUG areTagsCompatible] Supported tags: " + supported);
        debug("[DEBUG areTagsCompatible] Package tags: " + packageTags);
        
        auto supportedVec = splitTags(supported);
        auto packageVec = splitTags(packageTags);

        debug("[DEBUG areTagsCompatible] Supported count: " + std::to_string(supportedVec.size()));
        debug("[DEBUG areTagsCompatible] Package tags count: " + std::to_string(packageVec.size()));

        std::unordered_set<std::string> supportedSet(
            supportedVec.begin(), supportedVec.end());

        for (const auto &tag : packageVec)
        {
            if (supportedSet.find(tag) == supportedSet.end()) {
                debug("[DEBUG areTagsCompatible] Incompatible tag found: " + tag);
                return false;
            }
        }

        debug("[DEBUG areTagsCompatible] All tags compatible");
        return true;
    }
} // namespace openspm