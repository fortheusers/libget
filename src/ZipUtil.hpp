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
	explicit Zip(const std::string& zipPath);
	void Close();
	~Zip();
	int AddFile(const std::string& internalPath, const std::string& path);
	int AddDir(const std::string& internalDir, const std::string& externalDir);

private:
	int Add(const std::string& path);
	zipFile fileToZip;
};
class UnZip
{
public:
	explicit UnZip(const std::string& zipPath);
	~UnZip();
	void Close();

	bool IsValid();
	int Extract(const std::string& path, unz_file_pos& file_pos);
	int Extract(const std::string& path, const unz_file_info_s& fileInfo);
	int ExtractFile(const std::string& internalPath, const std::string& path);
	int ExtractAll(const std::string& dirToExtract);
	int ExtractDir(const std::string& internalDir, const std::string& externalDir);
	std::vector<std::string> PathDump();
	std::unordered_map<std::string, unz_file_pos> GetPathToFilePosMapping();

private:
	std::string GetFileName(const unz_file_info_s& fileInfo);
	std::string GetFullFileName(const unz_file_info_s& fileInfo);
	unz_file_info_s GetFileInfo();
	unzFile fileToUnzip;
};
