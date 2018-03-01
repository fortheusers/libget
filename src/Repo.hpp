#ifndef REPO_H
#define REPO_H
#include <vector>
#include <iostream>
#include "Package.hpp"

class Repo
{
	public:
	Repo();
	Repo(const char* name, const char* url);
	std::string toJson();	 
	std::string toString();
	void loadPackages(std::vector<Package*>* package);
	
	std::string name;
	std::string url;
	bool enabled;
};

std::string generateRepoJson(int count, ...);
#endif
