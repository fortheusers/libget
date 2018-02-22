#include "../src/Get.hpp"
#include "../src/Utils.hpp"
#include <cstdio>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>

using namespace std;

int main(int argc, char** args)
{
    // create main Get object
    Get* get = new Get("./.get/", "http://switchbru.com/appstore");
        
    vector<Repo*> repos = get->repos;
    vector<Package*> packages = get->packages;
    
    bool removeMode = false;
    
    for (int x=1; x<argc; x++)
    {
        std::string cur = args[x];
        
        if (cur == "--delete" || cur == "--remove")
        {
            removeMode = true;
        }
        else if (cur == "-l" || cur == "--list")
        {
            // list available remote packages
            cout << "--> Listing available remotes and packages" << endl;
            
            cout << repos.size() << " repo" << plural(repos.size()) << " loaded!" << endl;
            for (int x=0; x<repos.size(); x++)
                cout << "\t" << repos[x]->toString() << endl;
            
            cout << packages.size() << " package" << plural(packages.size()) << " available!" << endl;
            for (int x=0; x<packages.size(); x++)
                cout << "\t" << packages[x]->statusString() << " " << packages[x]->toString() << endl;
            
            //            cout << "--> Listing installed packages" << endl;
            int count = 0;
            int updatecount = 0;
            for (int x=0; x<packages.size(); x++)
            {
                if (packages[x]->status != GET)
                    count++;
                if (packages[x]->status == UPDATE)
                    updatecount++;
            }
            cout << count << " package" << plural(count) << " installed" << endl << updatecount << " update" << plural(updatecount) << " available" << endl;
            
        }
        else // assume argument is a package
        {
            // try to find the package in a local repo
            // TODO: use a hash map to improve speed
            bool found = false;
            
            for (int y=0; y<packages.size(); y++)
            {
                if (packages[y]->pkg_name == cur)
                {
                    found = true;
                    
                    if (removeMode)
                    {
                        // remove flag was specified, delete this package
                        get->remove(packages[y]);
                        break;
                    }
                    
                    get->install(packages[y]);
                    break;
                }
            }
            
            if (!found)
                cout << "--> No package named [" << cur << "] found in enabled repos!" << endl;
        }
    }
    
    return 0;
}
