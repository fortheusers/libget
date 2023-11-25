#include <cstdio>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdarg>
#include <memory>

#include "../src/Get.hpp"
#include "../src/Utils.hpp"

using namespace std;

int main(int argc, char** args)
{
	init_networking();

	setUserAgent("get-cli/" APP_VERSION);

	// create main Get object
	Get get("./.get/", "https://switch.cdn.fortheusers.org");

	auto repos = get.getRepos();
	auto packages = get.getPackages();

	bool removeMode = false;

	for (int x = 1; x < argc; x++)
	{
		std::string cur = args[x];

		if (cur == "-x" || cur == "-d" || cur == "-r" || cur == "--delete" || cur == "--remove")
		{
			removeMode = true;
		}
		else if (cur == "-s" || cur == "--search")
		{
			if (argc == x + 1)
			{
				printf("--> Please provide search query\n");
				break;
			}

			// get search lower_query
			std::string query = std::string(args[x + 1]);

			for (int y = x + 2; y < argc; y++)
				query += std::string(" ") + std::string(args[y]);

			printf("--> Searching for \"%s\" in all repos...\n", query.c_str());

			auto results = get.search(query);

			for (int y = 0; y < results.size(); y++)
				printf("\t%s %s\n", results[y].statusString(), results[y].toString().c_str());

			break;
		}
		else if (cur == "-l" || cur == "--list")
		{
			// list available remote packages
			printf("--> Listing available remotes and packages\n");

			printf("%zu repo%s loaded!\n", repos.size(), plural(repos.size()));
			int enabledCount = 0;
			for (int x = 0; x < repos.size(); x++) {
				printf("\t%s\n", repos[x]->toString().c_str());
				if (repos[x]->isLoaded() && repos[x]->isEnabled()) {
					enabledCount++;
				}
			}

			if (enabledCount == 0) {
				printf("--> No valid repos were enabled or available! Try -o to run in offline mode.\n");
				break;
			}

			printf("%zu package%s available!\n", packages.size(), plural(packages.size()));
			for (int x = 0; x < packages.size(); x++)
				printf("\t%s %s\n", packages[x]->statusString(), packages[x]->toString().c_str());

			int count = 0;
			int updatecount = 0;
			for (int x = 0; x < packages.size(); x++)
			{
				if (packages[x]->getStatus() != GET)
					count++;
				if (packages[x]->getStatus() == UPDATE)
					updatecount++;
			}
			printf("%d package%s installed\n", count, plural(count));
			printf("%d update%s available\n", updatecount, plural(updatecount));
		}
		else if (cur == "-o" || cur == "--offline") {
			// add the local repo to list locally installed packages
			get.addLocalRepo();
			repos = get.getRepos();
			packages = get.getPackages();
		}
		else if (cur == "-c" || cur == "--clone") {
			// download all packages directly (TODO: save repo json)
			printf("--> Cloning all packages...\n");
			for (auto & cur_package: packages) {
					get.install(*cur_package);
			}
			printf("--> Cloned %zu packages!\n", packages.size());
		}
		else // assume argument is a package
		{
			// try to find the package in a local repo
			// TODO: use a hash map to improve speed
			bool found = false;

			for (auto & cur_package: packages)
			{
				if (cur_package->getPackageName() == cur)
				{
					found = true;

					if (removeMode)
					{
						// remove flag was specified, delete this package
						get.remove(*cur_package);
						break;
					}

					get.install(*cur_package);
					break;
				}
			}

			if (!found)
				printf("--> No package named [%s] found in enabled repos!\n", cur.c_str());
		}
	}

	return 0;
}