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
	Repo();
    Repo(const char* name, const char* url, bool enabled);

	virtual void loadPackages(Get* get, std::vector<Package*>* package) = 0;
	virtual std::string getType() = 0;
	virtual std::string getZipUrl(Package* package) = 0;
	virtual std::string getIconUrl(Package* package) = 0;

	std::string toJson();
	std::string toString();
	
	std::string getName();
	std::string getUrl();
	bool isEnabled();
	bool isLoaded(); // whether this server could be reached or not
	void setEnabled(bool enabled);

	std::string name;
    std::string url;
    bool enabled;
    bool loaded = true;
};

std::string generateRepoJson(int count, ...);
Repo* createRepo(std::string name, std::string url, bool enabled, std::string type);
#endif
