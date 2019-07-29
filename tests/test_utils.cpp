#include "tests.hpp"

int count(Get* get, int state)
{
	int count = 0;
	for (int x = 0; x < get->packages.size(); x++)
		if (get->packages[x]->status == state)
			count++;
	return count;
}

bool install(Get* get, const char* name)
{
	for (int x = 0; x < get->packages.size(); x++)
		if (get->packages[x]->pkg_name == name)
			return get->install(get->packages[x]);

	return false;
}

bool remove(Get* get, const char* name)
{
	for (int x = 0; x < get->packages.size(); x++)
		if (get->packages[x]->pkg_name == name)
			return get->remove(get->packages[x]);

	return false;
}

bool exists(const char* path)
{
	struct stat sbuff;
	return (stat(path, &sbuff) == 0);
}