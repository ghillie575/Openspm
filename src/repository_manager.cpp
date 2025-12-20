#include <repository_manager.hpp>
#include <config.hpp>
#include <logger.hpp>
#include <yaml-cpp/yaml.h>
namespace openspm
{
    using namespace logger;
    std::vector<std::string> getRepositoryList()
    {
        Config *config = getConfig();
        std::vector<std::string> repoList;
        Archive *dataArchive = getDataArchive();
        std::string reposFileContent;
        int status = dataArchive->readFile("repositories.yaml", reposFileContent);
        if (status != 0)
        {
            warn("No repositories found.");
        }
        else
        {
            YAML::Node reposNode = YAML::Load(reposFileContent);
            for (const auto &it : reposNode)
            {
                repoList.push_back(it.first.as<std::string>());
            }
        }
        return repoList;
    }
    bool addRepository(const std::string &repoUrl)
    {
        Config *config = getConfig();
        Archive *dataArchive = getDataArchive();
        std::string reposFileContent;
        int status = dataArchive->readFile("repositories.yaml",reposFileContent);
        YAML::Node reposNode;
        if (status == 0)
        {
            reposNode = YAML::Load(reposFileContent);
        }
        else
        {
            reposNode = YAML::Node(YAML::NodeType::Map);
        }
        for (const auto &it : reposNode)
        {
            if (it.first.as<std::string>() == repoUrl)
            {
                warn("Repository already exists: " + repoUrl);
                return false;
            }
        }
        reposNode[repoUrl] = YAML::Node(YAML::NodeType::Map);
        std::stringstream ss;
        ss << reposNode;
        std::string data = ss.str();
        status = dataArchive->writeFile("repositories.yaml", data);
        if (status != 0)
        {
            error("Failed to add repository: " + repoUrl);
            return false;
        }
        return true;
    }
} // namespace openspm