#include "Package.hpp"
#include "util.hpp"

Package::Package()
{
	this->pkg_name = "?";
}

std::string Package::toString()
{
    return "[" + this->pkg_name + "] (" + this->version + ") \"" + this->title + "\" - " + this->short_desc;
}

bool Package::downloadZip()
{
    // fetch zip file to tmp directory using curl
    return downloadFileToDisk(*(this->repoUrl) + "/zips/" + this->pkg_name + ".zip", "./sdroot/" + this->pkg_name + ".zip");
}