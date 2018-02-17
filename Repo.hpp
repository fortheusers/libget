#include <vector>
#include <iostream>
#include "Package.hpp"

class Repo
{
	public:
	Repo();
    
	std::string toString();
    void loadPackages(std::vector<Package*>* package);
    
	std::string name;
	std::string url;
	bool enabled;
};