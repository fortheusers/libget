// operating system level utilities
// contains directory utils, http utils, and helper methods

#if defined(WII) && !defined(NETWORK_MOCK)
#include <wiisocket.h>
#endif

#if defined(SWITCH)
#include <switch.h>
#endif

#if defined(_3DS)
#include <3ds.h>
#include <malloc.h>
#endif

#if defined(WIN32)
#include <sys/types.h>
#endif

#include <algorithm>
#include <cstdint>
#include <ctime>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "Utils.hpp"

// resinfs support, if present
#if defined(USE_RAMFS)
#define RAMFS "resin:/"
#else
#define RAMFS "resin/"
#endif

#define BUF_SIZE 0x800000 // 8MB.

int (*networking_callback)(void*, double, double, double, double);
int (*libget_status_callback)(int, int, int);

// reference to the curl handle so that we can re-use the connection
#ifndef NETWORK_MOCK
CURL* curl = NULL;
#endif

#define SOCU_ALIGN 0x1000
#define SOCU_BUFFERSIZE 0x100000

#ifndef SO_TCPSACK
#define SO_TCPSACK 0x00200 /* Allow TCP SACK (Selective acknowledgment) */
#endif

#ifndef SO_WINSCALE
#define SO_WINSCALE 0x00400 /* Set scaling window option */
#endif

#ifndef SO_RCVBUF
#define SO_RCVBUF 0x01002 /* Receive buffer size */
#endif

#ifndef NETWORK_MOCK
// networking optimizations adapted from:
//  - https://github.com/samdejong86/Arria-V-ADC-Ethernet-software/blob/master/ADC_Socket_bsp/iniche/src/h/socket.h
int sockopt_callback(void* clientp, curl_socket_t curlfd, curlsocktype purpose)
{
	int winscale = 1, rcvbuf = 0x20000, tcpsack = 1;
#ifndef WIN32
	setsockopt(curlfd, SOL_SOCKET, SO_WINSCALE, &winscale, sizeof(int));
	setsockopt(curlfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(int));
	setsockopt(curlfd, SOL_SOCKET, SO_TCPSACK, &tcpsack, sizeof(int));
#endif
	return 0;
}
#endif

#if defined(_3DS)
u32* SOCUBuffer;
#endif

bool CreateSubfolder(char* cpath)
{
	std::string path(cpath);
	return mkpath(path);
}

// wrapper for unix mkdir
int my_mkdir(const char* path, int perms) {
#if defined(WIN32)
	return mkdir(path);
#else
	return mkdir(path, perms);
#endif
}

// http://stackoverflow.com/a/11366985
bool mkpath(std::string path)
{
	bool bSuccess = false;
	int nRC = my_mkdir(path.c_str(), 0775);
	if (nRC == -1)
	{
		switch (errno)
		{
		case ENOENT:
			// parent didn't exist, try to create it
			if (mkpath(path.substr(0, path.find_last_of('/'))))
				// Now, try to create again.
				bSuccess = 0 == my_mkdir(path.c_str(), 0775);
			else
				bSuccess = false;
			break;
		case EEXIST:
			// Done!
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

#ifndef NETWORK_MOCK
void setPlatformCurlFlags(CURL* c)
{
	// // from https://github.com/GaryOderNichts/wiiu-examples/blob/main/curl-https/romfs/cacert.pem
	curl_easy_setopt(c, CURLOPT_CAINFO, RAMFS "res/cacert.pem");

	curl_easy_setopt(curl, CURLOPT_SOCKOPTFUNCTION, sockopt_callback);
}
#endif

static size_t MemoryWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

static size_t DiskWriteCallback(void* contents, size_t size, size_t num_files, void* userp)
{
	ntwrk_struct_t* data_struct = (ntwrk_struct_t*)userp;
	size_t realsize = size * num_files;

	if (realsize + data_struct->offset >= data_struct->data_size)
	{
		fwrite(data_struct->data, data_struct->offset, 1, data_struct->out);
		data_struct->offset = 0;
	}

	memcpy(&data_struct->data[data_struct->offset], contents, realsize);
	data_struct->offset += realsize;
	data_struct->data[data_struct->offset] = 0;
	return realsize;
}

// https://gist.github.com/alghanmi/c5d7b761b2c9ab199157
// if data_struct is specified, file will go straight to disk as it downloads
bool downloadFileCommon(std::string path, std::string* buffer = NULL, ntwrk_struct_t* data_struct = NULL)
{
#ifndef NETWORK_MOCK
	CURLcode res;

	if (!curl)
		return false;

	setPlatformCurlFlags(curl);

	curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, networking_callback);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

	bool skipDisk = data_struct == NULL;

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, skipDisk ? MemoryWriteCallback : DiskWriteCallback);

	if (skipDisk)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
	else
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_struct);

	return curl_easy_perform(curl) == CURLE_OK;
#else
	return true;
#endif
}

bool downloadFileToMemory(std::string path, std::string* buffer)
{
	return downloadFileCommon(path, buffer, NULL);
}

bool downloadFileToDisk(std::string remote_path, std::string local_path)
{
	FILE* out_file = fopen(local_path.c_str(), "wb");
	if (!out_file)
		return false;

	uint8_t* buf = (uint8_t*)malloc(BUF_SIZE); // 8MB.
	if (buf == NULL)
	{
		fclose(out_file);
		return false;
	}

	ntwrk_struct_t data_struct = { buf, BUF_SIZE, 0, out_file };

	if (!downloadFileCommon(remote_path, NULL, &data_struct))
	{
		free(data_struct.data);
		fclose(data_struct.out);
		return false;
	}

	// write remaning data to file before free.
	fwrite(data_struct.data, data_struct.offset, 1, data_struct.out);
	free(data_struct.data);
	fclose(data_struct.out);

	return true;
}

const char* plural(int amount)
{
	return (amount == 1) ? "" : "s";
}

const std::string dir_name(std::string file_path)
{
	// turns "/hi/man/thing.txt to /hi/man"
	size_t pos = file_path.find_last_of("/");

	// no "/" in string, return empty string
	if (pos == std::string::npos)
		return "";

	return file_path.substr(0, pos);
}

// sorting function: put bigger strings at the front
bool compareLen(const std::string& a, const std::string& b)
{
	return (a.size() > b.size());
}

int init_networking()
{
#if defined(SWITCH)
	socketInitializeDefault();
#endif
#if defined(_3DS)
	SOCUBuffer = (u32*)memalign(SOCU_ALIGN, SOCU_BUFFERSIZE);
	socInit(SOCUBuffer, SOCU_BUFFERSIZE);
#endif

#if defined(WII) && !defined(NETWORK_MOCK)
	// TODO: network initialization on the wii is *extremly* slow (~10s)
	// It's probably a good idea to use wiisocket_init_async and
	// show something on the screen during that interval
	wiisocket_init();
#endif

#ifndef NETWORK_MOCK
	curl_global_init(CURL_GLOBAL_ALL);

	// init our curl handle
	curl = curl_easy_init();

#endif
	return 1;
}

int deinit_networking()
{
#ifndef NETWORK_MOCK
	curl_easy_cleanup(curl);
	curl_global_cleanup();
#endif

#if defined(WII) && !defined(NETWORK_MOCK)
	wiisocket_deinit();
#endif

#if defined(SWITCH)
	socketExit();
#endif

	return 1;
}

void cp(const char* from, const char* to)
{
	std::ifstream src(from, std::ios::binary);
	std::ofstream dst(to, std::ios::binary);

	dst << src.rdbuf();
}

std::string toLower(const std::string& str)
{
	std::string lower;
	transform(str.begin(), str.end(), std::back_inserter(lower), tolower);
	return lower;
}

int remove_empty_dirs(const char* name, int count)
{
	// from incoming path, recursively ensure all directories are deleted
	// return the number of files remaining (0 if totally erased and successful)

	// based on https://stackoverflow.com/a/8438663

	int starting_count = count;

	DIR* dir;
	struct dirent* entry;

	// already deleted
	if (!(dir = opendir(name)))
		return true;

	// go through files in directory
	while ((entry = readdir(dir)) != NULL)
	{
		if (is_dir(name, entry))
		{
			char path[1024];
			// skip current dir or parent dir
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;

			// update path so far
			snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);

			// recursively go into this directory too
			count += remove_empty_dirs(path, count);
		}
		else
		{
			// file found, increase file count
			count++;
		}
	}

	// now that we've been through this directory, check if it was an empty directory
	if (count == starting_count)
	{
		// empty, try rmdir (should only remove if empty anyway)
		rmdir(name);
	}

	closedir(dir);

	// return number of files at this level (total count minus starting)
	return count - starting_count;
}

bool libget_reset_data(const char* path)
{
	time_t seconds;
	time(&seconds);

	long int current_time = static_cast<long int>(seconds);

	// move the contents of the .get folder to .trash/get_backup_date
	std::stringstream ss;
	ss << ".trash/get_backup_" << current_time;
	printf("--> Info: %ld\n", current_time);

	printf("--> Renaming %s to %s\n", path, ss.str().c_str());

	std::stringstream ss2;
	ss2 << path << "../.trash";
	mkpath(ss2.str().c_str());

	int res = std::rename(path, ss.str().c_str());
	if (res == 0)
		printf("Folder renamed!\n");
	else
		printf("Issue renaming folder... %d: %s\n", errno, strerror(errno));

	return !res;
}

bool is_dir(const char* path, struct dirent* entry)
{
#ifndef WIN32
	return entry->d_type & DT_DIR;
#else
	// windows check, using stat
	struct stat s;
	// get full path using dir and entry
 	std::string full_path = std::string(path) + "/" + std::string(entry->d_name);
 	stat(full_path.c_str(), &s);
	return s.st_mode & S_IFDIR;
#endif
}