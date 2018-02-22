// opearting system level utilities
// currently only recursive mkdir

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <cstdint>
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <curl/easy.h>

#include "util.hpp"

bool CreateSubfolder(char* cpath)
{
	std::string * path = new std::string(cpath);
	return mkpath(*path);
}

// http://stackoverflow.com/a/11366985
bool mkpath( std::string path )
{
		bool bSuccess = false;
		int nRC = ::mkdir( path.c_str(), 0775 );
		if( nRC == -1 )
		{
				switch( errno )
				{
						case ENOENT:
								//parent didn't exist, try to create it
								if( mkpath( path.substr(0, path.find_last_of('/')) ) )
										//Now, try to create again.
										bSuccess = 0 == ::mkdir( path.c_str(), 0775 );
								else
										bSuccess = false;
								break;
						case EEXIST:
								//Done!
								bSuccess = true;
								break;
						default:
								bSuccess = false;
								break;
				}
		}
		else
				bSuccess = true;
		return bSuccess;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


// https://gist.github.com/alghanmi/c5d7b761b2c9ab199157
bool downloadFileToMemory(std::string path, std::string* buffer)
{
  CURL *curl;
  CURLcode res;
    
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
      
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
      
      if (*buffer == "")
          return false;
      
    return true;
  }
    
    return false;
}

bool downloadFileToDisk(std::string remote_path, std::string local_path)
{
    std::string fileContents;
    bool resp = downloadFileToMemory(remote_path, &fileContents);
    if (!resp)
        return false;
    
    std::ofstream file(local_path);
    file << fileContents;
    return true;
}

