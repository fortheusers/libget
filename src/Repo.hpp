#ifndef REPO_H
#define REPO_H
#include "Package.hpp"
#include <iostream>
#include <vector>

class Get;

#ifdef NETWORK_MOCK
#include "../tests/network_mock.hpp"
#endif

class Repo
{
public:
	virtual std::string toJson() = 0;
	virtual std::string toString() = 0;
	virtual void loadPackages(Get* get, std::vector<Package*>* package) = 0;

	virtual std::string getName() = 0;
	virtual std::string getUrl() = 0;
	virtual bool isEnabled() = 0;
	virtual bool isLoaded() = 0; // whether this server could be reached or not
	virtual void setEnabled(bool enabled) = 0;
};

std::string generateRepoJson(int count, ...);
Repo* createRepo(std::string name, std::string url, bool enabled, std::string type);
#endif
