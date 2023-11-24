#include "Repo.hpp"
#include "GetRepo.hpp"
#include "LocalRepo.hpp"
#include "OSCRepo.hpp"
#include "Package.hpp"
#include <iostream>
#include <sstream>

Repo::Repo()
{
	// default constructor
}

Repo::Repo(std::string_view name, std::string_view url, bool enabled)
{
	// create a repo from the passed parameters
	this->name = name;
	this->url = url;
	this->enabled = enabled;
}

std::string Repo::generateRepoJson(const Repo& repo)
{
	std::stringstream response;
	response << "{\n\t\"repos\": [\n";

	response << repo.toJson();

	response << "\t]\n}\n";

	return response.str();
}

std::unique_ptr<Repo> Repo::createRepo(std::string_view name, std::string_view url, bool enabled, std::string_view type, std::string_view package_path)
{
	if (type == "get")
		return std::make_unique<GetRepo>(name, url, enabled);
	else if (type == "local")
		return std::make_unique<LocalRepo>(std::string(package_path));
	else if (type == "osc")
		return std::make_unique<OSCRepo>(name, url, enabled);
	// TODO: add more supported repo formats here

	return nullptr;
}

std::string Repo::toJson() const
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

std::string Repo::toString() const
{
	return "[" + this->getType() + " - " + getName() + "] " + getUrl() + " (" + (isEnabled() ? "enabled" : "disabled") + ")";
}

std::string Repo::getName() const
{
	return this->name;
}

std::string Repo::getUrl() const
{
	return this->url;
}

bool Repo::isEnabled() const
{
	return this->enabled;
}

void Repo::setEnabled(bool enabled_)
{
	this->enabled = enabled_;
}

bool Repo::isLoaded() const
{
	return this->loaded;
}