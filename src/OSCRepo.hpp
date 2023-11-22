#ifndef OSC_REPO_H
#define OSC_REPO_H
#include "Repo.hpp"
#include "Package.hpp"
#include <iostream>
#include <vector>

class OSCRepo : public Repo
{
public:
    using Repo::Repo;
	void loadPackages(Get* get, std::vector<Package*>* package);
    std::string getZipUrl(Package* package);
    std::string getIconUrl(Package* package);

	std::string getType();
};
#endif
