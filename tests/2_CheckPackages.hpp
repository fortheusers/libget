#include "tests.hpp"

class CheckPackages : public Test {
    public:
    CheckPackages() {
        purpose = "Check packages loaded";
    }
	bool execute()
    {
        // make sure there are 3 installed packages
        if (get->packages.size() != 3)
        {
            error << "There should be 3 packages enabled, found " << get->packages.size() << endl;
            return false;
        }

        return true;
    }
};