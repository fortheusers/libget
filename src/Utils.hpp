#include <stdio.h>
#include <string>

// folder stuff
bool mkpath( std::string path );
bool CreateSubfolder( char* cstringpath );

// networking stuff
int init_networking();
bool downloadFileToMemory(std::string path, std::string* buffer);
bool downloadFileToDisk(std::string remote_path, std::string local_path);

// helper methods
const char* plural(int amount);

// switch/other platform specific
#if defined(__linux__) || defined(__APPLE__)
#else
	int http_get_file(std::string path, std::string* buff);
#endif
