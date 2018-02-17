#include <stdio.h>
#include <string>

bool mkpath( std::string path );
bool CreateSubfolder( char* cstringpath );
void downloadFileToMemory(std::string path, std::string* buffer);