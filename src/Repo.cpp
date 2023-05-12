#include "Repo.hpp"
#include "GetRepo.hpp"
#include "LocalRepo.hpp"
#include "Package.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <stdarg.h> /* va_list, va_start, va_arg, va_end */

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
	// TODO: add more supported repo formats here

	return NULL;
}