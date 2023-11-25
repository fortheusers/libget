#include "tests.hpp"

class FakeManifestUpgradeTest : public Test {
	public:
	FakeManifestUpgradeTest() {
		// depends on the previous test having succeeded
		purpose = "Packages without manifests can be upgraded";
	}
	bool execute()
	{
		// enable the 5th repo (server e)
		get->toggleRepo(*get->getRepos()[5]);

		// there should be 1 available UPDATE package, and 0 installed packages
		if (count(get, UPDATE) != 1 || count(get, INSTALLED) != 0) {
			error << "There should be 1 available UPDATE package, but there are " << count(get, UPDATE) << endl;
			error << "There should be 0 installed package, but there are " << count(get, INSTALLED) << endl;
			return false;
		}

		install(get, "missingmanifest");

		if (!exists("sdroot/image.png")) {
			error << "The downloaded file in package 'missingmanifest' on server 'e' did not successfully extract" << endl;
			return false;
		}

		// there should be 1 INSTALLED package, and 0 upgrade packages
		if (count(get, UPDATE) != 0 || count(get, INSTALLED) != 1) {
			error << "There should be 0 available UPDATE package, but there are " << count(get, UPDATE) << endl;
			error << "There should be 1 installed package, but there are " << count(get, INSTALLED) << endl;
			return false;
		}

		std::string sum = calculateMD5("sdroot/image.png");
		const char * rightSum = "26a7965e5aa6acced42de92eeee76d7a";
		if (rightSum != sum) {
			error << "The downloaded file in package 'missingmanifest' on server 'e' has incorrect md5 sum, expected: " << rightSum << ", received: " << sum.c_str() << endl;
            return false;
        }

		// make sure the missingmanifest has "installed" state
		auto missingmanifest = get->lookup("missingmanifest");
		if (!missingmanifest || missingmanifest->getStatus() != INSTALLED) {
			error << "The package 'missingmanifest' on server 'e' was not installed" << endl;
			return false;
		}

        // install one more time, and make sure we're still all good
        install(get, "missingmanifest");

        sum = calculateMD5("sdroot/image.png");
        if (rightSum != sum) {
            error << "The redownloaded file in package 'missingmanifest' on server 'e' has incorrect md5 sum, expected: " << rightSum << ", received: " << sum.c_str() << endl;
            return false;
        }

		// there should still be 1 INSTALLED package, and 0 upgrade packages
		if (count(get, UPDATE) != 0 || count(get, INSTALLED) != 1) {
			error << "There should be 0 available UPDATE package, but there are " << count(get, UPDATE) << endl;
			error << "There should be 1 installed package, but there are " << count(get, INSTALLED) << endl;
			return false;
		}

		// TODO: test zip extractions with an extra folder or no folder directory

		return true;
	}
};