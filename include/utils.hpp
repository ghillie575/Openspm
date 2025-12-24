#pragma once
#include <string>
#include <vector>
namespace openspm
{
    struct ParsedUrl
    {
        std::string scheme;
        std::string host;
        int port;
        std::string path;
    };
    std::vector<std::string> splitTags(const std::string &tags);
    bool areTagsCompatible(const std::string &supported,
                           const std::string &packageTags);
    ParsedUrl parse_url(const std::string &url);
}