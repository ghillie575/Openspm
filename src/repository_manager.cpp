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
        debug("[DEBUG getRepositoryList] Getting repository list");
        Config *config = getConfig();
        std::vector<std::string> repoList;
        Archive *dataArchive = getDataArchive();
        debug("[DEBUG getRepositoryList] Archive pointer: " + std::to_string((long)dataArchive));
        
        std::string reposFileContent;
        debug("[DEBUG getRepositoryList] Reading repositories.yaml");
        int status = dataArchive->readFile("repositories.yaml", reposFileContent);
        if (status != 0)
        {
            debug("[DEBUG getRepositoryList] No repositories file found");
            warn("No repositories found.");
        }
        else
        {
            debug("[DEBUG getRepositoryList] Repositories file size: " + std::to_string(reposFileContent.size()) + " bytes");
            debug("[DEBUG getRepositoryList] Parsing YAML");
            YAML::Node reposNode = YAML::Load(reposFileContent);
            for (const auto &it : reposNode)
            {
                std::string url = it.first.as<std::string>();
                debug("[DEBUG getRepositoryList] Found repository: " + url);
                repoList.push_back(url);
            }
        }
        debug("[DEBUG getRepositoryList] Total repositories: " + std::to_string(repoList.size()));
        return repoList;
    }
    
    bool fetchRepositoryInfo(const std::string &repoUrl, RepositoryInfo &outInfo)
    {
        debug("[DEBUG fetchRepositoryInfo] Fetching info for: " + repoUrl);
        auto parsed = parse_url(repoUrl);
        debug("[DEBUG fetchRepositoryInfo] Parsed URL - scheme: " + parsed.scheme + ", host: " + parsed.host + ", path: " + parsed.path);

        httplib::Client *cli = nullptr;
        httplib::SSLClient *sslCli = nullptr;
        bool useSSL = false;
        if (parsed.scheme == "https")
        {
            debug("[DEBUG fetchRepositoryInfo] Using HTTPS");
            if (parsed.port > 0)
                sslCli = new httplib::SSLClient(parsed.host, parsed.port);
            else
                sslCli = new httplib::SSLClient(parsed.host);
            useSSL = true;
        }
        else
        {
            debug("[DEBUG fetchRepositoryInfo] Using HTTP");
            if (parsed.port > 0)
                cli = new httplib::Client(parsed.host, parsed.port);
            else
                cli = new httplib::Client(parsed.host);
            warn("W: Repository URL is not using HTTPS: " + repoUrl);
        }
        
        std::string fullUrl = parsed.scheme + "://" + parsed.host;
        if (parsed.port > 0)
        {
            fullUrl += ":" + std::to_string(parsed.port);
        }
        fullUrl += parsed.path + "/repository.yaml";

        if (useSSL)
        {
            debug("[DEBUG fetchRepositoryInfo] Making HTTPS request");
            auto res = sslCli->Get((parsed.path + "/repository.yaml").c_str());
            if (res && res->status == 200)
            {
                logHttpRequest("GET", fullUrl, res->status);
                debug("[DEBUG fetchRepositoryInfo] Request successful, response size: " + std::to_string(res->body.size()) + " bytes");
                YAML::Node repoNode = YAML::Load(res->body);
                outInfo.url = repoUrl;
                outInfo.name = repoNode["name"].as<std::string>();
                outInfo.description = repoNode["description"].as<std::string>();
                outInfo.mantainer = repoNode["mantainer"].as<std::string>();
                debug("[DEBUG fetchRepositoryInfo] Repository name: " + outInfo.name);
                debug("[DEBUG fetchRepositoryInfo] Maintainer: " + outInfo.mantainer);
                delete sslCli;
                return true;
            }
            else
            {
                logHttpRequest("GET", fullUrl, res ? res->status : 0);
                debug("[DEBUG fetchRepositoryInfo] Request failed");
                delete sslCli;
                return false;
            }
        }
        else
        {
            debug("[DEBUG fetchRepositoryInfo] Making HTTP request");
            auto res = cli->Get((parsed.path + "/repository.yaml").c_str());
            if (res && res->status == 200)
            {
                logHttpRequest("GET", fullUrl, res->status);
                debug("[DEBUG fetchRepositoryInfo] Request successful, response size: " + std::to_string(res->body.size()) + " bytes");
                YAML::Node repoNode = YAML::Load(res->body);
                outInfo.url = repoUrl;
                outInfo.name = repoNode["name"].as<std::string>();
                outInfo.description = repoNode["description"].as<std::string>();
                outInfo.mantainer = repoNode["mantainer"].as<std::string>();
                debug("[DEBUG fetchRepositoryInfo] Repository name: " + outInfo.name);
                debug("[DEBUG fetchRepositoryInfo] Maintainer: " + outInfo.mantainer);
                delete cli;
                return true;
            }
            else
            {
                logHttpRequest("GET", fullUrl, res ? res->status : 0);
                debug("[DEBUG fetchRepositoryInfo] Request failed");
                delete cli;
                return false;
            }
        }
    }
    
    bool getRepositoryInfo(const std::string &repoUrl, RepositoryInfo &outInfo)
    {
        debug("[DEBUG getRepositoryInfo] Getting info for: " + repoUrl);
        Config *config = getConfig();
        Archive *dataArchive = getDataArchive();
        std::string reposFileContent;
        debug("[DEBUG getRepositoryInfo] Reading repositories.yaml");
        int status = dataArchive->readFile("repositories.yaml", reposFileContent);
        if (status == 0)
        {
            debug("[DEBUG getRepositoryInfo] Repositories file found, checking cache");
            YAML::Node reposNode = YAML::Load(reposFileContent);
            if (!reposNode[repoUrl])
            {
                debug("[DEBUG getRepositoryInfo] Not in cache, fetching from network");
                bool fetchStatus = fetchRepositoryInfo(repoUrl, outInfo);
                return fetchStatus;
            }
            debug("[DEBUG getRepositoryInfo] Found in cache");
            YAML::Node repoNode = reposNode[repoUrl];
            outInfo.url = repoUrl;
            outInfo.name = repoNode["name"].as<std::string>();
            outInfo.description = repoNode["description"].as<std::string>();
            outInfo.mantainer = repoNode["mantainer"].as<std::string>();
            debug("[DEBUG getRepositoryInfo] Retrieved from cache: " + outInfo.name);
            return true;
        }
        else
        {
            debug("[DEBUG getRepositoryInfo] No cache file, fetching from network");
            bool fetchStatus = fetchRepositoryInfo(repoUrl, outInfo);
            return fetchStatus;
        }
    }
    
    bool validateRepositoryInfo(const RepositoryInfo &repoInfo)
    {
        debug("[DEBUG validateRepositoryInfo] Validating repository info");
        if (repoInfo.url.empty() || repoInfo.name.empty() || repoInfo.description.empty() || repoInfo.mantainer.empty())
        {
            debug("[DEBUG validateRepositoryInfo] Validation failed - missing fields");
            return false;
        }
        debug("[DEBUG validateRepositoryInfo] Validation passed");
        return true;
    }
    
    int updateAllRepositories()
    {
        debug("[DEBUG updateAllRepositories] Updating all repositories");
        Config *config = getConfig();
        Archive *dataArchive = getDataArchive();
        std::string reposFileContent;
        debug("[DEBUG updateAllRepositories] Reading repositories.yaml");
        int status = dataArchive->readFile("repositories.yaml", reposFileContent);
        if (status != 0)
        {
            error("No repositories found.");
            return 1;
        }
        debug("[DEBUG updateAllRepositories] Repositories file size: " + std::to_string(reposFileContent.size()) + " bytes");
        YAML::Node reposNode = YAML::Load(reposFileContent);
        debug("[DEBUG updateAllRepositories] Processing " + std::to_string(reposNode.size()) + " repositories");
        size_t repoIndex = 0;
        for (const auto &it : reposNode)
        {
            std::string repoUrl = it.first.as<std::string>();
            debug("[DEBUG updateAllRepositories] Updating repository: " + repoUrl);
            RepositoryInfo repoInfo;
            bool fetchStatus = fetchRepositoryInfo(repoUrl, repoInfo);
            if (!fetchStatus)
            {
                error("Failed to fetch repository info: " + repoUrl);
                return 1;
            }
            debug("[DEBUG updateAllRepositories] Fetched info for: " + repoInfo.name);
            YAML::Node repoNode;
            repoNode["name"] = repoInfo.name;
            repoNode["description"] = repoInfo.description;
            repoNode["mantainer"] = repoInfo.mantainer;
            reposNode[repoUrl] = repoNode;
            repoIndex++;
        }
        debug("[DEBUG updateAllRepositories] All repositories updated successfully");
        log("\033[1;32mSuccessfully updated all repositories");
        return 0;
    }
    
    bool addRepository(RepositoryInfo repoInfo)
    {
        debug("[DEBUG addRepository] Adding repository: " + repoInfo.url);
        bool valid = validateRepositoryInfo(repoInfo);
        if (!valid)
        {
            error("Invalid repository information provided.");
            return false;
        }
        Config *config = getConfig();
        Archive *dataArchive = getDataArchive();
        std::string reposFileContent;
        debug("[DEBUG addRepository] Reading repositories.yaml");
        int status = dataArchive->readFile("repositories.yaml", reposFileContent);
        YAML::Node reposNode;
        if (status == 0)
        {
            debug("[DEBUG addRepository] Repositories file found, loading");
            reposNode = YAML::Load(reposFileContent);
        }
        else
        {
            debug("[DEBUG addRepository] No repositories file, creating new");
            reposNode = YAML::Node(YAML::NodeType::Map);
        }
        for (const auto &it : reposNode)
        {
            if (it.first.as<std::string>() == repoInfo.url)
            {
                warn("Repository already exists: " + repoInfo.url);
                return false;
            }
        }
        debug("[DEBUG addRepository] Creating repository node");
        YAML::Node repoNode;
        repoNode["name"] = repoInfo.name;
        repoNode["description"] = repoInfo.description;
        repoNode["mantainer"] = repoInfo.mantainer;
        reposNode[repoInfo.url] = repoNode;
        
        debug("[DEBUG addRepository] Converting to YAML string");
        std::stringstream ss;
        ss << reposNode;
        std::string data = ss.str();
        debug("[DEBUG addRepository] YAML data size: " + std::to_string(data.size()) + " bytes");
        
        debug("[DEBUG addRepository] Writing to archive");
        status = dataArchive->writeFile("repositories.yaml", data);
        if (status != 0)
        {
            error("Failed to add repository: " + repoInfo.url);
            return false;
        }
        debug("[DEBUG addRepository] Repository added successfully");
        return true;
    }
    
    bool removeRepository(RepositoryInfo repoInfo)
    {
        debug("[DEBUG removeRepository] Removing repository: " + repoInfo.url);
        Config *config = getConfig();
        Archive *dataArchive = getDataArchive();
        std::string reposFileContent;
        debug("[DEBUG removeRepository] Reading repositories.yaml");
        int status = dataArchive->readFile("repositories.yaml", reposFileContent);
        if (status != 0)
        {
            error("No repositories found.");
            return false;
        }
        debug("[DEBUG removeRepository] Repositories file size: " + std::to_string(reposFileContent.size()) + " bytes");
        YAML::Node reposNode = YAML::Load(reposFileContent);
        bool found = false;
        for (auto it = reposNode.begin(); it != reposNode.end(); ++it)
        {
            if (it->first.as<std::string>() == repoInfo.url)
            {
                debug("[DEBUG removeRepository] Found repository, removing");
                reposNode.remove(it->first);
                found = true;
                break;
            }
        }
        if (!found)
        {
            warn("Repository not found: " + repoInfo.url);
            return false;
        }
        debug("[DEBUG removeRepository] Converting updated list to YAML");
        std::stringstream ss;
        ss << reposNode;
        std::string data = ss.str();
        debug("[DEBUG removeRepository] Writing updated list to archive");
        status = dataArchive->writeFile("repositories.yaml", data);
        if (status != 0)
        {
            error("Failed to remove repository: " + repoInfo.url);
            return false;
        }
        debug("[DEBUG removeRepository] Repository removed successfully");
        return true;
    }
    
    bool verifyRepository(const std::string &repoUrl)
    {
        debug("[DEBUG verifyRepository] Verifying repository: " + repoUrl);
        Config *config = getConfig();
        Archive *dataArchive = getDataArchive();
        std::string reposFileContent;
        int status = dataArchive->readFile("repositories.yaml", reposFileContent);
        if (status != 0)
        {
            error("No repositories found.");
            return false;
        }
        YAML::Node reposNode = YAML::Load(reposFileContent);
        if (!reposNode[repoUrl])
        {
            warn("Repository not found: " + repoUrl);
            return false;
        }
        log("Repository verification not implemented yet");
        return true;
    }
} // namespace openspm