#include "tests.hpp"

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include <sstream>

#include <iomanip>

int count(Get* get, int state)
{
	int count = 0;
	for (auto const& package : get->getPackages())
		if (package->getStatus() == state)
			count++;
	return count;
}

bool install(Get* get, const char* name)
{
	for (auto const& package : get->getPackages())
		if (package->getPackageName() == name)
			return get->install(*package);

	return false;
}

bool remove(Get* get, const char* name)
{
	for (auto const& package : get->getPackages())
		if (package->getPackageName() == name)
			return get->remove(*package);

	return false;
}

bool exists(const char* path)
{
	struct stat sbuff;
	return (stat(path, &sbuff) == 0);
}


// https://stackoverflow.com/a/1220177

// Get the size of the file by its file descriptor
unsigned long get_size_by_fd(int fd) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf.st_size;
}

std::string calculateMD5(const char* path)
{
  // use  
// extern void MD5_Init(MD5_CTX *ctx);
// extern void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
// extern void MD5_Final(unsigned char *result, MD5_CTX *ctx);
// to calculate the md5 sum of the file at path

    auto file = fopen(path, "rb");
    if (!file) {
        return "";
    }
    auto fd = fileno(file);
    auto size = get_size_by_fd(fd);
    auto ptr = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        return "";
    }

    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, ptr, size);
    unsigned char result[16];
    MD5_Final(result, &ctx);
    munmap(ptr, size);
    fclose(file);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (auto i = 0; i < 16; ++i) {
        ss << std::setw(2) << static_cast<unsigned>(result[i]);
    }

  return ss.str();
}

