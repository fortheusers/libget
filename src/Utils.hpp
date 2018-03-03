#include <stdio.h>
#include <string>

// folder stuff
bool mkpath( std::string path );
bool CreateSubfolder( char* cstringpath );

// networking stuff
int init_networking();
bool downloadFileToMemory(std::string path, std::string* buffer);			// appends file to buffer
bool downloadFileToDisk(std::string remote_path, std::string local_path);	// saves file to local_path

// helper methods
const char* plural(int amount);
