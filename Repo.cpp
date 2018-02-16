#include "Repo.hpp"

Repo::Repo()
{
	
}

std::string Repo::toString()
{
	return "[" + this->name + "] <" + this->url + "> - " + ((this->enabled)? "enabled" : "disabled");
}