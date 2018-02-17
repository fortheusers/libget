#include <stdio.h>
#include <string>

bool mkpath( std::string path );
bool CreateSubfolder( char* cstringpath );
bool downloadFileToMemory(std::string path, std::string* buffer);
bool downloadFileToDisk(std::string remote_path, std::string local_path);