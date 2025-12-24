#include <repository_manager.hpp>
#include <config.hpp>
#include <logger.hpp>
#include <yaml-cpp/yaml.h>
#include <httplib.h>
#include <utils.hpp>
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
            warn("\033[1;33mNo repositories found.");
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
    bool fetchRepositoryInfo(const std::string &repoUrl, RepositoryInfo &outInfo)
    {
        auto parsed = parse_url(repoUrl);

        httplib::Client *cli = nullptr;
        httplib::SSLClient *sslCli = nullptr;
        bool useSSL = false;
        if (parsed.scheme == "https")
        {
            if (parsed.port > 0)
                sslCli = new httplib::SSLClient(parsed.host, parsed.port);
            else
                sslCli = new httplib::SSLClient(parsed.host);
            useSSL = true;
        }
        else
        {
            if (parsed.port > 0)
                cli = new httplib::Client(parsed.host, parsed.port);
            else
                cli = new httplib::Client(parsed.host);
            warn("\033[1;33mW: Repository URL is not using HTTPS: \033[1;37m" + repoUrl);
        }
        if (getConfig()->colorOutput)
        {
            std::cout << "\033[1;36mGET \033[1;35m" << parsed.scheme << "://" << parsed.host << (parsed.port > 0 ? (":" + std::to_string(parsed.port)) : "") << parsed.path << "/repository.yaml ";
        }
        else
        {
            std::cout << "GET " << parsed.scheme << "://" << parsed.host << (parsed.port > 0 ? (":" + std::to_string(parsed.port)) : "") << parsed.path << "/repository.yaml ";
        }
        if (useSSL)
        {
            auto res = sslCli->Get((parsed.path + "/repository.yaml").c_str());
            if (res && res->status == 200)
            {
                YAML::Node repoNode = YAML::Load(res->body);
                outInfo.url = repoUrl;
                outInfo.name = repoNode["name"].as<std::string>();
                outInfo.description = repoNode["description"].as<std::string>();
                outInfo.mantainer = repoNode["mantainer"].as<std::string>();
                delete sslCli;
                std::cout << "\033[1;32m" << res->status << "\033[0m" << std::endl;
                return true;
            }
            else
            {
                std::cout << "\033[1;31m" << (res ? std::to_string(res->status) : "no response") << "\033[0m" << std::endl;
                delete sslCli;
                return false;
            }
        }
        else
        {
            auto res = cli->Get((parsed.path + "/repository.yaml").c_str());
            if (res && res->status == 200)
            {
                YAML::Node repoNode = YAML::Load(res->body);
                outInfo.url = repoUrl;
                outInfo.name = repoNode["name"].as<std::string>();
                outInfo.description = repoNode["description"].as<std::string>();
                outInfo.mantainer = repoNode["mantainer"].as<std::string>();
                delete cli;
                std::cout << "\033[1;32m" << res->status << "\033[0m" << std::endl;
                return true;
            }
            else
            {
                std::cout << "\033[1;31m" << (res ? std::to_string(res->status) : "no response") << "\033[0m" << std::endl;
                delete cli;
                return false;
            }
        }
    }
    bool getRepositoryInfo(const std::string &repoUrl, RepositoryInfo &outInfo)
    {
        Config *config = getConfig();
        Archive *dataArchive = getDataArchive();
        std::string reposFileContent;
        int status = dataArchive->readFile("repositories.yaml", reposFileContent);
        if (status == 0)
        {
            YAML::Node reposNode = YAML::Load(reposFileContent);
            if (!reposNode[repoUrl])
            {
                bool fetchStatus = fetchRepositoryInfo(repoUrl, outInfo);
                return fetchStatus;
            }
            YAML::Node repoNode = reposNode[repoUrl];
            outInfo.url = repoUrl;
            outInfo.name = repoNode["name"].as<std::string>();
            outInfo.description = repoNode["description"].as<std::string>();
            outInfo.mantainer = repoNode["mantainer"].as<std::string>();
            return true;
        }
        else
        {
            bool fetchStatus = fetchRepositoryInfo(repoUrl, outInfo);
            return fetchStatus;
        }
    }
    bool validateRepositoryInfo(const RepositoryInfo &repoInfo)
    {
        if (repoInfo.url.empty() || repoInfo.name.empty() || repoInfo.description.empty() || repoInfo.mantainer.empty())
        {
            return false;
        }
        return true;
    }
    int updateAllRepositories()
    {
        Config *config = getConfig();
        Archive *dataArchive = getDataArchive();
        std::string reposFileContent;
        int status = dataArchive->readFile("repositories.yaml", reposFileContent);
        if (status != 0)
        {
            error("\033[1;34mNo repositories found.");
            return 1;
        }
        YAML::Node reposNode = YAML::Load(reposFileContent);
        for (const auto &it : reposNode)
        {
            std::string repoUrl = it.first.as<std::string>();
            RepositoryInfo repoInfo;
            bool fetchStatus = fetchRepositoryInfo(repoUrl, repoInfo);
            if (!fetchStatus)
            {
                error("\033[1;31mFailed to fetch repository info: " + repoUrl);
                continue;
            }
            YAML::Node repoNode;
            repoNode["name"] = repoInfo.name;
            repoNode["description"] = repoInfo.description;
            repoNode["mantainer"] = repoInfo.mantainer;
            reposNode[repoUrl] = repoNode;
        }
        return 0;
    }
    bool addRepository(RepositoryInfo repoInfo)
    {
        bool valid = validateRepositoryInfo(repoInfo);
        if (!valid)
        {
            error("\033[1;31mInvalid repository information provided.");
            return false;
        }
        Config *config = getConfig();
        Archive *dataArchive = getDataArchive();
        std::string reposFileContent;
        int status = dataArchive->readFile("repositories.yaml", reposFileContent);
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
            if (it.first.as<std::string>() == repoInfo.url)
            {
                warn("\033[1;33mRepository already exists: " + repoInfo.url);
                return false;
            }
        }
        YAML::Node repoNode;
        repoNode["name"] = repoInfo.name;
        repoNode["description"] = repoInfo.description;
        repoNode["mantainer"] = repoInfo.mantainer;
        reposNode[repoInfo.url] = repoNode;
        std::stringstream ss;
        ss << reposNode;
        std::string data = ss.str();
        status = dataArchive->writeFile("repositories.yaml", data);
        if (status != 0)
        {
            error("\033[1;31mFailed to add repository: " + repoInfo.url);
            return false;
        }
        return true;
    }
    bool removeRepository(RepositoryInfo repoInfo)
    {
        Config *config = getConfig();
        Archive *dataArchive = getDataArchive();
        std::string reposFileContent;
        int status = dataArchive->readFile("repositories.yaml", reposFileContent);
        if (status != 0)
        {
            error("\033[1;31mNo repositories found.");
            return false;
        }
        YAML::Node reposNode = YAML::Load(reposFileContent);
        bool found = false;
        for (auto it = reposNode.begin(); it != reposNode.end(); ++it)
        {
            if (it->first.as<std::string>() == repoInfo.url)
            {
                reposNode.remove(it->first);
                found = true;
                break;
            }
        }
        if (!found)
        {
            warn("\033[1;33mRepository not found: " + repoInfo.url);
            return false;
        }
        std::stringstream ss;
        ss << reposNode;
        std::string data = ss.str();
        status = dataArchive->writeFile("repositories.yaml", data);
        if (status != 0)
        {
            error("\033[1;31mFailed to remove repository: " + repoInfo.url);
            return false;
        }
        return true;
    }
    bool verifyRepository(const std::string &repoUrl)
    {
        Config *config = getConfig();
        Archive *dataArchive = getDataArchive();
        std::string reposFileContent;
        int status = dataArchive->readFile("repositories.yaml", reposFileContent);
        if (status != 0)
        {
            error("\033[1;31mNo repositories found.");
            return false;
        }
        YAML::Node reposNode = YAML::Load(reposFileContent);
        if (!reposNode[repoUrl])
        {
            warn("\033[1;33mRepository not found: " + repoUrl);
            return false;
        }
        log("Repository verification not implemented yet");
        return true;
    }
} // namespace openspm