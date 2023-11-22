#include "LocalRepo.hpp"
#include "Package.hpp"
#include "Get.hpp"
#include "Utils.hpp"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include <dirent.h>
#include <cstring>

using namespace std;

std::string dummyUrl = "";
std::string dummyContents = "";

void LocalRepo::loadPackages(Get* get, std::vector<Package*>* packages) {
    // go through each folder in the .get/packages directory, and load their info
    DIR* dir;
    struct dirent* ent;
    std::string jsonPathInternal = "info.json";

    if ((dir = opendir(get->pkg_path)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            // skip . and ..
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            if (is_dir(get->pkg_path, ent))
            {
                string* pkg_dir = new string(get->pkg_path);
                pkg_dir->append(ent->d_name);
                pkg_dir->append("/");

                Package* pkg = new Package(LOCAL);
                pkg->pkg_name = ent->d_name;

                std::string jsonPath = get->pkg_path + pkg->pkg_name + "/" + jsonPathInternal;

                // grab the info json
                struct stat sbuff;
                if (stat(jsonPath.c_str(), &sbuff) == 0)
                {
                    // pull out the version number and check if it's
                    // different than the one on the repo
                    std::ifstream ifs(jsonPath.c_str());
                    rapidjson::IStreamWrapper isw(ifs);

                    if (!ifs.good())
                    {
                        printf("--> Could not locate %s for local package folder", jsonPath.c_str());
                        // so, we have a folder for this file, but no info.json
                        // we can't in good faith do anything with this!
                        continue;
                    }

                    rapidjson::Document doc;
                    rapidjson::ParseResult ok = doc.ParseStream(isw);
                    if (!ok) {
                        printf("--> Could not parse %s for local package folder", jsonPath.c_str());
                        continue;
                    }

                    // try to load as much info as possible from the info json
                    // (normally, a get repo's package only uses the verison)

                    if (doc.HasMember("title")) {
                        pkg->title = doc["title"].GetString();
                    }
                    if (doc.HasMember("version")) {
                        pkg->version = doc["version"].GetString();
                    }
                    if (doc.HasMember("author")) {
                        pkg->author = doc["author"].GetString();
                    }
                    if (doc.HasMember("license")) {
                        pkg->license = doc["license"].GetString();
                    }
                    if (doc.HasMember("details")) {
                        pkg->short_desc = doc["description"].GetString();
                    }
                    if (doc.HasMember("description")) {
                        pkg->long_desc = doc["details"].GetString();
                    }
                    if (doc.HasMember("changelog")) {
                        pkg->changelog = doc["changelog"].GetString();
                    }
                }

                pkg->repoUrl = &dummyUrl;
                pkg->contents = &dummyContents;
                pkg->status = LOCAL;

                // // TODO: load some other info from the json
                packages->push_back(pkg);
            }
        }
        closedir(dir);
    }
    else
    {
        printf("--> Could not open packages directory\n");
    }
}

std::string LocalRepo::getType() {
    return "local";
}

std::string LocalRepo::getZipUrl(Package* package) {
    // Local packages don't have a zip url
    return "";
}

std::string LocalRepo::getIconUrl(Package* package) {
    // ditto and/or ibid
    return "";
}