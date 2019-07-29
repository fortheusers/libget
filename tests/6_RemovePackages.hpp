#include "tests.hpp"

class RemovePackages : public Test {
    public:
    RemovePackages() {
        purpose = "Remove and verify packages";
    }
	bool execute()
    {
        // make sure that empty folders are cleaned up afterwards... when removing a package
        remove(get, "four");
        if (exists("./sdroot/folder1") || exists("./sdroot/folder2") || exists("./sdroot/folder3"))
        {
            error << "Tried to remove \"four\", but directory remained behind (should be empty)." << endl;
            return false;
        }

        // make sure it goes down (total installed/updated package count)
        int sum = count(get, INSTALLED) + count(get, UPDATE);
        if (sum != 2)
        {
            error << "There should only be 2 packages installed after removing one" << endl;
            return false;
        }

        return true;
    }
};