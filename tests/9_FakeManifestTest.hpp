#include "tests.hpp"

class FakeManifestTest : public Test {
	public:
	FakeManifestTest() {
		purpose = "If the manifest is missing, the package is still installed";
	}
	bool execute()
	{
		install(get, "missingmanifest");

		if (!exists("sdroot/image.png")) {
			error << "The downloaded file in package 'missingmanifest' on server 'c' did not successfully extract" << endl;
			return false;
		}

		std::string sum = calculateMD5("sdroot/image.png").c_str();
		const char * rightSum = "26a7965e5aa6acced42de92eeee76d7a";
		if (rightSum != sum) {
			error << "The downloaded file in package 'missingmanifest' on server 'c' has incorrect md5 sum, expected: " << rightSum << ", received: " << sum.c_str() << endl;
            return false;
        }

		// make sure the missingmanifest has "installed" state
		auto missingmanifest = get->lookup("missingmanifest");
		if (!missingmanifest || missingmanifest->getStatus() != INSTALLED) {
			error << "The package 'missingmanifest' on server 'c' was not installed" << endl;
			return false;
		}

        // install one more time, and make sure we're still all good
        install(get, "missingmanifest");

        sum = calculateMD5("sdroot/image.png").c_str();
        if (rightSum != sum) {
            error << "The redownloaded file in package 'missingmanifest' on server 'c' has incorrect md5 sum, expected: " << rightSum << ", received: " << sum.c_str() << endl;
            return false;
        }

		// TODO: test zip extractions with an extra folder or no folder directory
		// TODO: test upgrade of fake manifest packages

		return true;
	}
};