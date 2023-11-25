#include <algorithm>
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unordered_set>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "Get.hpp"
#include "GetRepo.hpp"
#include "LocalRepo.hpp"
#include "Utils.hpp"

using namespace rapidjson;

bool debug = false;

Get::Get(std::string_view config_dir, std::string_view defaultRepo)
	: mDefaultRepo(defaultRepo)
{

	// the path for the get metadata folder
	std::string config_path = std::string(config_dir);

	mRepos_path = std::string(config_path + "repos.json");
	mPkg_path = std::string(config_path + "packages/");
	mTmp_path = std::string(config_path + "tmp/");

	//	  printf("--> Using \"./sdroot\" as local download root directory\n");
	//	  my_mkdir("./sdroot");

	my_mkdir(config_dir.data());
	my_mkdir(mPkg_path.c_str());
	my_mkdir(mTmp_path.c_str());

	printf("--> Using \"%s\" as repo list\n", mRepos_path.c_str());

	// load repo info
	this->loadRepos();
	this->update();
}

int Get::install(Package& package)
{
	// found package in a remote server, fetch it
	bool located = package.downloadZip(mTmp_path);

	if (!located)
	{
		// according to the repo list, the package zip file should've been here
		// but we got a 404 and couldn't find it
		printf("--> Error retrieving remote file for [%s] (check network or 404 error?)\n", package.getPackageName().c_str());
		return false;
	}

	// install the package, (extracts manifest, etc)
	package.install(mPkg_path, mTmp_path);

	printf("--> Downloaded [%s] to sdroot/\n", package.getPackageName().c_str());

	// update again post-install
	update();
	return true;
}

int Get::remove(Package& package)
{
	package.remove(mPkg_path);
	printf("--> Uninstalled [%s] package\n", package.getPackageName().c_str());
	update();

	return true;
}

int Get::toggleRepo(Repo& repo)
{
	repo.setEnabled(!repo.isEnabled());
	update();
	return true;
}

void Get::addLocalRepo()
{
	repos.push_back(std::make_unique<LocalRepo>(mPkg_path));
	update();
}

/**
Load any repos from a config file into the repos vector.
**/
void Get::loadRepos()
{
	repos.clear();

	auto& config_path = mRepos_path;
	std::ifstream ifs(config_path);

	if (!ifs.good() || ifs.peek() == std::ifstream::traits_type::eof())
	{
		printf("--> Could not load repos from %s, generating default GET repos.json\n", config_path.c_str());

		auto defaultRepo = std::make_unique<GetRepo>("Default Repo", this->mDefaultRepo, true);

		Document d;
		d.Parse(Repo::generateRepoJson(*defaultRepo).c_str());

		std::ofstream file(config_path);
		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		d.Accept(writer);
		file << buffer.GetString();
		file.close();

		ifs = std::ifstream(config_path);

		if (!ifs.good())
		{
			printf("--> Could not generate a new repos.json\n");

			// manually create a repo, no file access (so we append now, since we won't be able to load later)
			repos.push_back(std::move(defaultRepo));
			return;
		}
	}

	IStreamWrapper isw(ifs);

	Document doc;
	ParseResult ok = doc.ParseStream(isw);

	if (!ok || !doc.HasMember("repos"))
	{
		printf("--> Invalid format in %s", config_path.c_str());
		return;
	}

	const Value& repos_doc = doc["repos"];

	// for every repo
	for (Value::ConstValueIterator it = repos_doc.Begin(); it != repos_doc.End(); it++)
	{

		auto repoName = "Default Repo";
		auto repoUrl = "";
		auto repoEnabled = false;
		auto repoType = "get"; // carryover from before this was defined

		if ((*it).HasMember("name"))
			repoName = (*it)["name"].GetString();
		if ((*it).HasMember("url"))
			repoUrl = (*it)["url"].GetString();
		if ((*it).HasMember("enabled"))
			repoEnabled = (*it)["enabled"].GetBool();
		if ((*it).HasMember("type"))
			repoType = (*it)["type"].GetString();

		printf("--> Found repo: %s, %s\n", repoName, repoType);

		auto repo = Repo::createRepo(repoName, repoUrl, repoEnabled, repoType, mPkg_path);
		if (repo)
		{
			repos.push_back(std::move(repo));
		}
	}
}

void Get::update()
{
	printf("--> Updating package list\n");
	// clear current packages
	packages.clear();

	// fetch recent package list from enabled repos
	int i = 0;
	for (const auto& repo : repos)
	{
		printf("--> Checking repo %s\n", repo->getName().c_str());
		if (repo->isLoaded() && repo->isEnabled())
		{
			printf("--> Repo %s is loaded and enabled\n", repo->getName().c_str());
			if (libget_status_callback != nullptr)
			{
				libget_status_callback(STATUS_RELOADING, i + 1, (int32_t)repos.size());
			}

			for (auto& element : repo->loadPackages())
			{
				element->mRepo = repo;
				packages.push_back(std::move(element));
			}
		}
		i++;
	}

	if (libget_status_callback != nullptr)
	{
		libget_status_callback(STATUS_UPDATING_STATUS, 1, 1);
	}

	// remove duplicates, prioritizing later packages over earlier ones
	this->removeDuplicates();

	// check for any installed packages to update their status
	for (const auto& package : packages) {
		package->updateStatus(mPkg_path);
	}

	// sort the packages by name
	// TODO: apply other sort orders here, and potentially search filters
	std::sort(packages.begin(), packages.end(), [](const std::shared_ptr<Package>& a, const std::shared_ptr<Package>& b) {
		return a->getPackageName() < b->getPackageName();
	});
}

int Get::validateRepos() const
{
	if (repos.empty())
	{
		printf("--> There are no repos configured!\n");
		return ERR_NO_REPOS;
	}

	return 0;
}

std::vector<Package> Get::list()
{
	// packages is a vector of shared_ptrs, so we need to dereference them
	std::vector<Package> ret;
	for (auto& cur : packages) {
		if (cur != nullptr)
			ret.emplace_back(*cur);
	}
	return ret;
}

std::vector<Package> Get::search(const std::string& query)
{
	// TODO: replace with inverted index for speed
	// https://vgmoose.com/blog/implementing-a-static-blog-search-clientside-in-js-6629164446/

	std::vector<Package> results;
	std::string lower_query = toLower(query);

	for (auto& cur : packages)
	{
		if (cur != nullptr && (toLower(cur->getTitle()).find(lower_query) != std::string::npos || toLower(cur->getAuthor()).find(lower_query) != std::string::npos || toLower(cur->getShortDescription()).find(lower_query) != std::string::npos || toLower(cur->getLongDescription()).find(lower_query) != std::string::npos))
		{
			// matches, add to return vector, and continue
			results.emplace_back(*cur); // add copy to result;
			continue;
		}
	}

	return results;
}

std::optional<Package> Get::lookup(const std::string& pkg_name)
{
	for (auto& cur : packages)
	{
		if (cur && cur->getPackageName() == pkg_name)
		{
			// return copy!
			return *cur;
		}
	}
	return std::nullopt;
}

void Get::removeDuplicates()
{
	std::unordered_set<std::string> packageSet;
	std::unordered_set<std::shared_ptr<Package>> removalSet;

	// going backards, fill out our sets
	// (prioritizes later repo packages over earlier ones, regardless of versioning)
	// TODO: semantic versioning or have a versionCode int that increments every update
	for (int32_t x = (int32_t)packages.size() - 1; x >= 0; x--)
	{
		auto& name = packages[x]->getPackageName();
		if (packageSet.find(name) == packageSet.end())
			packageSet.insert(name);
		else
			removalSet.insert(packages[x]);
	}

	// remove them, if they are in the removal set
	packages.erase(std::remove_if(packages.begin(), packages.end(), [removalSet](auto& p)
					   { return removalSet.find(p) != removalSet.end(); }),
		packages.end());
}

void info(const char* format, ...)
{
	if (!debug) return;
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
}
