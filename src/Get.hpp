#include <vector>
#include "Repo.hpp"
#include "constants.h"

class Get
{
public:
    // constructor takes path to the .get directory, and a fallback default repo url
    Get(const char* config_dir, const char* defaultRepo);
    
    int install(Package* pkg_name);    // download the given package name and manifest data
    int remove(Package* pkg_name);      // delete and remove all files for the given package name
    int toggleRepo(Repo* repo);         // enable/disable the specified repo (and write changes)
    
    // the remote repos and packages
    std::vector<Repo*> repos;
    std::vector<Package*> packages;
    
    // TODO: add queue functionality
    //    void enqueue(int count, ...)  // add a number of packages to the download queue
    //    void downloadAll()            // download all of the queued packages
    
private:
    void loadRepos(const char* config_path);
    void update();
    int validateRepos();
    
    const char* defaultRepo;
};
