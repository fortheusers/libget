#include "tests.hpp"

class FakeManifestTest : public Test {
	public:
	FakeManifestTest() {
		purpose = "Packages without manifests are still installed";
	}
	bool execute()
	{
		// disable all repos
		for (int i = 0; i < get->getRepos().size(); i++) {
			printf("Disabling repo %s\n", get->getRepos()[i]->getUrl().c_str());
			get->getRepos()[i]->setEnabled(false);
		}

		// enable only the 4th repo (server d)
		get->getRepos()[4]->setEnabled(true);
		get->update();

		// there should be 1 available GET package
		if (count(get, GET) != 1) {
			error << "(1) There should be 1 GET package, but there are " << count(get, GET) << endl;
			return false;
		}

		install(get, "missingmanifest");

		if (!exists("sdroot/image.png")) {
			error << "The downloaded file in package 'missingmanifest' on server 'd' did not successfully extract" << endl;
			return false;
		}

		std::string sum = calculateMD5("sdroot/image.png");
		std::string rightSum = "26a7965e5aa6acced42de92eeee76d7a";
		if (rightSum != sum) {
			error << "The downloaded file in package 'missingmanifest' on server 'd' has incorrect md5 sum, expected: " << rightSum << ", received: " << sum << endl;
            return false;
        }

		// make sure the missingmanifest has "installed" state
		auto missingmanifest = get->lookup("missingmanifest");
		if (!missingmanifest || missingmanifest->getStatus() != INSTALLED) {
			error << "The package 'missingmanifest' on server 'd' was not installed" << endl;
			return false;
		}

		// there should be one installed package total
		if (count(get, INSTALLED) != 1) {
			error << "(2) There should be 1 installed package, but there are " << count(get, INSTALLED) << endl;
			return false;
		}

		// manually remove the image on disk, breaking the package
		// TODO: a test to show broken packages as their own state
		std::remove("sdroot/image.png");

        // install one more time, and make sure we're still all good
		// should work even though the package is broken
        install(get, "missingmanifest");

        sum = calculateMD5("sdroot/image.png");
        if (rightSum != sum) {
            error << "The redownloaded file in package 'missingmanifest' on server 'd' has incorrect md5 sum, expected: " << rightSum << ", received: " << sum.c_str() << endl;
            return false;
        }

		// TODO: test zip extractions with an extra folder or no folder directory

		return true;
	}
};