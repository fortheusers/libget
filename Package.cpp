#include "Package.hpp"
#include "util.hpp"
#include "zip.hpp"
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <sstream>
#define u8 uint8_t

Package::Package()
{
	this->pkg_name = "?";
    this->title = "???";
    this->author = "Unknown";
    this->version = "0.0.0";
    this->short_desc = "N/A";
}

const char* pkg_path = "./.get/packages/";
const char* tmp_path = "./.get/tmp/";

std::string Package::toString()
{
    return "[" + this->pkg_name + "] (" + this->version + ") \"" + this->title + "\" - " + this->short_desc;
}

bool Package::downloadZip()
{
    // fetch zip file to tmp directory using curl
    return downloadFileToDisk(*(this->repoUrl) + "/zips/" + this->pkg_name + ".zip", tmp_path + this->pkg_name + ".zip");
}

bool Package::install()
{
    // assumes that download was called first
    
//    printf("-> HomebrewManager::installZip");
//	if(UseProgressBar)
//		Progress->setTitle("Installing Homebrew to SDCard...");
	
	//! Open the Zip file
	UnZip * HomebrewZip = new UnZip((tmp_path + this->pkg_name + ".zip").c_str());
	
	//! First extract the Manifest	
	std::string ManifestPathInternal = "manifest.install";
	std::string ManifestPath = pkg_path + this->pkg_name + "/" + ManifestPathInternal;
	HomebrewZip->ExtractFile(ManifestPathInternal.c_str(), ManifestPath.c_str());
	
	//! Open the Manifest
//	CFile * ManifestFile = new CFile(ManifestPath, CFile::ReadOnly);
    std::ifstream ManifestFile;
    ManifestFile.open(ManifestPath.c_str());
    	
	//! Make sure the manifest is present and not empty
	if (ManifestFile.good())
	{
		//! Parse the manifest
//		printf("Parsing the manifest\n");
		
		std::stringstream Manifest;
        Manifest << ManifestFile.rdbuf();
		
		std::string CurrentLine;
		while(std::getline(Manifest, CurrentLine))
		{		
			char Mode = CurrentLine.at(0);
			std::string Path = CurrentLine.substr(3);
			std::string ExtractPath = "sd:/" + Path;
			
			switch(Mode)
			{
				case 'E':
					//! Simply Extract, with no checks or anything, won't be deleted upon removal
					printf("%s : EXTRACT\n", Path.c_str());
					HomebrewZip->ExtractFile(Path.c_str(), ExtractPath.c_str());
					break;
				case 'U':
					printf("%s : UPDATE\n", Path.c_str());
					HomebrewZip->ExtractFile(Path.c_str(), ExtractPath.c_str());
					break;
				case 'G':
					printf("%s : GET\n", Path.c_str());
					struct stat sbuff;
					if (stat(ExtractPath.c_str(), &sbuff) != 0) //! File doesn't exist, extract
						HomebrewZip->ExtractFile(Path.c_str(), ExtractPath.c_str());
					else
						printf("File already exists, skipping...");
					break;
				default:
					printf("%s : NOP\n", Path.c_str());
					break;
			}
		}
		
		//! Close the manifest
		Manifest.str("");
//		free(Manifest_cstr);
	}
	else
	{
		//! Extract the whole zip
		printf("No manifest found: extracting the Zip\n");
		HomebrewZip->ExtractAll("sd:/");
	}
	
	ManifestFile.close();
	
	//! Close the Zip file
	delete HomebrewZip;
	
	//! Delete the Zip file
	std::remove((tmp_path + this->pkg_name + ".zip").c_str());
    
    return true;
}

bool Package::remove()
{
    // perform an uninstall of the current package, parsing the cached metadata
    
    return true;
}