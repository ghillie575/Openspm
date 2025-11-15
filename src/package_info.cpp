#include <package_info.h>
#include <yaml-cpp/yaml.h>
#include <sstream>
#include <stdexcept>
#include <cstring>
static void checkPackageFields(const PackageInfo &p)
{
    if (p.package.empty())
        throw std::runtime_error("Missing field: package");
    if (p.version.empty())
        throw std::runtime_error("Missing field: version");
    if (p.description.empty())
        throw std::runtime_error("Missing field: description");
    if (p.req.empty())
        throw std::runtime_error("Missing field: req");
    if (p.path.empty())
        throw std::runtime_error("Missing field: path");
    if (p.checksum.empty())
        throw std::runtime_error("Missing field: checksum");
}

std::string serializeRepository(const std::vector<PackageInfo> &packages)
{
    YAML::Emitter out;

    out << YAML::BeginMap;
    out << YAML::Key << "packages" << YAML::Value;
    out << YAML::BeginSeq;

    for (const auto &p : packages)
    {
        // Validate all fields are present
        checkPackageFields(p);

        out << YAML::BeginMap;
        out << YAML::Key << "package" << YAML::Value << p.package;
        out << YAML::Key << "version" << YAML::Value << p.version;
        out << YAML::Key << "description" << YAML::Value << p.description;
        out << YAML::Key << "req" << YAML::Value << p.req;
        out << YAML::Key << "path" << YAML::Value << p.path;
        out << YAML::Key << "checksum" << YAML::Value << p.checksum;
        out << YAML::EndMap;
    }

    out << YAML::EndSeq;
    out << YAML::EndMap;

    return out.c_str();
}
// ------------------------
// Deserialization (Parser)
// ------------------------
std::vector<PackageInfo> deserializeRepository(const std::string &yamlString)
{

    std::vector<PackageInfo> packages;

    YAML::Node root = YAML::Load(yamlString);

    if (!root["packages"] || !root["packages"].IsSequence())
    {
        throw std::runtime_error("YAML does not contain a 'packages' sequence");
    }

    for (const auto &node : root["packages"])
    {
        PackageInfo p;

        if (node["package"])
            p.package = node["package"].as<std::string>();
        if (node["version"])
            p.version = node["version"].as<std::string>();
        if (node["description"])
            p.description = node["description"].as<std::string>();
        if (node["req"])
            p.req = node["req"].as<std::string>();
        if (node["path"])
            p.path = node["path"].as<std::string>();
        if (node["checksum"])
            p.checksum = node["checksum"].as<std::string>();

        packages.push_back(p);
    }

    return packages;
}