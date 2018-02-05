#include "Package.hpp"
#include <vector>

class Repo
{
	Repo(std::string url);
	std::string name;
	std::string url;
	std::vector<Package> packages;
	bool enabled;
};
