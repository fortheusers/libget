#include "tests.hpp"

class Search : public Test {
    public:
    Search() {
        purpose = "Search for a package";
    }
	bool execute()
    {
        // try to do a search for a known string
        auto results = get->search("t");

        // must be size two ("two", and "three")
        if (results.size() != 2)
        {
            error << "Searching for \"t\" returned " << results.size() << " results instead of 2" << endl;
            return false;
        }

        return true;
    }
};