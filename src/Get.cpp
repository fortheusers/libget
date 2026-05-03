#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <errno.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unordered_set>

#if !defined(WIN32) && !defined(_WIN32)
#include <fcntl.h>
#include <unistd.h>
#endif

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "Get.hpp"
#include "./repos/GetRepo.hpp"
#include "./repos/LocalRepo.hpp"
#include "Utils.hpp"

using namespace rapidjson;

bool debug = false;

Get::Get(
	std::string_view config_dir,
	std::string_view defaultRepo,
	bool performInitialLoad,
	std::string defaultRepoType
)
	: mDefaultRepo(defaultRepo), mDefaultRepoType(defaultRepoType)
{

	// the path for the get metadata folder
	std::string config_path = std::string(config_dir);

	mRepos_path = std::string(config_path + "repos.json");
	mPkg_path = std::string(config_path + "packages/");
	mTmp_path = std::string(config_path + "tmp/");

	//	  printf("--> Using \"./sdroot\" as local download root directory\n");
	//	  my_mkdir("./sdroot");

	if (!mkpath(config_path))
		printf("--> Could not create config dir %s\n", config_path.c_str());
	if (!mkpath(mPkg_path))
		printf("--> Could not create packages dir %s\n", mPkg_path.c_str());
	if (!mkpath(mTmp_path))
		printf("--> Could not create tmp dir %s\n", mTmp_path.c_str());

	printf("--> Using \"%s\" as repo list\n", mRepos_path.c_str());

	this->loadRepos();

	if (performInitialLoad) {
		// load repo info
		this->update();
	}
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

void Get::addAndRemoveReposByURL(
	const std::unordered_map<std::string, std::string>& reposToAdd,
	const std::unordered_set<std::string>& reposToRemove
)
{
	int reposLen = repos.size();

	bool madeChanges = false;

	repos.erase(std::remove_if(repos.begin(), repos.end(), 
		[reposToRemove](auto curRepo) {
			std::string curUrl = curRepo->getUrl();
			return reposToRemove.find(curUrl) != reposToRemove.end();
		}), repos.end()
	);

	madeChanges = reposLen != repos.size();

	std::unordered_set<std::string> currentUrls;
	for (auto& curRepo : repos) {
		currentUrls.insert(curRepo->getUrl());
	}
	
	for (auto& entry : reposToAdd) {
		auto url = entry.first;
		auto curType = entry.second;
		if (currentUrls.find(url) == currentUrls.end()) {
			// extract domain from url string
			std::string nameSummary;
			size_t start = url.find("//");

			if (start != std::string::npos) {
				nameSummary = url.substr(start + 2);
				size_t end = nameSummary.find("/");
				if (end != std::string::npos) {
					nameSummary = nameSummary.substr(0, end);
				}
			}

			// if nameSummary is still empty, provide a fallback
			if (nameSummary.empty()) {
				nameSummary = "Auto-added from Meta";
			}
			auto newRepo = Repo::createRepo(nameSummary, url, true, curType, "");
			repos.push_back(std::shared_ptr<Repo>(std::move(newRepo)));
		}
	}

	madeChanges = madeChanges || reposLen != repos.size();

	// save the repos to disk, if we've made any changes
	if (madeChanges) {
		saveRepos();
		loadRepos();
	}
}

// Saves the repos from our current Get object to disk, via a tmp file and
// atomic rename so a crash mid-write can't truncate repos.json.
void Get::saveRepos() {
	std::string tmpPath = mRepos_path + ".tmp";

	Document d;
	d.SetObject();
	Document::AllocatorType& allocator = d.GetAllocator();

	Value reposOut(kArrayType);

	for (auto& repo : repos) {
		Value repoObj(kObjectType);
		repoObj.AddMember("name", rapidjson::Value(repo->getName().c_str(), allocator), allocator);
		repoObj.AddMember("url",  rapidjson::Value(repo->getUrl().c_str(), allocator), allocator);
		repoObj.AddMember("type", rapidjson::Value(repo->getType().c_str(), allocator), allocator);
		repoObj.AddMember("enabled", repo->isEnabled(), allocator);
		reposOut.PushBack(repoObj, allocator);
	}

	d.AddMember("repos", reposOut, allocator);

	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	d.Accept(writer);

	{
		std::ofstream file(tmpPath);
		if (!file.is_open()) {
			printf("--> Could not open %s for writing\n", tmpPath.c_str());
			return;
		}
		file << buffer.GetString();
		file.flush();
		if (file.fail()) {
			printf("--> Write failed for %s\n", tmpPath.c_str());
			file.close();
			std::remove(tmpPath.c_str());
			return;
		}
	}

#if !defined(WIN32) && !defined(_WIN32)
	// fsync so the new content reaches the SD card before the rename
	int fd = open(tmpPath.c_str(), O_RDONLY);
	if (fd >= 0) {
		fsync(fd);
		close(fd);
	}
#endif

	// std::filesystem::rename atomically replaces an existing destination on
	// both POSIX and Windows; plain std::rename does not on Windows.
	std::error_code ec;
	std::filesystem::rename(tmpPath, mRepos_path, ec);
	if (ec) {
		printf("--> Could not rename %s to %s: %s\n", tmpPath.c_str(), mRepos_path.c_str(), ec.message().c_str());
		std::filesystem::remove(tmpPath, ec);
		return;
	}

#if !defined(WIN32) && !defined(_WIN32)
	// fsync the directory so the rename itself survives a power-off
	std::string parentDir = dir_name(mRepos_path);
	if (parentDir.empty()) parentDir = ".";
	int dirFd = open(parentDir.c_str(), O_RDONLY);
	if (dirFd >= 0) {
		fsync(dirFd);
		close(dirFd);
	}
#endif
}

/**
Load any repos from a config file into the repos vector. Regenerates the
default repo when the file is missing/empty/corrupted, or when it parses to
a valid but empty repos array.
**/
void Get::loadRepos()
{
	repos.clear();

	auto& config_path = mRepos_path;

	auto generateDefault = [&](const char* reason, bool writeToDisk = true) {
		printf("--> Generating default repos.json (%s)\n", reason);

#if defined(WII) || defined(_3DS) || defined(WII_MOCK)
		auto defaultRepo = GetRepo::createRepo("Default Repo", this->mDefaultRepo, true, this->mDefaultRepoType, mPkg_path);
#else
		auto defaultRepo = std::make_unique<GetRepo>("Default Repo", this->mDefaultRepo, true);
#endif

		if (writeToDisk)
		{
			Document d;
			d.Parse(Repo::generateRepoJson(*defaultRepo).c_str());

			std::ofstream file(config_path);
			if (file.is_open())
			{
				StringBuffer buffer;
				Writer<StringBuffer> writer(buffer);
				d.Accept(writer);
				file << buffer.GetString();
				file.close();
			}
			else
			{
				printf("--> Could not write default to %s, using in-memory only\n", config_path.c_str());
			}
		}

		// keep an in-memory default even if the disk write failed or was skipped
		repos.clear();
		repos.push_back(std::move(defaultRepo));
	};

	std::ifstream ifs(config_path);

	if (!ifs.good() || ifs.peek() == std::ifstream::traits_type::eof())
	{
		generateDefault("file missing or empty");
		return;
	}

	IStreamWrapper isw(ifs);

	Document doc;
	ParseResult ok = doc.ParseStream(isw);

	if (!ok || !doc.HasMember("repos"))
	{
		printf("--> Invalid JSON in %s\n", config_path.c_str());
		ifs.close();

		// preserve the corrupted file so the user can recover from it manually;
		// fall back to a timestamped name so a previous .bad isn't clobbered
		std::string badPath = config_path + ".bad";
		std::error_code ec;
		if (std::filesystem::exists(badPath, ec))
			badPath = config_path + ".bad." + std::to_string(time(nullptr));

		std::filesystem::rename(config_path, badPath, ec);
		if (ec)
		{
			// can't preserve safely; leave the bad file alone and only run an in-memory default
			printf("--> Could not preserve corrupted file (%s); leaving %s untouched\n", ec.message().c_str(), config_path.c_str());
			generateDefault("invalid JSON, no on-disk write", false);
		}
		else
		{
			printf("--> Moved corrupted file to %s\n", badPath.c_str());
			generateDefault("invalid JSON");
		}
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

	if (repos.empty())
	{
		ifs.close();
		generateDefault("repos array was empty");
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
	// std::sort(packages.begin(), packages.end(), [](const std::shared_ptr<Package>& a, const std::shared_ptr<Package>& b) {
	// 	return a->getPackageName() < b->getPackageName();
	// });
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
