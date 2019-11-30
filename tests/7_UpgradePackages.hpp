#include "tests.hpp"

class UpgradePackages : public Test {
	public:
	UpgradePackages() {
		purpose = "Upgrade a package with different file structure";
	}
	bool execute()
	{
		get->toggleRepo(get->repos[1]);

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

		get->toggleRepo(get->repos[3]);

		// this new repo has a collision for the "one" package, with a newer update available

		if (count(get, INSTALLED) != 2 || count(get, GET) != 1 || count(get, UPDATE) != 1)
		{
			error << "There should be 2 packages installed and 1 GET, 1 UPDATE available, found " << count(get, INSTALLED) << ", " << count(get, GET) << ", and " << count(get, UPDATE) << endl;
			return false;
		}

		return true;
	}
};