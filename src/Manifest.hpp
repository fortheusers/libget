#ifndef MANIFEST_H_
#define MANIFEST_H_
#include <vector>
#include <string>
#include <regex>
#include <fstream>
#include <sys/stat.h>

enum ManifestOp{
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
    std::string extension;
    enum ManifestOp operation;
};

class Manifest
{
    public:
        std::vector<ManifestState> entries;
        bool valid = true;
        bool fakeManifestPossible = true;
        Manifest(std::vector<std::string> paths, std::string root_path)
        {
            for (size_t i = 0; i <= paths.size() - 1; i++)
            {
                std::string ExtractPath = root_path + paths[i];
                ManifestState CurrentPath;
                CurrentPath.path = ExtractPath;
                CurrentPath.raw = "U: " + paths[i];
                CurrentPath.zip_path = paths[i];
                CurrentPath.operation = MUPDATE;
                std::smatch match;
                std::regex_search(CurrentPath.zip_path, match, std::regex("[^.]+$"));
                CurrentPath.extension = match[0];
                entries.push_back(CurrentPath);
            }
        }

        Manifest(std::string ManifestPath, std::string root_path)
        {
            struct stat buf;
            if(stat(ManifestPath.c_str(), &buf) == 0){
                std::ifstream ManifestFile;
                ManifestFile.open(ManifestPath.c_str());
                if(ManifestFile.good())
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
                            printf("%s : Ignored in manifest\n", CurrentLine.c_str());
                            mode = NOP;
                            break;
                        }
                        ManifestState CurrentLState;
                        if (mode != NOP)
                        {
                            std::string Path = CurrentLine.substr(3);
                            std::string ExtractPath = root_path + Path;

                            CurrentLState.operation = mode;
                            CurrentLState.path = ExtractPath;
                            CurrentLState.raw = CurrentLine;
                            CurrentLState.zip_path = Path;
                            std::smatch match;
                            std::regex_search(Path, match, std::regex("[^.]+$"));
                            CurrentLState.extension = match[0];
                            entries.push_back(CurrentLState);
                        }
                    }
                }else{
                    this->valid = false;
                    this->fakeManifestPossible = false;
                }
                ManifestFile.close();
            }else{
                this->valid = false;
            }
        }

        std::vector<ManifestState> regexMatchPath(std::string expression)
        {
            std::vector<ManifestState> vec;
            for (size_t i = 0; i < entries.size(); i++)
            {
                if(std::regex_match(entries[i].path, std::regex(expression)))
                {
                    vec.push_back(entries[i]);
                }
            }
            return vec;
        }

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
