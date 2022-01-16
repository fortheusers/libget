#include <cstdarg>
#include "tests.hpp"

// this is a "good enough" way to do this
// (really– there are actual testing frameworks that demand more, (and also blood))
#include "1_InitCheck.hpp"
#include "2_CheckPackages.hpp"
#include "3_InstallPackages.hpp"
#include "4_InstallPackages2.hpp"
#include "5_Search.hpp"
#include "6_RemovePackages.hpp"
#include "7_UpgradePackages.hpp"
#include "8_ContentTest.hpp"

using namespace std;

// This testing suite depends on a server to serve the contents of
// the "server" folder. There's a server.sh script that will do this

void summarize(Test* test)
{
	if (test->passed) {
		cout << "✅ Test [" << test->purpose << "] passed!" << endl;
	} else {
		cout << "❌ Test [" << test->purpose << "] failed: " << test->error.str() << endl;
	}
}

int main()
{
	init_networking();

	// create a Get object
	Get* get = new Get("./tests/.get/", URL "a");

	// all the tests are "system" tests, even though this
	// main test suite is linked against the libget library (and so
	// it's not a true end-to-end test), the tests depend on the previous
	// tests' states, and also don't clean themselves up.
	// due to this dependency, if one fails, the rest don't execute
	// they will always run in the order defined here, though
	vector<Test*> tests = {
		new InitCheck(),
		new CheckPackages(),
		new InstallPackages(),
		new InstallPackages2(),
		new Search(),
		new RemovePackages(),
		new UpgradePackages(),
    new ContentTest()
	};

	// main test loop that goes through all our tests, and prints out
	// their status with a happy friendly emoji
	int count = 0;
	int failCount = 0;

	for (auto& test : tests)
	{
		test->get = get;
		count += 1;

		cout << "---------------------------" << endl;
		cout << " Test #" << count << " - " << test->purpose << endl;
		cout << "---------------------------" << endl;

		// actually execute test and get status
		bool success = test->execute();
		test->passed = success;

		cout << "---------------------------" << endl;
		summarize(test);
		cout << "---------------------------" << endl << endl;

		// update failcount with status
		failCount += !success;
	}

	// summarize the information at the bottom
	cout << "Test Suite Summary!" << endl;
	cout << "===========================" << endl;
	for (auto& test : tests)
		summarize(test);
	cout << "===========================" << endl;
	cout << count << " tests executed, " << (count - failCount) << " passed, " << failCount << " failed." << endl;

	// return the count of failures (0 - we're good!)
	return failCount;
}

void info(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
}