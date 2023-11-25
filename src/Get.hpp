#ifndef GET_H
#define GET_H
#include "Repo.hpp"
#include "constants.h"
#include <optional>
#include <vector>

void info(const char* format, ...);

class Get
{
public:
	// constructor takes path to the .get directory, and a fallback default repo url
	Get(std::string_view config_dir, std::string_view defaultRepo);

	int install(Package& pkg_name); // download the given package name and manifest data
	int remove(Package& pkg_name);	// delete and remove all files for the given package name
	int toggleRepo(Repo& repo);		// enable/disable the specified repo (and write changes)

	std::vector<Package> search(const std::string& query); // return a list of packages matching query
	std::vector<Package> list();							// return a list of all packages
	std::optional<Package> lookup(const std::string& pkg_name);
	void addLocalRepo();
	void removeDuplicates();

	// map of word -> list of packages whose info matches that word
	// TODO: this
	// std::map<std::string, std::vector<Package*>>;

	// TODO: add queue functionality
	//	  void enqueue(int count, ...)	// add a number of packages to the download queue
	//	  void downloadAll()			// download all of the queued packages

	// config paths (TODO: replace with a Config class)
	std::string mRepos_path;
	std::string mPkg_path;
	std::string mTmp_path;

	[[nodiscard]] const std::vector<std::shared_ptr<Repo>>& getRepos() const
	{
		return repos;
	}
	[[nodiscard]] const std::vector<std::shared_ptr<Package>>& getPackages() const
	{
		return packages;
	}

private:
	void loadRepos();
	void update();
	int validateRepos() const;

	// the remote repos and packages
	std::vector<std::shared_ptr<Repo>> repos;
	std::vector<std::shared_ptr<Package>> packages;

	std::string mDefaultRepo;
};
#endif
