#ifndef GET_REPO_H
#define GET_REPO_H
#include "Repo.hpp"
#include "Package.hpp"
#include <iostream>
#include <vector>

#ifdef NETWORK_MOCK
#include "../tests/network_mock.hpp"
#endif

class GetRepo : public Repo
{
public:
	GetRepo(const char* name, const char* url, bool enabled);
	std::string toJson();
	std::string toString();
	void loadPackages(Get* get, std::vector<Package*>* package);

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
#endif
