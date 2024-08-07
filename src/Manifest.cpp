#include "Manifest.hpp"

Manifest::Manifest(const std::vector<std::string>& paths, std::string_view root_path)
{
	for (const auto& path : paths)
	{
		std::string ExtractPath = std::string(root_path) + path;
		ManifestState CurrentPath;
		CurrentPath.path = ExtractPath;
		CurrentPath.raw = "U: " + path;
		CurrentPath.zip_path = path;
		CurrentPath.operation = MUPDATE;
		std::string::size_type idx = path.rfind('.');
		if (idx != std::string::npos)
		{
			CurrentPath.extension = path.substr(idx + 1);
		}
		entries.push_back(CurrentPath);
	}
	valid = true;
	fakeManifestPossible = true;
}

Manifest::Manifest(std::string_view ManifestPath, std::string_view root_path)
{
	fakeManifestPossible = true;
	struct stat buf = {};

	// check if file is empty
	if (stat(ManifestPath.data(), &buf) == 0 && buf.st_size == 0)
	{
		// empty file, could still be faked but is invalid
		return;
	}

	if (stat(ManifestPath.data(), &buf) == 0)
	{
		std::ifstream ManifestFile;
		ManifestFile.open(ManifestPath.data());
		if (ManifestFile.good())
		{
			std::string CurrentLine;

			while (std::getline(ManifestFile, CurrentLine))
			{
				char ModeChar = CurrentLine.at(0);
				enum ManifestOp mode;

				switch (ModeChar)
				{
				case 'E': // EXT
					mode = MEXTRACT;
					break;
				case 'U': // UPD
					mode = MUPDATE;
					break;
				case 'G': // GET
					mode = MGET;
					break;
				case 'L': // LOC
					mode = MLOCAL;
					break;
				default:
					info("%s : Ignored in manifest\n", CurrentLine.c_str());
					mode = NOP;
					break;
				}
				ManifestState CurrentLState;
				if (mode != NOP)
				{
					std::string Path = CurrentLine.substr(3);
					std::string ExtractPath = std::string(root_path) + Path;

					CurrentLState.operation = mode;
					CurrentLState.path = ExtractPath;
					CurrentLState.raw = CurrentLine;
					CurrentLState.zip_path = Path;
					std::string::size_type idx = Path.rfind('.');
					if (idx != std::string::npos)
						CurrentLState.extension = Path.substr(idx + 1);
					entries.push_back(CurrentLState);
				}
			}
			valid = true;
		} else {
			fakeManifestPossible = false;
		}
		ManifestFile.close();
	}
}

std::vector<ManifestState> Manifest::regexMatchPath(std::string_view expression)
{
	std::vector<ManifestState> vec;
	for (auto& entry : entries)
	{
		if (std::regex_match(entry.path, std::regex(expression.data())))
		{
			vec.push_back(entry);
		}
	}
	return vec;
}
