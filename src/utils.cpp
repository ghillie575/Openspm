#include <utils.hpp>
#include <sstream>
#include <unordered_set>
namespace openspm
{
    ParsedUrl parse_url(const std::string &url)
    {
        ParsedUrl result;
        result.port = -1;
        result.path = "";
        std::string s = url;

        auto scheme_pos = s.find("://");
        if (scheme_pos != std::string::npos)
        {
            result.scheme = s.substr(0, scheme_pos);
            s = s.substr(scheme_pos + 3);
        }

        auto path_pos = s.find('/');
        if (path_pos != std::string::npos)
        {
            result.path = s.substr(path_pos);
            s = s.substr(0, path_pos);
        }

        if (result.path.size() > 1 && result.path.back() == '/')
        {
            result.path.pop_back();
        }

        auto port_pos = s.find(':');
        if (port_pos != std::string::npos)
        {
            result.host = s.substr(0, port_pos);
            result.port = std::stoi(s.substr(port_pos + 1));
        }
        else
        {
            result.host = s;
        }

        return result;
    }
    std::vector<std::string> splitTags(const std::string &tags)
    {
        std::vector<std::string> result;
        std::stringstream ss(tags);
        std::string tag;

        while (std::getline(ss, tag, ';'))
        {
            if (!tag.empty())
                result.push_back(tag);
        }
        return result;
    }
    bool areTagsCompatible(const std::string &supported,
                           const std::string &packageTags)
    {
        auto supportedVec = splitTags(supported);
        auto packageVec = splitTags(packageTags);

        std::unordered_set<std::string> supportedSet(
            supportedVec.begin(), supportedVec.end());

        for (const auto &tag : packageVec)
        {
            if (supportedSet.find(tag) == supportedSet.end())
                return false;
        }

        return true;
    }
} // namespace openspm
