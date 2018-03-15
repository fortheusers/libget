#include "Package.hpp"
#include "Utils.hpp"
#include "ZipUtil.hpp"
#include "constants.h"
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/document.h"
#define u8 uint8_t

#if defined (SWITCH)
#define ROOT_PATH "/"
#else
#define ROOT_PATH "sdroot/"
#endif

Package::Package(int state)
{
	this->pkg_name = "?";
	this->title = "???";
	this->author = "Unknown";
	this->version = "0.0.0";
	this->short_desc = "N/A";
	
	this->status = state;
}

Package::~Package()
{
	delete this->contents;
}

std::string Package::toString()
{
	return "[" + this->pkg_name + "] (" + this->version + ") \"" + this->title + "\" - " + this->short_desc;
}

bool Package::downloadZip(const char* tmp_path)
{
	// fetch zip file to tmp directory using curl
	printf("--> Downloading %s to %s\n", this->pkg_name.c_str(), tmp_path);
	return downloadFileToDisk(*(this->repoUrl) + "/zips/" + this->pkg_name + ".zip", tmp_path + this->pkg_name + ".zip");
}

bool Package::install(const char* pkg_path, const char* tmp_path)
{
	// assumes that download was called first
	
	//! Open the Zip file
	UnZip * HomebrewZip = new UnZip((tmp_path + this->pkg_name + ".zip").c_str());

	//! First extract the Manifest
	std::string ManifestPathInternal = "manifest.install";
	std::string ManifestPath = pkg_path + this->pkg_name + "/" + ManifestPathInternal;
	HomebrewZip->ExtractFile(ManifestPathInternal.c_str(), ManifestPath.c_str());

	//! Then extract the info.json file (to know what version we have installed and stuff)
	std::string jsonPathInternal = "info.json";
	std::string jsonPath = pkg_path + this->pkg_name + "/" + jsonPathInternal;
	HomebrewZip->ExtractFile(jsonPathInternal.c_str(), jsonPath.c_str());
	
	//! Open the Manifest
	std::ifstream ManifestFile;
	ManifestFile.open(ManifestPath.c_str());

	//! Make sure the manifest is present and not empty
	if (ManifestFile.good())
	{
		//! Parse the manifest
		std::stringstream Manifest;
		Manifest << ManifestFile.rdbuf();
		std::string CurrentLine;

		while(std::getline(Manifest, CurrentLine))
		{
			char Mode = CurrentLine.at(0);
			std::string Path = CurrentLine.substr(3);
			std::string ExtractPath = ROOT_PATH + Path;

			int resp = 0;
			switch(Mode)
			{
				case 'E':
					//! Simply Extract, with no checks or anything, won't be deleted upon removal
					printf("%s : EXTRACT\n", Path.c_str());
					resp = HomebrewZip->ExtractFile(Path.c_str(), ExtractPath.c_str());
					break;
				case 'U':
					printf("%s : UPDATE\n", Path.c_str());
					resp = HomebrewZip->ExtractFile(Path.c_str(), ExtractPath.c_str());
					break;
				case 'G':
					printf("%s : GET\n", Path.c_str());
					struct stat sbuff;
					if (stat(ExtractPath.c_str(), &sbuff) != 0) //! File doesn't exist, extract
						resp = HomebrewZip->ExtractFile(Path.c_str(), ExtractPath.c_str());
					else
						printf("File already exists, skipping...");
					break;
				default:
					printf("%s : NOP\n", Path.c_str());
					break;
			}

			if (resp < 0)
			{
				printf("--> Some issue happened while extracting! Error: %d\n", resp);
				return false;
			}
		}

		//! Close the manifest
		Manifest.str("");
	}
	else
	{
		//! Extract the whole zip
//		printf("No manifest found: extracting the Zip\n");
//		HomebrewZip->ExtractAll("sdroot/");
		// TODO: generate a manifest here, it's needed for deletion
		printf("No manifest file found (or error writing manifest download)! Refusing to extract.\n");
		return false;
	}

	ManifestFile.close();

	//! Close the Zip file
	delete HomebrewZip;

	//! Delete the Zip file
	std::remove((tmp_path + this->pkg_name + ".zip").c_str());

	return true;
}

bool Package::remove(const char* pkg_path)
{
	// perform an uninstall of the current package, parsing the cached metadata
	std::string ManifestPathInternal = "manifest.install";
	std::string ManifestPath = pkg_path + this->pkg_name + "/" + ManifestPathInternal;
	
	printf("-> HomebrewManager::Delete\n");
	
	struct stat sbuff;
	if (stat(ManifestPath.c_str(), &sbuff) != 0) //! There's no manifest
	{
		// there should've been one!
		// TODO: generate a temporary one
		printf("--> ERROR: no manifest found at %s\n", ManifestPath.c_str());
		return false;
	}
	
	//! Open the manifest normally
	std::ifstream ManifestFile;
	ManifestFile.open(ManifestPath.c_str());
	
	std::stringstream Manifest;
	Manifest << ManifestFile.rdbuf();
	
	//! Parse the manifest
	printf("Parsing the Manifest\n");
	
	std::string CurrentLine;
	while(std::getline(Manifest, CurrentLine))
	{		
		char Mode = CurrentLine.at(0);
		std::string DeletePath = "sdroot/" + CurrentLine.substr(3);
		
		switch(Mode)
		{
			case 'U':
				printf("%s : UPDATE\n", DeletePath.c_str());
				printf("Removing %s\n", DeletePath.c_str());
				std::remove(DeletePath.c_str());
				break;
			case 'G':
				printf("%s : GET\n", DeletePath.c_str());
				printf("Removing %s\n", DeletePath.c_str());
				std::remove(DeletePath.c_str());
				break;
			case 'L':
				printf("%s : LOCAL\n", DeletePath.c_str());
				printf("Removing %s\n", DeletePath.c_str());
				std::remove(DeletePath.c_str());
				break;
			default:
				break;
		}
	}
	
	printf("Removing manifest...\n");
	
	ManifestFile.close();
	
	std::remove(ManifestPath.c_str());
	std::remove((std::string(pkg_path) + this->pkg_name + "/info.json").c_str());
	
	rmdir((std::string(pkg_path) + this->pkg_name).c_str());
	
	printf("Homebrew removed\n");
	
	return true;
}

void Package::updateStatus(const char* pkg_path)
{
	// check if the manifest for this package exists
	std::string ManifestPathInternal = "manifest.install";
	std::string ManifestPath = pkg_path + this->pkg_name + "/" + ManifestPathInternal;	
	
	struct stat sbuff;
	if (stat(ManifestPath.c_str(), &sbuff) == 0)
	{
		// manifest exists, we are at least installed
		this->status = INSTALLED;
	}
	
	// TODO: check for info.json, parse version out of it
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
		std::string version;
		
		if (ok && doc.HasMember("version"))
		{
			const rapidjson::Value& info_doc = doc["version"];
			version = info_doc.GetString();
		}
		else
			version = "0.0.0";

		if (version != this->version)
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
		
}

const char* Package::statusString()
{
	switch (this->status)
	{
		case LOCAL:
			return "LOCAL	 ";
		case INSTALLED:
			return "INSTALLED";
		case UPDATE:
			return "UPDATE	 ";
		case GET:
			return "GET		 ";
	}
	return "UNKNOWN";
}
