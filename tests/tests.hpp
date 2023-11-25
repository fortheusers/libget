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
std::string calculateMD5(const char* path);
 
 /* Any 32-bit or wider unsigned integer data type will do */
typedef unsigned int MD5_u32plus;
 
typedef struct {
	MD5_u32plus lo, hi;
	MD5_u32plus a, b, c, d;
	unsigned char buffer[64];
	MD5_u32plus block[16];
} MD5_CTX;

extern void MD5_Init(MD5_CTX *ctx);
extern void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
extern void MD5_Final(unsigned char *result, MD5_CTX *ctx);
 

#endif