#ifndef MANIFEST_H_
#define MANIFEST_H_
#include <fstream>
#include <regex>
#include <string>
#include <sys/stat.h>
#include <vector>

void info(const char* format, ...);

enum ManifestOp
{
	MEXTRACT,
	MGET,
	MUPDATE,
	MLOCAL,
	NOP
};

struct ManifestState
{
	std::string raw;
	std::string path;
	std::string zip_path;
	std::string extension = "?";
	enum ManifestOp operation;
};

class Manifest
{
public:
	Manifest() = default;
	Manifest(const std::vector<std::string>& paths, std::string_view root_path);

	Manifest(std::string_view ManifestPath, std::string_view root_path);

	[[maybe_unused]] std::vector<ManifestState> regexMatchPath(std::string_view expression);

	[[nodiscard]] bool isValid() const
	{
		return valid;
	}

	[[nodiscard]] bool isFakeManifestPossible() const
	{
		return fakeManifestPossible;
	}

	[[nodiscard]] const std::vector<ManifestState>& getEntries() const
	{
		return entries;
	}

private:
	std::vector<ManifestState> entries;
	bool valid = false;
	bool fakeManifestPossible = false;

	// ManifestState regexMatchPathFirst(std::string expression)
	// {
	//     for (size_t i = 0; i < entries.size(); i++)
	//     {
	//         if(std::regex_match(entries[i].path, std::regex(expression)))
	//         {
	//             return entries[i];
	//         }
	//     }
	// }
};

#endif
