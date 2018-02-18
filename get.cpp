#include <cstdio>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>

#include "Repo.hpp"
#include "codes.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"

using namespace std;
using namespace rapidjson;

vector<Repo*> repos;
vector<Package*> packages;

/**
Load any repos from a config file into the repos vector.
**/
void loadRepos(const char* config_path)
{
	ifstream ifs(config_path);
    IStreamWrapper isw(ifs);
    
    if (!ifs.good())
    {
        cout << "--> Could not load repos from " << config_path << endl;
        return;
    }
    
    Document doc;
    doc.ParseStream(isw);
	
	const Value& repos_doc = doc["repos"];
	
	// for every repo
	for(Value::ConstValueIterator it=repos_doc.Begin(); it != repos_doc.End(); it++)
	{
		Repo* repo = new Repo();
		repo->name = (*it)["name"].GetString();
		repo->url = (*it)["url"].GetString();
		repo->enabled = (*it)["enabled"].GetBool();
		repos.push_back(repo);
	}
    
	return;
}

void update()
{
    // fetch recent package list from enabled repos
	for (int x=0; x<repos.size(); x++)
    {
		if (repos[x]->enabled)
            repos[x]->loadPackages(&packages);
    }
}

int validateRepos()
{
    if (repos.size() == 0)
	{
		printf("There are no repos configured!\n");
		return ERR_NO_REPOS;
	}
    
    return 0;
}

int main(int argc, char** args)
{
    // the path for the get metadata folder
	string config_path = "./.get/";
    string repo_file = config_path + "repos.json";
    string package_dir = config_path + "packages";
    string tmp_dir = config_path + "tmp";
    
//    printf("--> Using \"./sdroot\" as local download root directory\n");
//    mkdir("./sdroot", 0700);
    
    mkdir(config_path.c_str(), 0700);
    mkdir(package_dir.c_str(), 0700);
    mkdir(tmp_dir.c_str(), 0700);
    
    cout << "--> Using \"" << repo_file << "\" as repo list" << endl;
    
    // load repo info
    loadRepos(repo_file.c_str());
    update();
    
    bool removeMode = false;
    
    for (int x=1; x<argc; x++)
    {
        std::string cur = args[x];
        
        if (cur == "--delete")
        {  
            removeMode = true;
        }
        else if (cur == "-l")
        {
            // list available remote packages
            cout << "--> Listing available remotes and  packages" << endl;
            
            cout << repos.size() << " repos loaded!" << endl;
            for (int x=0; x<repos.size(); x++)
                cout << "\t" << repos[x]->toString() << endl;
            
            cout << packages.size() << " packages loaded!" << endl;
            for (int x=0; x<packages.size(); x++)
                cout << "\t" << packages[x]->toString() << endl;
        }
        else // assume argument is a package
        {
            // try to find the package in a local repo
            // TODO: use a hash map to improve speed
            bool found = false;
            
            // 
            
            for (int y=0; y<packages.size(); y++)
            {
                if (packages[y]->pkg_name == cur)
                {
                    found = true;
                    
                    if (removeMode)
                    {
                        // remove flag was specified, delete this package
                        packages[y]->remove();
                        cout << "--> Uninstalled [" << cur << "] package" << endl;
                        break;
                    }
                        
                    // found package in a remote server, fetch it
                    bool located = packages[y]->downloadZip();
                    
                    if (!located)
                    {
                        // according to the repo list, the package zip file should've been here
                        // but we got a 404 and couldn't find it
                        cout << "--> Error retreiving remote file for [" << cur << "] (check network or 404 error?)" << endl;
                        break;
                    }
                    
                    // install the package, (extracts manifest, etc)
                    packages[y]->install();
                    
                    cout << "--> Downloaded [" << cur << "] to sdroot/" << endl;
                    
                    break;
                }
            }
            
            if (!found)
                cout << "--> No package named [" << cur << "] found in enabled repos!" << endl;
        }
    }
	
	return 0;
}