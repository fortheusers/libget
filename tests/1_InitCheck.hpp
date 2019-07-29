#include "tests.hpp"

// This test assumes all repos start disabled
// currently commented out based on repos.json

class InitCheck : public Test {
    public:
    InitCheck() {
        purpose = "Initialization Check";
    }
	bool execute()
    {
    //     vector<Package*> packages = get->packages;

	//    if (packages.size() != 0)
	//    {
    //        stringstream stream;
	//        stream << "There shouldn't be any packages available, found " << packages.size();
    //        error = stream.str();
	//        return false;
	//    }
	
	//    // enable the first repo
	//    get->toggleRepo(get->repos[0]);

    cout << "Checking repo count..." << endl;

    for (int x=0; x<3; x++) {
        if (get->repos[x]->enabled == (x != 0)) {
	       error << "Repo [" << get->repos[x]->name << "] should've been " << (x == 0) << ", but was " << get->repos[x]->enabled << endl;
	       return false;
        }
    }

    cout << "Get library and repos initialized successfully" << endl;

       return true;
    }
};