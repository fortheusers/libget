#include "Package.hpp"

Package::Package()
{
	this->pkg_name = "?";
}

std::string Package::toString()
{
    return "[" + this->pkg_name + "] (" + this->version + ") \"" + this->title + "\" - " + this->short_desc;
}