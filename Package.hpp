#include <string>

class Package
{
    public:
	Package();
    
    std::string toString();
    
    // Package attributes
	std::string pkg_name;
	std::string title;
	std::string author;
	std::string short_desc;
	std::string version;
    
    // Sorting attributes
//    Repo* parentRepo;
    
    // see top of file for enums
	char category;
    
    // bitmask for permissions, from left to right:
    // unused, iosu, kernel, nand, usb, sd, wifi, sound
    char permissions;
    
};