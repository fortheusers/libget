#include "Package.hpp"
#include <vector>
#include <iostream>

class Repo
{
	public:
	Repo();
	std::string toString();
	std::string name;
	std::string url;
	std::vector<Package> packages;
	bool enabled;
};