#include "./repos/Repo.hpp"
#include "Utils.hpp"
#include "ZipUtil.hpp"
#include "./repos/constants.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_set>
#include <vector>
#define u8 uint8_t

#if defined(__WIIU__)
// include xml files for legacy hb app store support
#include "tinyxml.h"
#endif

Package::Package(int state)
{
	this->pkg_name = "?";
	this->title = "???";
	this->author = "Unknown";
	this->version = "0.0.0";
	this->short_desc = "N/A";
	this->long_desc = "N/A";

	this->license = "";
	this->changelog = "";
	this->url = "";
	this->updated = "";
	this->updated_timestamp = 0;

	this->download_size = 0;
	this->extracted_size = 0;
	this->downloads = 0;

	this->category = "_all";
	this->binary = "none";

	this->status = state;
	this->screens = 0;
}

Package::~Package() = default;

std::string Package::toString() const
{
	return "[" + this->pkg_name + "] (" + this->version + ") \"" + this->title + "\" - " + this->short_desc;
}

bool Package::downloadZip(std::string_view tmp_path, float*) const
{
	if (libget_status_callback != nullptr)
		libget_status_callback(STATUS_DOWNLOADING, 1, 1);

	// fetch zip file to tmp directory using curl
	printf("--> Downloading %s to %s\n", this->pkg_name.c_str(), tmp_path.data());
	auto zipUrl = this->mRepo->getZipUrl(*this);
	return downloadFileToDisk(zipUrl, std::string(tmp_path) + this->pkg_name + ".zip");
}

bool Package::install(const std::string& pkg_path, const std::string& tmp_path)
{
	// assumes that download was called first
	if (libget_status_callback != nullptr)
		libget_status_callback(STATUS_ANALYZING, 1, 1);

	if (networking_callback != nullptr)
		networking_callback(nullptr, 10, 10, 0, 0);

#ifdef NETWORK_MOCK
	// for network mocking, copy over a /mock.zip to the expected download path
	cp(ROOT_PATH "mock.zip", (tmp_path + this->pkg_name + ".zip").c_str());
#endif

	// our internal path of where the manifest will be
	std::string ManifestPathInternal = "manifest.install";
	std::string ManifestPath = pkg_path + this->pkg_name + "/" + ManifestPathInternal;

	// before we uninstall, open up the current manifest, and get all the files in it
	// (later we will remove any that aren't in the new manifest)
	Manifest existingManifest(ManifestPath, ROOT_PATH);

	std::unordered_set<std::string> existing_package_paths;
	if (existingManifest.isValid() && manifest.isValid())
	{
		// go through its paths, add them our existing set
		for (const auto& entry : manifest.getEntries())
		{
			ManifestOp op = entry.operation;
			if (op == MUPDATE || op == MEXTRACT)
			{
				existing_package_paths.insert(entry.path);
			}
		}
	}

	//! Open the Zip file
	UnZip HomebrewZip(tmp_path + this->pkg_name + ".zip");

	//! First extract the Manifest
	HomebrewZip.ExtractFile(ManifestPathInternal, ManifestPath);

	//! Then extract the info.json file (to know what version we have installed and stuff)
	std::string jsonPathInternal = "info.json";
	std::string jsonPath = pkg_path + this->pkg_name + "/" + jsonPathInternal;
	HomebrewZip.ExtractFile(jsonPathInternal, jsonPath);

	this->manifest = Manifest(ManifestPath, ROOT_PATH);

	if (!manifest.isValid() && manifest.isFakeManifestPossible())
	{
#ifndef NETWORK_MOCK
		printf("--> Manifest invalid/doesn't exist but recoverable, generating pseudo-manifest\n");
		this->manifest = Manifest(HomebrewZip.PathDump(), ROOT_PATH);

		// write the pseudo-manifest to the internal package .get directory
		mkpath((pkg_path + this->pkg_name).c_str());

		std::ofstream pseudomanifest(ManifestPath);
		printf("--> Writing pseudo-manifest to %s\n", ManifestPath.c_str());
		for (const auto& entry : manifest.getEntries()) {
			pseudomanifest << entry.raw << std::endl;
		}
		pseudomanifest.close();

		// write a pseudo-info.json here too
		// TODO: load other attributes from the package, besides version
		std::ofstream pseudojson(jsonPath);
		pseudojson << "{\"version\":\"" << this->version << "\"}" << std::endl;
		pseudojson.close();
#endif
	}

	std::unordered_set<std::string> incoming_package_paths;

	if (manifest.isValid())
	{
		// get all file info from within the zip, for every path
		auto infoMap = HomebrewZip.GetPathToFilePosMapping();

		if (libget_status_callback != nullptr)
			libget_status_callback(STATUS_INSTALLING, 1, 1);

		int i = 0;
		const auto& entries = manifest.getEntries();
		for (const auto& entry : entries)
		{
			if (networking_callback != nullptr)
				networking_callback(nullptr, entries.size(), i + 1, 0, 0);

			i++;

			std::string Path = entry.zip_path;
			std::string ExtractPath = entry.path;
			auto pathCStr = Path.c_str();
			auto ePathCStr = ExtractPath.c_str();

			// track this specific file for later, when we remove files that we don't have entries for
			incoming_package_paths.insert(ExtractPath);

			// lookup this path from our map, to get its file info
			auto mapResult = infoMap.find(Path);
			if (mapResult == infoMap.end())
			{
				// auto onlyZipPaths = HomebrewZip.PathDump();
				// for (auto zipPath : onlyZipPaths)
				// {
				// 	printf("zip path: %s\n", zipPath.c_str());
				// }
				printf("--> ERROR: Could not find [%s] path in zip file\n", pathCStr);
				continue;
			}

			auto filePos = mapResult->second;

			int resp = 0;
			switch (entry.operation)
			{
			case MEXTRACT:
				//! Simply Extract, with no checks or anything, won't be deleted upon removal
				info("%s : EXTRACT\n", pathCStr);
				resp = HomebrewZip.Extract(ePathCStr, filePos);
				break;
			case MUPDATE:
				info("%s : UPDATE\n", pathCStr);
				resp = HomebrewZip.Extract(ePathCStr, filePos);
				break;
			case MGET:
			{
				info("%s : GET\n", pathCStr);
				struct stat sbuff = {};
				if (stat(ePathCStr, &sbuff) != 0) //! File doesn't exist, extract
					resp = HomebrewZip.Extract(ePathCStr, filePos);
				else
					info("File already exists, skipping...");
				break;
			}
			default:
				info("%s : NOP\n", ePathCStr);
				break;
			}

			if (resp < 0)
			{
				printf("--> Some issue happened while extracting! Error: %d\n", resp);
				return false;
			}
		}

		// done installing new files, go through the remaining files that we didn't just visit
		// and remove them (files that WERE in our old manifest, and AREN'T in the new one we got)
		for (auto& path : existing_package_paths)
		{
			// only continue if it's not in our incoming package path set
			if (incoming_package_paths.find(path) == incoming_package_paths.end())
			{
				std::remove(path.c_str());
				// printf("REMOVING: %s\n", path.c_str());
			}
		}
	}
	else
	{
		//! Extract the whole zip
		//		printf("No manifest found: extracting the Zip\n");
		//		HomebrewZip.ExtractAll("sdroot/");
		// TODO: generate a manifest here, it's needed for deletion
		if (!manifest.isFakeManifestPossible())
		{
			printf("--> Invalid/No manifest file found (or error writing manifest download)! Refusing to extract.\n");
			return false;
		}
	}

	//! Delete the Zip file
	std::remove((tmp_path + this->pkg_name + ".zip").c_str());

	return true;
}

bool Package::remove(std::string_view pkg_path)
{
	if (libget_status_callback != nullptr)
		libget_status_callback(STATUS_REMOVING, 1, 1);

	// perform an uninstall of the current package, parsing the cached metadata
	std::string ManifestPathInternal = "manifest.install";
	std::string ManifestPath = std::string(pkg_path) + this->pkg_name + "/" + ManifestPathInternal;

	info("HomebrewManager::Delete\n");
	std::unordered_set<std::string> uniq_folders;

	//! Parse the manifest
	info("Parsing the Manifest\n");
	if (!manifest.isValid())
	{
		this->manifest = Manifest(ManifestPath, ROOT_PATH); // Load and parse manifest if not yet done
	}
	if (this->manifest.isValid())
	{
		int i = 0;
		const auto& entries = manifest.getEntries();
		for (const auto& entry : entries)
		{
			if (networking_callback != nullptr)
				networking_callback(nullptr, entries.size(), i + 1, 0, 0);
			i++;
			const std::string& DeletePath = entry.path;

			// the current directory
			std::string cur_dir = dir_name(DeletePath);
			uniq_folders.insert(cur_dir);

			ManifestOp op = entry.operation;
			if (op != NOP && op != MEXTRACT) // get, upgrade, and local
			{
				info("Removing %s\n", DeletePath.c_str());
				std::remove(DeletePath.c_str());
			}
		}
	}
	else
	{
		printf("--> ERROR: Manifest missing or invalid at %s\n", ManifestPath.c_str());
		return false;
	}

	// sort unique folders from longest to shortest
	std::vector<std::string> folders;
	for (auto& folder : uniq_folders)
	{
		folders.push_back(folder);
	}
	std::sort(folders.begin(), folders.end(), compareLen);

	std::vector<std::string> intermediate_folders;

	// rmdir (only works if folders are empty!) out all uniq dirs...
	std::string fsroot(ROOT_PATH);
	for (auto& folder : folders)
	{
		auto parent = dir_name(folder);
		while (!parent.empty())
		{
			std::cout << "processing... " << parent << "\n";
			if ((uniq_folders.find(parent) == uniq_folders.end()) && (parent.length() > fsroot.length()))
			{
				std::cout << "adding " << parent << "\n";

				// folder not already seen, track it
				uniq_folders.insert(parent);
				intermediate_folders.push_back(parent);
			}
			parent = dir_name(parent);
		}
	}

	// have to re-add these outside of the loop because we can't
	// modify the vector while iterating through it
	for (auto& folder : intermediate_folders)
		folders.push_back(folder);

	// re-sort it
	std::sort(folders.begin(), folders.end(), compareLen);

	for (auto& folder : folders)
	{
		rmdir(folder.c_str());
	}

	printf("--> Removing manifest...\n");

	std::remove(ManifestPath.c_str());
	auto full_pkg_path = std::string(pkg_path) + this->pkg_name;
	std::remove((full_pkg_path + "/info.json").c_str());
	std::remove((full_pkg_path + "/icon.png").c_str()); // clean up icon if present

	rmdir((std::string(pkg_path) + this->pkg_name).c_str());

	// package removed, clean up empty directories
	// TODO: potentially prompt user to remove some known config files for a given package
	// see: https://github.com/vgmoose/get/issues/1
	// remove_empty_dirs(ROOT_PATH, 0);

	printf("--> Homebrew removed\n");

	return true;
}

void Package::updateStatus(const std::string& pkg_path)
{
	// check if the manifest for this package exists
	std::string ManifestPathInternal = "manifest.install";
	std::string ManifestPath = pkg_path + this->pkg_name + "/" + ManifestPathInternal;

	struct stat sbuff = {};
	if (stat(ManifestPath.c_str(), &sbuff) == 0)
	{
		// manifest exists, we are at least installed
		this->status = INSTALLED;

		this->manifest = Manifest(ManifestPath, ROOT_PATH);
	}

	// check for info.json, parse version out of it
	// and compare against the package's to know whether
	// it's an update or not
	std::string jsonPathInternal = "info.json";
	std::string jsonPath = pkg_path + this->pkg_name + "/" + jsonPathInternal;

	if (INSTALLED && stat(jsonPath.c_str(), &sbuff) == 0)
	{
		// pull out the version number and check if it's
		// different than the one on the repo
		std::ifstream ifs(jsonPath.c_str());
		rapidjson::IStreamWrapper isw(ifs);

		if (!ifs.good())
		{
			printf("--> Could not locate %s", jsonPath.c_str());
			this->status = UPDATE; // issue opening info.json, assume update
			return;
		}

		rapidjson::Document doc;
		rapidjson::ParseResult ok = doc.ParseStream(isw);
		std::string tmpVersion;

		if (ok && doc.HasMember("version"))
		{
			const rapidjson::Value& info_doc = doc["version"];
			tmpVersion = info_doc.GetString();
		}
		else
			tmpVersion = "0.0.0";

		if (tmpVersion != this->version)
			this->status = UPDATE;

		// we're eithe ran update or an install at this point
		return;
	}
	else if (this->status == INSTALLED)
	{
		this->status = UPDATE; // manifest, but no info, always update
		return;
	}

	// if we're down here, and it's not a local package
	// already, it's probably a get package (package was
	// available, but the manifest wasn't installed)
	if (this->status != LOCAL)
		this->status = GET;

	// check for any homebrew that may have been previously installed
	// TODO: see https://github.com/vgmoose/hb-appstore/issues/20
	this->status = this->isPreviouslyInstalled();
}

int Package::isPreviouslyInstalled()
{
	// TODO: check for and scan Switch NRO files
#if defined(__WIIU__)
	// we're on a Wii U, so let's check for any HBL meta.xml files that match this package's name,
	// and if it exists check the version based on that
	// TODO: check for and scan WUHB files
	TiXmlDocument xmlDoc((std::string(ROOT_PATH) + "wiiu/apps/" + this->pkg_name + "/meta.xml").c_str());
	bool xmlExists = xmlDoc.LoadFile();

	if (xmlExists)
	{
		TiXmlElement* appNode = xmlDoc.FirstChildElement("app");
		if (appNode)
		{
			TiXmlElement* node = appNode->FirstChildElement("version");
			if (node && node->FirstChild() && node->FirstChild()->Value())
			{
				// version exists, we should compare the value to the one on the server (this package)
				if (this->version != node->FirstChild()->Value())
					return UPDATE;
				else
					return LOCAL;
			}
		}
	}
#endif

	// since we are appstore and know that what version we're supposed to be, mark us local or updated if needed
	// TODO: make version check here dynamic, and also support other NROs or hint files
	// notice: this means that even if appstore isn't installed but is running, it will show as an update
	if (this->pkg_name == APP_SHORTNAME)
	{
		// it's app store, but wasn't detected as installed
		if (this->version == APP_VERSION)
			return LOCAL;
		else
			return UPDATE;
	}

	return this->status;
}

const char* Package::statusString() const
{
	switch (this->status)
	{
	case LOCAL:
		return "LOCAL";
	case INSTALLED:
		return "INSTALLED";
	case UPDATE:
		return "UPDATE";
	case GET:
		return "GET";
	}
	return "UNKNOWN";
}

std::string Package::getIconUrl() const
{
	// ask the parent repo for the icon url TODO: some fallback?
	if (this->mRepo == nullptr)
	{
		printf("--> ERROR: Parent repo not set for package %s\n", this->pkg_name.c_str());
		return "";
	}
	return this->mRepo->getIconUrl(*this);
}

std::string Package::getBannerUrl() const
{
	return this->mRepo->getUrl() + "/packages/" + this->pkg_name + "/screen.png";
}

std::string Package::getScreenShotUrl(int count) const
{
	return this->mRepo->getUrl() + "/packages/" + this->pkg_name + "/screen" + std::to_string(count) + ".png";
}

std::string Package::getManifestUrl() const
{
	return this->mRepo->getUrl() + "/packages/" + this->pkg_name + "/manifest.install";
}
