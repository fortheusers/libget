#include "Repo.hpp"
#include "util.hpp"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

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
    downloadFileToMemory(directoryUrl, &response);
    
    // extract out packages, append to package list
    Document doc;
    doc.Parse(response.c_str());
	
	const Value& packages_doc = doc["packages"];
	
	// for every repo
	for(Value::ConstValueIterator it=packages_doc.Begin(); it != packages_doc.End(); it++)
	{
		Package* package = new Package();
		package->pkg_name = (*it)["name"].GetString();
		package->title = (*it)["title"].GetString();
		package->author = (*it)["author"].GetString();
        package->short_desc = (*it)["desc"].GetString();
        package->version = (*it)["version"].GetString();
        
		packages->push_back(package);
	}
}