#include "tests.hpp"

class InstallPackages2 : public Test {
    public:
    InstallPackages2() {
        purpose = "Install packages from 2nd repo";
    }
	bool execute()
    {
        // enable the second repo
        get->toggleRepo(get->repos[1]);

        // make sure that there are 5 total packages
        //    if (packages.size() != 5)
        //    {
        //        cout << "There should be 5 packages enabled, found " << packages.size() << endl;
        //        return 2;
        //    }

        // there should be 1 installed package, 3 get packages, and 1 update (TODO: add update between multiple repos)
        if (count(get, INSTALLED) != 2 /*2*/ || count(get, GET) != 3 || count(get, UPDATE) != 0 /*1*/)
        {
            error << "There should be 1 package installed and 1 update, found " << count(get, INSTALLED) << endl;
            return false;
        }

        // install one of the new packages, upgrade the other
        install(get, "three");
        install(get, "four");

        // there should be 3 installed packages, 2 get packages, and 0 updates
        if (count(get, INSTALLED) != 3 || count(get, GET) != 2 || count(get, UPDATE) != 0)
        {
            error << "There should be 3 packages installed, found " << count(get, INSTALLED) << endl;
            return false;
        }

        return true;
    }
};