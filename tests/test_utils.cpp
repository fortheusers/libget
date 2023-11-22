#include "tests.hpp"

#include <openssl/md5.h>
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
  int file_descript;
  unsigned long file_size;
  char* file_buffer;

  file_descript = open(path, O_RDONLY);
  if(file_descript < 0) exit(-1);

  file_size = get_size_by_fd(file_descript);

  unsigned char result[MD5_DIGEST_LENGTH];

  file_buffer = static_cast<char*>(mmap(0, file_size, PROT_READ, MAP_SHARED, file_descript, 0));
  MD5((unsigned char*) file_buffer, file_size, result);

  int i;
  std::stringstream ss;
  for(i=0; i <MD5_DIGEST_LENGTH; i++) {
    ss << std::hex << std::setfill('0') << (int)result[i];
  }

  return ss.str();
}

