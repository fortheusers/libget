#ifndef PACKAGE_H
#define PACKAGE_H
#include <string>
#include "libs/rapidjson/include/rapidjson/rapidjson.h"
#include "libs/rapidjson/include/rapidjson/document.h"

class Package
{
	public:
	Package(int state);
	~Package();
	
	std::string toString();
	bool downloadZip(const char* tmp_path);
	bool install(const char* pkg_path, const char* tmp_path);
	bool remove(const char* pkg_path);
	const char* statusString();
	void updateStatus(const char* pkg_path);
	
	// Package attributes
	std::string pkg_name;
	std::string title;
	std::string author;
	std::string short_desc;
	std::string version;
	
	// Sorting attributes
//	  Repo* parentRepo;
	std::string* repoUrl;
	
	int status;	 // local, update, installed, get
	
	// see top of file for enums
	char category;
	
	// bitmask for permissions, from left to right:
	// unused, iosu, kernel, nand, usb, sd, wifi, sound
	char permissions;

	// the downloaded contents file, to keep memory around to cleanup later
	std::string* contents;
	
};

#endif
