#include "tests.hpp"
#include "../src/Utils.hpp"

// this test removes all the test repos, and then adds the two production CDN ones
// using the same method (addAndRemoveReposByURL) used by the hbas meta-repo
class RepoAddAndRemoveTest : public Test {
	public:
	RepoAddAndRemoveTest() {
		purpose = "Bulk add and remove repos (meta-repo test)";
	}
	bool execute()
	{
		// get all the repos
		auto repos = get->getRepos();

		// add their URLs to the remove list
		std::unordered_set<std::string> removeSet;
		for (auto& repo : repos)
			removeSet.insert(repo->getUrl());
		
		// create some test repos to add
		std::unordered_set<std::string> addSet;
		addSet.insert("https://wiiu.cdn.fortheusers.org");
		addSet.insert("https://switch.cdn.fortheusers.org");

		// call the add and remove functions
		get->addAndRemoveReposByURL(addSet, removeSet);

		// we should end up with only the repos we added
		repos = get->getRepos();
		if (repos.size() != addSet.size())
		{
			error << "Expected " << addSet.size() << " repos, got " << repos.size();
			return false;
		}

		// check that there are like, a lot of packages (since these are the real repos)
		int packageCount = get->getPackages().size();

		if (packageCount < 200)
		{
			error << "Expected at least 200 packages, got " << packageCount;
			return false;
		}

		// check that our repo json looks how we expect
		auto path = get->mRepos_path;
		std::string expectedFile = R""""({"repos":[{"name":"switch.cdn.fortheusers.org","url":"https://switch.cdn.fortheusers.org","type":"get","enabled":true},{"name":"wiiu.cdn.fortheusers.org","url":"https://wiiu.cdn.fortheusers.org","type":"get","enabled":true}]})"""";

		std::ifstream t(path);
		std::stringstream buffer;
		buffer << t.rdbuf();
		std::string fileContents = buffer.str();

		// compare the file to what we expect
		if (fileContents != expectedFile)
		{
			error << "Expected file to be:\n" << expectedFile << "\n\nGot:\n" << fileContents;
			return false;
		}

		// remove one of the repos we added, again using a repo set
		removeSet.clear();
		removeSet.insert("https://switch.cdn.fortheusers.org");
		get->addAndRemoveReposByURL({}, removeSet);

		// make sure we only have one repo left
		repos = get->getRepos();
		if (repos.size() != 1)
		{
			error << "Expected 1 repo, got " << repos.size();
			return false;
		}

		// make sure it's the right one
		if (repos[0]->getUrl() != "https://wiiu.cdn.fortheusers.org")
		{
			error << "Expected repo to be https://wiiu.cdn.fortheusers.org, got " << repos[0]->getUrl();
			return false;
		}

		// and just make sure we have less packages, as a sanity check
		int wiiuPackagesCount = get->getPackages().size();

		if (wiiuPackagesCount > packageCount)
		{
			error << "Expected less packages, got " << wiiuPackagesCount;
			return false;
		}

		expectedFile = R""""({"repos":[{"name":"wiiu.cdn.fortheusers.org","url":"https://wiiu.cdn.fortheusers.org","type":"get","enabled":true}]})"""";
		t = std::ifstream(path);
		buffer = std::stringstream();
		buffer << t.rdbuf();
		fileContents = buffer.str();

		if (fileContents != expectedFile)
		{
			error << "Expected file to be:\n" << expectedFile << "\n\nGot:\n" << fileContents;
			return false;
		}
		

		return true;
	}
};