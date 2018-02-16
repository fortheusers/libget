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

/**
Load any repos from a config file into the repos vector.
**/
void loadRepos(char* config_path)
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
	
	return;
}

int main(int argc, char** args)
{
	char* config_path = "repos.json";
	loadRepos(config_path);
	
	if (repos.size() == 0)
	{
		printf("There are no repos configured!\n");
		return ERR_NO_REPOS;
	}
	
	// fetch recent package list from enabled repos
	for (int x=0; x<repos.size(); x++)
		cout << repos[x]->toString() << endl;
	
	return 0;
}