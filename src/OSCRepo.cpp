#include "OSCRepo.hpp"
#include "Utils.hpp"
#include "constants.h"
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include <time.h>
#include <iomanip>
#include <regex>
#include <sstream>
#include <stdarg.h> /* va_list, va_start, va_arg, va_end */

using namespace rapidjson;

#ifdef WIN32
// https://stackoverflow.com/a/33542189
extern "C" char* strptime(const char* s,
                          const char* f,
                          struct tm* tm) {
  std::istringstream input(s);
  input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
  input >> std::get_time(tm, f);
  if (input.fail()) {
    return nullptr;
  }
  return (char*)(s + input.tellg());
}
#endif

void OSCRepo::loadPackages(Get* get, std::vector<Package*>* packages)
{
	std::string directoryUrl = this->url + "/api/v3/contents";

	std::string response;
	bool success = downloadFileToMemory(directoryUrl, &response);

	// attempt fallback to http in case of https repos failure
    // TODO: Abstract this out, and allow to be optional
	if (!success && (this->url.rfind("https", 0) == 0))
	{
		printf("--> Attempting http fallback for https repo \"%s\" after loading failure\n", this->name.c_str());

		// update repo url
		this->url.replace(0, 5, "http");
		directoryUrl = this->url + "/api/v3/contents";

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

	if (!ok || !doc.IsArray())
	{
		printf("--> Invalid format in downloaded repo.json for %s\n", this->url.c_str());
		this->loaded = false;
		return;
	}
    
    const Value& packages_doc = doc.GetArray();

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
        if (cur.HasMember("slug")) {
            package->pkg_name = cur["slug"].GetString();
        } else {
            printf("Missing slug for package on repo, skipping\n");
            continue;
        }
        if (cur.HasMember("name")) {
            package->title = cur["name"].GetString();
        } else {
            package->title = package->pkg_name;
        }
        // printf("The name is %s\n", package->pkg_name.c_str());

		if (cur.HasMember("author"))
			package->author = cur["author"].GetString();
        // printf("The author is %s\n", package->author.c_str());

		if (cur.HasMember("description")) {
			auto desc = cur["description"].GetObject();
			if (desc.HasMember("short") && desc["short"].IsString())
				package->short_desc = desc["short"].GetString();
			if (desc.HasMember("long") && desc["long"].IsString())
				package->long_desc = std::regex_replace(desc["long"].GetString(), std::regex("\\\\n"), "\n");
		}
        // printf("The short description is %s\n", package->short_desc.c_str());
        // printf("The long description is %s\n", package->long_desc.c_str());

		if (cur.HasMember("version"))
			package->version = cur["version"].GetString();
        
        // printf("The version is %s\n", package->version.c_str());
		
		if (cur.HasMember("release_date"))
		{
			auto unixTimestamp = cur["release_date"].GetInt64();
            time_t t = unixTimestamp;
            struct tm tm;
            char buf[80];
            tm = *localtime(&t);
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
            package->updated = buf;
            package->updated_timestamp = unixTimestamp;
		}

        // printf("The updated is %s\n", package->updated.c_str());
        // printf("The updated timestamp is %d\n", package->updated_timestamp);

		// even more details

		if (cur.HasMember("file_size")) {
			auto fileSize = cur["file_size"].GetObject();
			if (fileSize.HasMember("zip_compressed"))
				package->download_size += fileSize["zip_compressed"].GetInt();
			if (fileSize.HasMember("zip_uncompressed"))
				package->extracted_size += fileSize["zip_uncompressed"].GetInt();
		}

        // printf("The download size is %d\n", package->download_size);
        // printf("The extracted size is %d\n", package->extracted_size);

		if (cur.HasMember("category"))
			package->category = cur["category"].GetString();
        
        if (cur.HasMember("url")) {
            auto url = cur["url"].GetObject();
            if (url.HasMember("zip"))
                package->url = url["zip"].GetString();
			if (url.HasMember("icon"))
				package->iconUrl = url["icon"].GetString();
        }
        
        // printf("The category is %s\n", package->category.c_str());

		package->repoUrl = &this->url;
        package->parentRepo = this;

		// save the response string to cleanup later
		package->contents = response_copy;

		packages->push_back(package);
	}
}

std::string OSCRepo::getType()
{
	return "osc";
}

std::string OSCRepo::getZipUrl(Package* package) {
    // OSC packages just use the url field directly
	return package->url;
}

std::string OSCRepo::getIconUrl(Package* package) {
    // OSC packages just use the url field directly
	return package->iconUrl;
}