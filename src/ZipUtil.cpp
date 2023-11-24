// Modified from Zarklord1 : https://github.com/Zarklord1/WiiU_Save_Manager (src/file_utils/Zip.cpp)
#include <cstdint>
#include <dirent.h>
#include <fcntl.h>
#include <malloc.h>
#include <memory>
#include <optional>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "Utils.hpp"
#include "ZipUtil.hpp"

#define u32 uint32_t
#define u8 uint8_t

#ifdef WIN32
#define aligned_alloc _aligned_malloc
#define cross_free _aligned_free
#define fsync _commit
#else
#define cross_free free
#endif

// void info(const char* format, ...);

Zip::Zip(const std::string& zipPath)
{
	fileToZip = zipOpen(zipPath.c_str(), APPEND_STATUS_CREATE);
	if (fileToZip == nullptr) printf("--> Error Opening: %s for zipping files!\n", zipPath.c_str());
}

Zip::~Zip()
{
	Close();
}

int Zip::AddFile(const std::string& internalPath, const std::string& path)
{
	zipOpenNewFileInZip(fileToZip, internalPath.c_str(), nullptr, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
	// info("Adding %s to zip, under path %s\n", path, internalPath);
	int code = Add(path);
	zipCloseFileInZip(fileToZip);
	return code;
}

int Zip::AddDir(const std::string& internalDir, const std::string& externalDir)
{
	DIR* dir = opendir(externalDir.c_str());
	if (dir == nullptr)
	{
		return -1;
	}

	struct dirent* dirent;
	while ((dirent = readdir(dir)) != nullptr)
	{
		if (strcmp(dirent->d_name, "..") == 0 || strcmp(dirent->d_name, ".") == 0)
			continue;

		std::string zipPath(internalDir);
		zipPath += '/';
		zipPath += dirent->d_name;
		std::string realPath(externalDir);
		realPath += '/';
		realPath += dirent->d_name;

		if (is_dir(externalDir, dirent))
		{
			AddDir(zipPath, realPath);
		}
		else
		{
			AddFile(zipPath, realPath);
		}
	}
	closedir(dir);
	return 0;
}

int Zip::Add(const std::string& path)
{
	int fileNumber = open(path.c_str(), O_RDONLY);
	if (fileNumber == -1)
		return -1;

	u32 filesize = lseek(fileNumber, 0, SEEK_END);
	lseek(fileNumber, 0, SEEK_SET);

	u32 blocksize = 0x80000;
	u8* buffer = (u8*)memalign(0x40, blocksize);
	if (buffer == nullptr)
	{
		close(fileNumber);
		return -2;
	}

	u32 done = 0;
	int readBytes;

	while (done < filesize)
	{
		if (done + blocksize > filesize)
		{
			blocksize = filesize - done;
		}
		readBytes = read(fileNumber, buffer, blocksize);
		if (readBytes <= 0)
			break;
		zipWriteInFileInZip(fileToZip, buffer, blocksize);
		done += readBytes;
	}
	close(fileNumber);
	free(buffer);

	if (done != filesize)
	{
		return -3;
	}
	return 0;
}

void Zip::Close()
{
	zipClose(fileToZip, nullptr);
}

UnZip::UnZip(const std::string& zipPath)
{
	fileToUnzip = unzOpen(zipPath.c_str());
}

UnZip::~UnZip()
{
	Close();
}

void UnZip::Close()
{
	unzClose(fileToUnzip);
}

int UnZip::ExtractFile(const std::string& internalPath, const std::string& path)
{
	int code = unzLocateFile(fileToUnzip, internalPath.c_str(), 1);
	if (code == UNZ_END_OF_LIST_OF_FILE)
	{
		return -1;
	}

	unz_file_info_s fileInfo = GetFileInfo();
	// info("Extracting file %s to: %s\n", internalPath.c_str(), path.c_str());
	return Extract(path, fileInfo);
}

int UnZip::ExtractDir(const std::string& internalDir, const std::string& externalDir)
{
	int i = 0;
	for (;;)
	{
		int code;
		if (i == 0)
		{
			code = unzGoToFirstFile(fileToUnzip);
			i++;
		}
		else
		{
			code = unzGoToNextFile(fileToUnzip);
		}
		if (code == UNZ_END_OF_LIST_OF_FILE)
		{
			if (i > 1)
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}
		unz_file_info_s fileInfo = GetFileInfo();

		std::string outputPath = GetFullFileName(fileInfo);
		if (outputPath.find(internalDir, 0) != 0)
		{
			continue;
		}

		outputPath.replace(0, internalDir.length(), externalDir);
		if (fileInfo.uncompressed_size != 0 && fileInfo.compression_method != 0)
		{
			// file
			i++;
			// info("Extracting %s to: %s\n", GetFullFileName(fileInfo).c_str(), outputPath.c_str());
			Extract(outputPath, fileInfo);
		}
	}
}

int UnZip::ExtractAll(const std::string& dirToExtract)
{
	int i = 0;
	for (;;)
	{
		int code;
		if (i == 0)
		{
			code = unzGoToFirstFile(fileToUnzip);
			i++;
		}
		else
		{
			code = unzGoToNextFile(fileToUnzip);
		}
		if (code == UNZ_END_OF_LIST_OF_FILE) return 0;

		unz_file_info_s fileInfo = GetFileInfo();
		std::string fileName(dirToExtract);
		fileName += '/';
		fileName += GetFullFileName(fileInfo);
		if (fileInfo.uncompressed_size != 0 && fileInfo.compression_method != 0)
		{
			// file
			//  info("Extracting %s to: %s\n", GetFullFileName(fileInfo).c_str(), fileName.c_str());
			Extract(fileName, fileInfo);
		}
	}
}

std::vector<std::string> UnZip::PathDump()
{
	int i = 0;
	std::vector<std::string> paths;
	for (;;)
	{
		int code;
		if (i == 0)
		{
			code = unzGoToFirstFile(fileToUnzip);
			i++;
		}
		else
		{
			code = unzGoToNextFile(fileToUnzip);
		}
		if (code == UNZ_END_OF_LIST_OF_FILE)
		{
			break;
		}

		unz_file_info_s fileInfo = GetFileInfo();
		std::string fileName = GetFullFileName(fileInfo);
		if (fileInfo.uncompressed_size != 0 && fileInfo.compression_method != 0)
		{
			// info("PathDump: %s\n", fileName.c_str());
			paths.push_back(fileName);
		}
	}
	return paths;
}

std::unordered_map<std::string, unz_file_pos> UnZip::GetPathToFilePosMapping()
{
	int i = 0;
	std::unordered_map<std::string, unz_file_pos> paths;
	for (;;)
	{
		int code;
		if (i == 0)
		{
			code = unzGoToFirstFile(fileToUnzip);
			i++;
		}
		else
		{
			code = unzGoToNextFile(fileToUnzip);
		}
		if (code == UNZ_END_OF_LIST_OF_FILE)
		{
			break;
		}

		unz_file_info_s fileInfo = GetFileInfo();
		std::string fileName = GetFullFileName(fileInfo);

		unz_file_pos pos = {};
		unzGetFilePos(fileToUnzip, &pos);

		// save to map
		paths[fileName] = pos;
	}
	return paths;
}

int UnZip::Extract(const std::string& path, unz_file_pos& file_pos)
{
	// we have a file pos, seek to that file
	unzGoToFilePos(fileToUnzip, &file_pos);
	auto file_info = GetFileInfo();
	return Extract(path, file_info);
}

int UnZip::Extract(const std::string& path, const unz_file_info_s& fileInfo)
{
	if (unzOpenCurrentFile(fileToUnzip) != UNZ_OK)
	{
		return -2;
	}

	size_t last_slash_idx = path.find_last_of("/\\");
	if (last_slash_idx != std::string::npos)
	{
		CreateSubfolder(path.substr(0, last_slash_idx));
	}

	u32 blocksize = 0x80000;
	u8* buffer = (u8*)aligned_alloc(0x20000, blocksize);
	if (buffer == nullptr)
	{
		unzCloseCurrentFile(fileToUnzip);
		return -3;
	}
	int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0777);
	if (fd == -1)
	{
		unzCloseCurrentFile(fileToUnzip);
		cross_free(buffer);
		return -4;
	}

	u32 done = 0;
	int writeBytes;
	while (done < fileInfo.uncompressed_size)
	{
		if (done + blocksize > fileInfo.uncompressed_size)
		{
			blocksize = fileInfo.uncompressed_size - done;
		}
		unzReadCurrentFile(fileToUnzip, buffer, blocksize);
		writeBytes = write(fd, buffer, blocksize);
		if (writeBytes <= 0)
		{
			break;
		}
		done += writeBytes;
	}

	fsync(fd);
	close(fd);
	cross_free(buffer);
	unzCloseCurrentFile(fileToUnzip);

	if (done != fileInfo.uncompressed_size)
	{
		return -4;
	}

	return 0;
}

std::string UnZip::GetFileName(const unz_file_info_s& fileInfo)
{
	auto filePath = GetFullFileName(fileInfo);
	const size_t last_slash_idx = filePath.find_last_of("/\\");
	if (last_slash_idx != std::string::npos)
	{
		return filePath.substr(last_slash_idx + 1);
	}
	return filePath;
}

std::string UnZip::GetFullFileName(const unz_file_info_s& fileInfo)
{
	std::string path;
	path.resize(fileInfo.size_filename);
	unzGetCurrentFileInfo(fileToUnzip, (unz_file_info_s*)&fileInfo, path.data(), path.size(), nullptr, 0, nullptr, 0);
	return path;
}

unz_file_info_s UnZip::GetFileInfo()
{
	unz_file_info_s fileInfo = {};
	unzGetCurrentFileInfo(fileToUnzip, &fileInfo, nullptr, 0, nullptr, 0, nullptr, 0);
	return fileInfo;
}
