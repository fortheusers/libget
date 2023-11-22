#ifndef LOCAL_REPO_H
#define LOCAL_REPO_H
#include "Repo.hpp"
#include "Package.hpp"
#include <iostream>
#include <vector>

/**
 * A local repository has packages that use the internal format of installed packages.
 * 
 * The structure resemblers the packages in GetRepo, however they are loaded from the local
 * directory rather than from a server + json.
 */

class LocalRepo : public Repo
{
public:
	using Repo::Repo;
	void loadPackages(Get* get, std::vector<Package*>* package);
	std::string getZipUrl(Package* package);
	std::string getIconUrl(Package* package);

	std::string getType();
};
#endif
