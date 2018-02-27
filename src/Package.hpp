#ifndef PACKAGE_H
#define PACKAGE_H
#include <string>

class Package
{
    public:
    Package(int state);
    
    std::string toString();
    bool downloadZip();
    bool install();
    bool remove();
    const char* statusString();
    void updateStatus();
    
    // Package attributes
    std::string pkg_name;
    std::string title;
    std::string author;
    std::string short_desc;
    std::string version;
    
    // Sorting attributes
//    Repo* parentRepo;
    std::string* repoUrl;
    
    int status;  // local, update, installed, get
    
    // see top of file for enums
	char category;
    
    // bitmask for permissions, from left to right:
    // unused, iosu, kernel, nand, usb, sd, wifi, sound
    char permissions;
    
};
#endif