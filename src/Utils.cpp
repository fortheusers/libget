// operating system level utilities
// contains directory utils, http utils, and helper methods

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <unistd.h>

#if defined(NOCURL)
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <sys/ioctl.h>
#else
	#include <curl/curl.h>
	#include <curl/easy.h>
#endif

#if defined(SWITCH)
	#include <switch.h>
#endif



#include "Utils.hpp"

void (*networking_callback)(float) = NULL;

bool CreateSubfolder(char* cpath)
{
	std::string path(cpath);
	return mkpath(path);
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

#if !defined(NOCURL)
	static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
	{
		((std::string*)userp)->append((char*)contents, size * nmemb);
		return size * nmemb;
	}
#else
	#define SOCK_BUFFERSIZE 65536

	// Establish connection with host
	int establishConnection()
	{
		int clientfd = socket(AF_INET, SOCK_STREAM, 0);
		
		// Connect to the remote server
		struct sockaddr_in remoteaddr;
		remoteaddr.sin_family = AF_INET;
		remoteaddr.sin_addr.s_addr = inet_addr("95.142.154.181");	// hardcoded to switchbru IP, no DNS
//		remoteaddr.sin_addr.s_addr = inet_addr("192.168.1.104");
		remoteaddr.sin_port = htons(80);
		connect(clientfd, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr));
		
		return clientfd;
	}

	// Send GET request
	void GET(int clientfd, const char *path) {
		char req[1000] = {0};
		sprintf(req, "GET %s HTTP/1.0\r\n\r\n", path);
		send(clientfd, req, strlen(req), 0);
	}
#endif

// https://gist.github.com/alghanmi/c5d7b761b2c9ab199157
bool downloadFileToMemory(std::string path, std::string* buffer)
{
	#if defined(NOCURL)
		int clientfd;
		char buf[SOCK_BUFFERSIZE];
	
		static int sock_buffersize = SOCK_BUFFERSIZE;
	
		// Establish connection with <hostname>:<port>
		clientfd = establishConnection();
	
		// increase buffer size
		setsockopt(clientfd, SOL_SOCKET, SO_RCVBUF, &sock_buffersize, sizeof(sock_buffersize));
		setsockopt(clientfd, SOL_SOCKET, SO_SNDBUF, &sock_buffersize, sizeof(sock_buffersize));
	
		// send GET request to remote
		GET(clientfd, path.c_str());
	
		int size = 1;
		int cumulative = 0;
	
		int got;
		while (true)
		{
			// receive SOCK_BUFFERSIZE bytes at a time
			got = recv(clientfd, buf, SOCK_BUFFERSIZE, 0);
			
			// break if nothing received
			if (got <= 0) break;
			
			// save string to return later
			buffer->append((char*)buf, got);
			
			// if the size is 1, try to parse the size out of the header
			if (size == 1)
			{
				// TODO: better way to do this (probably best just to move to libcurl)
				int lpos = buffer->find("Content-Length: ");
				int rpos = buffer->find("\r\n", lpos);
				if (lpos <=0 || rpos <= 0)
				{
					size = 2;
					continue;
				}
				std::string size_string = buffer->substr(lpos+16, rpos-lpos);
				size = std::stoi(size_string);		// get size out of header
			}
			
			// update total downloaded so far
			cumulative += got;
			
			// update progress bar, if present
			if (networking_callback != NULL)
				networking_callback(cumulative / (size + 0.0f));
		}
	
		close(clientfd);
	
		// chop off the header (and also totally ignore anything it said)
		int pos = buffer->find("\r\n\r\n");
		buffer->erase(0, (pos == std::string::npos)? buffer->size() : pos+4);

		return !buffer->empty() && *buffer != "404";
	
	#else
		// below code uses libcurl, not available on the switch
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

			if (*buffer == "" || *buffer == "404")
				return false;

			return true;
		}

	#endif
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
	file.close();
	
	#if defined(SWITCH)
		usleep(1000);
	#endif
	
	return true;
}

const char* plural(int amount)
{
	return (amount == 1)? "" : "s";
}

int init_networking()
{
	#if defined (SWITCH)
		usleep(1000);
		gfxInitDefault();
		socketInitializeDefault();
	#endif
	return 1;
}

void cp(const char* from, const char* to)
{
	std::ifstream  src(from, std::ios::binary);
	std::ofstream  dst(to,   std::ios::binary);
	
	dst << src.rdbuf();
}
