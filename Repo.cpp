#include "Repo.hpp"
#include "util.hpp"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "codes.h"

using namespace rapidjson;

Repo::Repo()
{
	
}

std::string Repo::toString()
{
	return "[" + this->name + "] <" + this->url + "> - " + ((this->enabled)? "enabled" : "disabled");
}

void Repo::loadPackages(std::vector<Package*>* packages)
{
    std::string directoryUrl = this->url + "/repo.json";
    
    // fetch current repository json
    std::string response;
    bool success = downloadFileToMemory(directoryUrl, &response);
    
    if (!success)
    {
        std::cout << "--> Could not update repository metadata for \"" << this->name << "\" repo!" << std::endl;
        return;
    }
    
    // extract out packages, append to package list
    Document doc;
    doc.Parse(response.c_str());
	
	const Value& packages_doc = doc["packages"];
	
	// for every repo
	for(Value::ConstValueIterator it=packages_doc.Begin(); it != packages_doc.End(); it++)
	{
		Package* package = new Package(GET);
		package->pkg_name = (*it)["name"].GetString();
        if ((*it).HasMember("title"))
            package->title = (*it)["title"].GetString();
        if ((*it).HasMember("author"))
            package->author = (*it)["author"].GetString();
        if ((*it).HasMember("description"))
            package->short_desc = (*it)["description"].GetString();
        if ((*it).HasMember("version"))
            package->version = (*it)["version"].GetString();
        package->repoUrl = &this->url;
        
		packages->push_back(package);
	}
}
