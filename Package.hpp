#include <string>

class Package
{
	Package(std::string url);
	std::string pkg_name;
	std::string title;
	std::string author;
	std::string short_desc;
	std::string version;
	int category;
};