#include "tests.hpp"
#include "../src/Utils.hpp"

class PackagesSortedTest : public Test {
	public:
	PackagesSortedTest() {
		// TODO: support other sort modes directly in libget (currently in hb-appstore)
		// also TODO: the code doesn't do sorting at all! so implement that so this test passes
		purpose = "Packages return in alphabetically sorted order";
	}
	bool execute()
	{
		// enable all test repos
		for (int i = 0; i < get->getRepos().size(); i++) {
			get->getRepos()[i]->setEnabled(true);
		}
		get->update();

		// get all packages from the server
		auto packages = get->list();

		// create a sorted copy of the packages
		auto sorted = packages;
		std::sort(sorted.begin(), sorted.end(), [](const Package a, const Package b) {
			return toLower(a.getPackageName()) < toLower(b.getPackageName());
		});

		// compare the two lists
		for (int i = 0; i < packages.size(); i++) {
			if (packages[i].getPackageName() != sorted[i].getPackageName()) {
				error << "Package " << i << " is not sorted correctly" << endl;
				return false;
			}
		}

		return true;
	}
};