#include <cstdio>
#include <vector>
#include <fstream>
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
    
    // print info about loaded repos
    cout << repos.size() << " repos loaded!" << endl;
	for (int x=0; x<repos.size(); x++)
        cout << "\t" << repos[x]->toString() << endl;
    
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
    
    // print info about loaded packages
    cout << packages.size() << " packages loaded!" << endl;
    for (int x=0; x<packages.size(); x++)
        cout << "\t" << packages[x]->toString() << endl;
}

int main(int argc, char** args)
{
	const char* config_path = "repos.json";
	loadRepos(config_path);
	
	if (repos.size() == 0)
	{
		printf("There are no repos configured!\n");
		return ERR_NO_REPOS;
	}
        
    update();
	
	return 0;
}