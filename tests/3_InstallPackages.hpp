#include "tests.hpp"

class InstallPackages : public Test {
    public:
    InstallPackages() {
        purpose = "Install packages";
    }
	bool execute()
    {
		auto& packages = get->getPackages();
        // install two of the packages
        auto threepkg = get->lookup("three");
        get->install(threepkg.value());
        
        auto twopkg = get->lookup("two");
        get->install(twopkg.value());

        // verify that some files got created
        // TODO: verify some!

        // there should be 2 installed packages, 1 get package, 0 updates
        if (count(get, INSTALLED) != 2 || count(get, GET) != 1 || count(get, UPDATE) != 0)
        {
            error << "There should be two packages installed, found " << count(get, INSTALLED) << endl;
            return false;
        }

        return true;
    }
};