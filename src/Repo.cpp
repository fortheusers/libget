#include "Repo.hpp"
#include "Utils.hpp"
#include "constants.h"
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include <regex>
#include <sstream>
#include <stdarg.h> /* va_list, va_start, va_arg, va_end */

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
	resp << "\t\t{\n\t\t\t\"name\": \"" << this->name << "\",\n\t\t\t\"url\": \"" << this->url << "\",\n\t\t\t\"enabled\": " << (this->enabled ? "true" : "false") << "\n\t\t}\n";
	return resp.str();
}

std::string generateRepoJson(int count, ...)
{
	va_list ap;

	std::stringstream response;
	response << "{\n\t\"repos\": [\n";

	va_start(ap, count);

	for (int x = 0; x < count; x++)
		response << (va_arg(ap, Repo*))->toJson();

	va_end(ap);
	response << "\t]\n}\n";

	return response.str();
}

std::string Repo::toString()
{
	return "[" + this->name + "] <" + this->url + "> - " + ((this->enabled) ? "enabled" : "disabled");
}

void Repo::loadPackages(std::vector<Package*>* packages)
{
	std::string directoryUrl = this->url + "/repo.json";

	// fetch current repository json
	std::string response;
	bool success = downloadFileToMemory(directoryUrl, &response);

#ifdef NETWORK_MOCK
  mockPopulatePackages(&response);
#endif

	// attempt fallback to http in case of https repos failure
	if (!success && (this->url.rfind("https", 0) == 0))
	{
		printf("--> Attempting http fallback for https repo \"%s\" after loading failure\n", this->name.c_str());

		// update repo url
		this->url.replace(0, 5, "http");
		directoryUrl = this->url + "/repo.json";

		// retry fetch
		success = downloadFileToMemory(directoryUrl, &response);
	}

	if (!success)
	{
		printf("--> Could not update repository metadata for \"%s\" repo!\n", this->name.c_str());
		this->loaded = false;
		return;
	}

	if (libget_status_callback != NULL)
		libget_status_callback(STATUS_UPDATING_STATUS, 1, 1);

	std::string* response_copy = new std::string(response);

	// extract out packages, append to package list
	Document doc;
	ParseResult ok = doc.Parse(response_copy->c_str());

	if (!ok || !doc.IsObject() || !doc.HasMember("packages"))
	{
		printf("--> Invalid format in downloaded repo.json for %s\n", this->url.c_str());
		this->loaded = false;
		return;
	}

	const Value& packages_doc = doc["packages"];

	// for every repo
  auto total = packages_doc.Size();
	for (int i = 0; i < total; i++)
	{
		if (networking_callback != NULL)
			networking_callback(0, total, i+1, 0, 0);

		Package* package = new Package(GET);

		// TODO: use arrays and loops for parsing this info, and also check the type first

    auto& cur = packages_doc[i];

		// mostly essential attributes
		package->pkg_name = cur["name"].GetString();
		if (cur.HasMember("title"))
			package->title = cur["title"].GetString();
		else
			package->title = package->pkg_name;
		if (cur.HasMember("author"))
			package->author = cur["author"].GetString();
		if (cur.HasMember("description"))
			package->short_desc = cur["description"].GetString();
		if (cur.HasMember("details"))
			package->long_desc = std::regex_replace(cur["details"].GetString(), std::regex("\\\\n"), "\n");
		if (cur.HasMember("version"))
			package->version = cur["version"].GetString();

		// more information and details
		if (cur.HasMember("license"))
			package->license = cur["license"].GetString();
		if (cur.HasMember("changelog"))
			package->changelog = std::regex_replace(cur["changelog"].GetString(), std::regex("\\\\n"), "\n");
		if (cur.HasMember("url"))
			package->url = cur["url"].GetString();
		if (cur.HasMember("updated"))
		{
			package->updated = cur["updated"].GetString();
			struct tm tm;

#if !defined(_3DS) && !defined(WII)
			auto res = strptime(package->updated.c_str(), "%d/%m/%Y", &tm);
			if (res)
			{
				// make sure that all the time-related fields are set
				tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
				package->updated_timestamp = (int)mktime(&tm);
			}
#endif
		}

		// even more details
		if (cur.HasMember("app_dls"))
			package->downloads += cur["app_dls"].GetInt();
		if (cur.HasMember("web_dls"))
			package->downloads += cur["web_dls"].GetInt();
		if (cur.HasMember("extracted"))
			package->extracted_size += cur["extracted"].GetInt();
		if (cur.HasMember("filesize"))
			package->download_size += cur["filesize"].GetInt();

		if (cur.HasMember("category"))
			package->category = cur["category"].GetString();
		if (cur.HasMember("binary"))
			package->binary = cur["binary"].GetString();
		if (cur.HasMember("screens"))
			package->screens = cur["screens"].GetInt();

		package->repoUrl = &this->url;

		// save the response string to cleanup later
		package->contents = response_copy;

		packages->push_back(package);
	}
}
