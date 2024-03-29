#include "tests.hpp"

class UpgradePackages : public Test {
	public:
	UpgradePackages() {
		purpose = "Upgrade a package with different file structure";
	}
	bool execute()
	{
		get->toggleRepo(*get->getRepos()[1]);

		if (count(get, INSTALLED) != 2 || count(get, GET) != 1)
		{
			error << "There should be 2 packages installed and 1 GET available, found " << count(get, INSTALLED) << " and " << count(get, GET) << endl;
			return false;
		}
		
		// get the package prior to the update
		install(get, "one");

		if (count(get, INSTALLED) != 3 || count(get, GET) != 0 || count(get, UPDATE) != 0)
		{
			error << "There should be 3 packages installed and no GET or UPDATE available, found " << count(get, INSTALLED) << ", " << count(get, GET) << ", and " << count(get, UPDATE) << endl;
			return false;
		}

		get->toggleRepo(*get->getRepos()[3]);

		// this new repo has a collision for the "one" package, with a newer update available

		if (count(get, INSTALLED) != 2 || count(get, GET) != 1 || count(get, UPDATE) != 1)
		{
			error << "There should be 2 packages installed and 1 GET, 1 UPDATE available, found " << count(get, INSTALLED) << ", " << count(get, GET) << ", and " << count(get, UPDATE) << endl;
			return false;
		}

		// check some files exist and don't exist given the un-updated version of the package
		bool ok = true;
		ok &= exists("sdroot/dir2/x");
		ok &= exists("sdroot/dir2/s");
		ok &= exists("sdroot/dir2/z");
		ok &= exists("sdroot/dir2/dir3/dogs");
		ok &= exists("sdroot/dir1/a");
		ok &= !exists("sdroot/newdir/dogs");
		ok &= !exists("sdroot/dir2/mario");

		if (!ok) {
			error << "(1) Some files that weren't expected to be installed were installed" << endl;
			return false;
		}

		// do the upgrade
		install(get, "one");

		if (count(get, INSTALLED) != 3 || count(get, GET) != 1 || count(get, UPDATE) != 0)
		{
			error << "There should be 3 packages installed and 1 GET, no UPDATE available, found " << count(get, INSTALLED) << ", " << count(get, GET) << ", and " << count(get, UPDATE) << endl;
			return false;
		}

		ok = true;
		ok &= exists("sdroot/dir2/x");
		ok &= exists("sdroot/newdir/dogs");
		ok &= exists("sdroot/dir2/mario");
		ok &= !exists("sdroot/dir2/s");
		ok &= !exists("sdroot/dir2/z");
		ok &= !exists("sdroot/dir2/dir3/dogs");
		ok &= !exists("sdroot/dir1/a");

		if (!ok) {
			error << "(2) After upgrading, some unexpected files were left behind" << endl;
			return false;
		}

		// clean up the rest of the files directly on sdroot
		remove("sdroot/dir2/x");
		remove("sdroot/newdir/dogs");
		remove("sdroot/dir2/mario");
		
		return true;
	}
};