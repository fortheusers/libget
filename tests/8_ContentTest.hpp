#include "tests.hpp"

class ContentTest : public Test {
	public:
	ContentTest() {
		purpose = "Installed content has proper data";
	}
	bool execute()
	{
		install(get, "raichu");

		std::string sum = calculateMD5("sdroot/image.jpg").c_str();
		const char * rightSum = "737b4d9cc7a7957aaa28d7b088e5acb8";
		if (rightSum != sum) {
			error << "The downloaded file in package 'raichu' on server 'c' has incorrect md5 sum, expected: " << rightSum << ", received: " << sum.c_str() << endl;
      		return false;
		}

		// install one more time, and make sure we're still all good
		install(get, "raichu");

		sum = calculateMD5("sdroot/image.jpg").c_str();
		if (rightSum != sum) {
			error << "The redownloaded file in package 'raichu' on server 'c' has incorrect md5 sum, expected: " << rightSum << ", received: " << sum.c_str() << endl;
			return false;
		}

		return true;
	}
};