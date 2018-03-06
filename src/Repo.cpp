#include "Repo.hpp"
#include "Utils.hpp"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "constants.h"
#include <sstream>
#include <stdarg.h>		/* va_list, va_start, va_arg, va_end */

using namespace rapidjson;

Repo::Repo()
{
	
}

Repo::Repo(const char* name, const char* url)
{
	// create a repo from the passed parameters
	this->name = name;
	this->url = url;
	this->enabled = true;
}

std::string Repo::toJson()
{
	std::stringstream resp;
	resp << "\t\t{\n\t\t\t\"name\": \"" << this->name << "\",\n\t\t\t\"url\": \"" << this->url << "\",\n\t\t\t\"enabled\": " << (this->enabled? "true" : "false") << "\n\t\t}\n";
	return resp.str();
}

std::string generateRepoJson(int count, ...)
{
	va_list ap;
	
	std::stringstream response;
	response << "{\n\t\"repos\": [\n";
	
	va_start(ap, count);
	
	for (int x=0; x<count; x++)
		response << (va_arg(ap, Repo*))->toJson();
		
	va_end(ap);
	response << "\t]\n}\n";
			
	return response.str();
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
	
	std::string* response_copy = new std::string(response);

	if (!success)
	{
		std::cout << "--> Could not update repository metadata for \"" << this->name << "\" repo!" << std::endl;
		return;
	}
	
	// extract out packages, append to package list
	Document doc;
	doc.Parse(response_copy->c_str());
	
	if (!doc.IsObject() || !doc.HasMember("packages"))
	{
		std::cout << "--> Invalid format in downloaded repo.json for " << this->url << std::endl;
		return;
	}
	
	const Value& packages_doc = doc["packages"];

	// for every repo
	for(Value::ConstValueIterator it=packages_doc.Begin(); it != packages_doc.End(); it++)
	{
		Package* package = new Package(GET);
		package->pkg_name = (*it)["name"].GetString();
		if ((*it).HasMember("title"))
			package->title = (*it)["title"].GetString();
		else
			package->title = package->pkg_name;
		if ((*it).HasMember("author"))
			package->author = (*it)["author"].GetString();
		if ((*it).HasMember("description"))
			package->short_desc = (*it)["description"].GetString();
		if ((*it).HasMember("version"))
			package->version = (*it)["version"].GetString();
		package->repoUrl = &this->url;

		// save the response string to cleanup later
		package->contents = response_copy;

		packages->push_back(package);
	}
}
