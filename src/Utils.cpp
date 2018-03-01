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

#if defined(__linux__) || defined(__APPLE__)
	// these platforms use curl
	#include <curl/curl.h>
	#include <curl/easy.h>

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <sys/ioctl.h>
#else
	// custom impl (for switch)
	extern "C"
	{
		#include<libtransistor/ipc/bsd.h>
		#include <sys/fcntl.h>
		#include<libtransistor/nx.h>
		#define make_ip(a,b,c,d)	((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
	}
	#include <sstream>
#endif

#include "Utils.hpp"

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
	// these platforms have curl, use that
	#if defined(__linux__) || defined(__APPLE__)
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
	#else
		// these platforms don't have curl, manually download HTTP over bsd sockets
		int ret = http_get_file(path, buffer);
		return ret < 0;
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
	return true;
}

const char* plural(int amount)
{
	return (amount == 1)? "" : "s";
}

int init_networking()
{
	#if defined(__linux__) || defined(__APPLE__)
		// no initialization required
	#else
		svcSleepThread(100000000);

		if(sm_init() != RESULT_OK || bsd_init() != RESULT_OK) {
			printf("failed to init bsd\n");
			return 0;
		}

		// TODO: lookup IP from DNS in repo config file(s)
		//		 and move this into the http_get_file method
		server_addr.sin_addr.s_addr = make_ip(192, 168, 1, 104);
		server_addr.sin_port = htons(80);

		http_stdout._write = stdout_http;
		http_stdout._flags = __SWR | __SNBF;
		http_stdout._bf._base = (unsigned char*)1;


		printf("setup networking\n");
	#endif
	return 1;
}

// everything below here is switch/other OS
// ideally all of the below would be removed to use libcurl
#if defined(__linux__) || defined(__APPLE__)
#else
static FILE http_stdout;
static const char http_get_template[] = "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: appstore-nx\r\nAccept-Encoding: none\r\nConnection: close\r\n\r\n";

char server_ip_addr[4] = {127, 0, 0, 1};

static struct sockaddr_in server_addr =
{
	.sin_family = AF_INET,
	.sin_port = htons(80),
	.sin_addr = {
		.s_addr = *(uint32_t*) server_ip_addr
	}
};

// -temporary

static int http_socket;

static int stdout_http(struct _reent *reent, void *v, const char *ptr, int len)
{
	bsd_send(http_socket, ptr, len, 0);
	return len;
}

static int parse_header(char *buf, int len, int *offset, int *got)
{
	char *ptr = buf;
	char *pptr = buf;
	int ret;
	int state = 0;
	int content_length = 0;

	while(len)
	{
		// get some bytes
		ret = bsd_recv(http_socket, ptr, len, 0);
		if(ret <= 0)
			return -1;
		ptr += ret;
		// parse line(s)
		while(1)
		{
			char *tptr = pptr;
			char *eptr = pptr + ret;
			// get line end
			while(tptr < eptr)
			{
				if(*tptr == '\n')
				{
					if(tptr - pptr <= 1)
					{
						// empty line, header ending
						if(state)
						{
							*offset = (int)((tptr + 1) - buf);
							*got = (int)((ptr - buf) - *offset);
							return content_length;
						} else
							return -2;
					}
					// got one line, parse it
					if(state)
					{
						if(!strncmp(pptr, "Content-Length:", 15))
							sscanf(pptr + 15, "%d", &content_length);
					} else
					{
						int v1, v2, res;
						// HTTP response
						state = 1;
						if(sscanf(pptr, "HTTP/%d.%d %d", &v1, &v2, &res) != 3 || !res)
							return -1;
						if(res != 200)
							return -res;
					}
					// go next
					pptr = tptr + 1;
					break;
				}
				tptr++;
			}
			if(tptr == pptr)
				// no more lines left
				break;
		}
		// go next
		len -= ret;
	}
	
	return -1;
}

int http_get_file(std::string path, std::string* buff)
{
	char temp[1024];
	int ret, offs, got;
	int size;

	http_socket = bsd_socket(2, 1, 6); // AF_INET, SOCK_STREAM, PROTO_TCP

	if(http_socket < 0)
		return -1;


	if(bsd_connect(http_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
	{
		bsd_close(http_socket);
		return -2;
	}


	// make a request
	// TODO: don't hardcode IP
	fprintf(&http_stdout, http_get_template, &(path.c_str()[20]), "192.168.1.104");

	// get an answer
	ret = parse_header(temp, sizeof(temp), &offs, &got);

	char* ptr;
	char resp[ret+1];

	// load it now
	if(ret > 0)
	{
		size = ret;
		ptr = resp;
		if(got)
		{
			// copy over already downloaded data, and remove header from total size
			memcpy(ptr, temp+offs, got);
			ptr += got;
			size -= got;
		}
		while(size)
		{
			// receive some more bytes from the socket (after first 1024 (including header))
			got = bsd_recv(http_socket, ptr, size, 0);
			if(got <= 0)
			{
				bsd_close(http_socket);
				printf("- read error\n");
				return -4;
			}
			size -= got;
			ptr = ptr + got;
		}
		printf("- file loaded\n");
	}

	bsd_close(http_socket);
	
	buff->append(resp, ret);

	return ret;
}
// -temporary
#endif
