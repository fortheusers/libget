#ifndef TEST_H
#define TEST_H

#include "../src/Get.hpp"
#include "../src/Utils.hpp"
#include <cstdio>
#include <sys/stat.h>
#include <vector>
#include <sstream>

#define URL "https://vgmoose.github.io/get/tests/server/"

using namespace std;

class Test
{
	public:
	Get* get;
	string purpose = "Demo Test";
	stringstream error;
	bool passed = false;
	virtual bool execute() {
		return false;
	};
};

int count(Get* get, int state);
bool install(Get* get, const char* name);
bool remove(Get* get, const char* name);
bool exists(const char* path);

#endif