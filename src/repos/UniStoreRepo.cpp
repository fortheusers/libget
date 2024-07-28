#include "UniStoreRepo.hpp"
#include "../Utils.hpp"
#include "constants.h"
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include <iomanip>
#include <regex>
#include <sstream>
#include <map>

using namespace rapidjson;

std::vector<std::unique_ptr<Package>> UniStoreRepo::loadPackages()
{
	std::vector<std::unique_ptr<Package>> result;
	std::string directoryUrl = this->url;

	std::string response;
	bool success = downloadFileToMemory(directoryUrl, &response);

	// attempt fallback to http in case of https repos failure
	// TODO: Abstract this out, and allow to be optional
	if (!success && (this->url.rfind("https", 0) == 0))
	{
		printf("--> Attempting http fallback for https repo \"%s\" after loading failure\n", this->name.c_str());

		// update repo url
		this->url.replace(0, 5, "http");
		directoryUrl = this->url;

		// retry fetch
		success = downloadFileToMemory(directoryUrl, &response);
	}

	if (!success)
	{
		printf("--> Could not update repository metadata for \"%s\" repo!\n", this->name.c_str());
		this->loaded = false;
		return {};
	}

	if (libget_status_callback != nullptr)
	{
		libget_status_callback(STATUS_UPDATING_STATUS, 1, 1);
	}

	// extract out packages, append to package list
	Document doc;
	ParseResult ok = doc.Parse(response.c_str());

	if (!ok || !doc.IsObject() || !doc.HasMember("storeContent"))
	{
		printf("--> Invalid format in downloaded data for %s\n", this->url.c_str());
		this->loaded = false;
		return {};
	}

	const Value& packages_doc = doc["storeContent"];

	// for every package in the repo
	auto total = (int32_t)packages_doc.Size();
	for (int i = 0; i < total; i++)
	{
		if (networking_callback != nullptr)
			networking_callback(nullptr, total, i + 1, 0, 0);
		
		std::vector<std::string> keys;
		auto start = packages_doc[i].GetObject().MemberBegin();
		auto end = packages_doc[i].GetObject().MemberEnd();
		for (auto it = start; it != end; ++it) {
			keys.push_back(it->name.GetString());
		}

		if (packages_doc[i].HasMember("info"))
		{
			auto package = std::make_unique<Package>(GET);
			auto& cur = packages_doc[i]["info"];

			if (cur.HasMember("title")) {
				package->pkg_name = cur["title"].GetString();
				package->title = package->pkg_name;
			}
			if (cur.HasMember("author"))
				package->author = cur["author"].GetString();
			if (cur.HasMember("description")) {
				package->short_desc = cur["description"].GetString();
				package->long_desc = package->short_desc;
			}
			if (cur.HasMember("version"))
				package->version = cur["version"].GetString();
			if (cur.HasMember("license"))
				package->license = cur["license"].GetString();
			if (cur.HasMember("releasenotes"))
				package->changelog = cur["releasenotes"].GetString();
			if (cur.HasMember("category")) {
				// category is an array of strings in the JSON, we'll take just the first one
				const Value& category = cur["category"];
				if (category.Size() > 0)
					package->category = category[0].GetString();
			}

			// TODO: screen shots and icons
			// TODO: address needing to follow unistore script instructions properly
			
			result.push_back(std::move(package));
		}

	}
	return result;
}

std::string UniStoreRepo::getType() const
{
	return "unistore";
}

std::string UniStoreRepo::getZipUrl(const Package& package) const
{
	// OSC packages just use the url field directly
	return "";
}

std::string UniStoreRepo::getIconUrl(const Package& package) const
{
	// OSC packages just use the url field directly
	return "";
}