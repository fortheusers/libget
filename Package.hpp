#include <string>

class Package
{
    public:
	Package();
    
    std::string toString();
    bool downloadZip();
    bool install();
    bool remove();
    
    // Package attributes
	std::string pkg_name;
	std::string title;
	std::string author;
	std::string short_desc;
	std::string version;
    
    // Sorting attributes
//    Repo* parentRepo;
    std::string* repoUrl;
    
    // see top of file for enums
	char category;
    
    // bitmask for permissions, from left to right:
    // unused, iosu, kernel, nand, usb, sd, wifi, sound
    char permissions;
    
};