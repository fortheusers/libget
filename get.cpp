#include <cstdio>
#include <vector>
#include "Repo.hpp"
#include "codes.h"

using namespace std;

vector<Repo> repos;

/**
Load any repos from a config file into the repos vector.
**/
void loadRepos(char* config_path)
{
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
	
	return 0;
}