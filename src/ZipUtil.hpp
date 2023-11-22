#pragma once

// use included minizip library
#include "unzip.h"
#include "zip.h"

#include <string>
#include <unordered_map>
#include <vector>
class Zip
{
public:
	Zip(const char* zipPath);
	void Close();
	~Zip();
	int AddFile(const char* internalPath, const char* path);
	int AddDir(const char* internalDir, const char* externalDir);

private:
	int Add(const char* path);
	zipFile fileToZip;
};
class UnZip
{
public:
	UnZip(const char* zipPath);
	~UnZip();
	void Close();
	int Extract(const char* path, unz_file_info_s* fileInfo, unz_file_pos* file_pos = NULL);
	int ExtractFile(const char* internalPath, const char* path);
	int ExtractAll(const char* dirToExtract);
	int ExtractDir(const char* internalDir, const char* externalDir);
	std::vector<std::string> PathDump();
	std::unordered_map<std::string, unz_file_pos> GetPathToFilePosMapping();

private:
	std::string GetFileName(unz_file_info_s* fileInfo);
	std::string GetFullFileName(unz_file_info_s* fileInfo);
	unz_file_info_s* GetFileInfo();
	unzFile fileToUnzip;
};
