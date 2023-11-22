#include "Repo.hpp"
#include "GetRepo.hpp"
#include "LocalRepo.hpp"
#include "OSCRepo.hpp"
#include "Package.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <stdarg.h> /* va_list, va_start, va_arg, va_end */

Repo::Repo()
{
	// default constructor
}

Repo::Repo(const char* name, const char* url, bool enabled)
{
	// create a repo from the passed parameters
	this->name = name;
	this->url = url;
	this->enabled = enabled;
}

std::string generateRepoJson(int count, ...)
{
	va_list ap;

	std::stringstream response;
	response << "{\n\t\"repos\": [\n";

	va_start(ap, count);

	for (int x = 0; x < count; x++)
		response << (va_arg(ap, Repo*))->toJson();

	va_end(ap);
	response << "\t]\n}\n";

	return response.str();
}

Repo* createRepo(std::string name, std::string url, bool enabled, std::string type)
{
	if (type == "get")
		return new GetRepo(name.c_str(), url.c_str(), enabled);
	else if (type == "local")
		return new LocalRepo();
	else if (type == "osc")
		return new OSCRepo(name.c_str(), url.c_str(), enabled);
	// TODO: add more supported repo formats here

	return NULL;
}

std::string Repo::toJson()
{
	std::stringstream resp;
	resp << "\t\t{\n";
	resp << "\t\t\t\"name\": \"" << getName() << "\",\n";
	resp << "\t\t\t\"url\": \"" << getUrl() << "\",\n";
	resp << "\t\t\t\"type\": \"" << getType() << "\",\n";
	resp << "\t\t\t\"enabled\": " << (isEnabled() ? "true" : "false") << "\n";
	resp << "\t\t}\n";
	return resp.str();
}

std::string Repo::toString()
{
	return "[" + this->getType() + " - " + getName() + "] " + getUrl() + " (" + (isEnabled() ? "enabled" : "disabled") + ")";
}


std::string Repo::getName()
{
	return this->name;
}

std::string Repo::getUrl()
{
	return this->url;
}

bool Repo::isEnabled()
{
	return this->enabled;
}

void Repo::setEnabled(bool enabled)
{
	this->enabled = enabled;
}

bool Repo::isLoaded()
{
	return this->loaded;
}