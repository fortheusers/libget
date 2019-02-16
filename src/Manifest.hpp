#ifndef MANIFEST_H_
#define MANIFEST_H_
#include "ManifestEntry.hpp"
#include <vector>
class Manifest {
    public:
        std::vector<ManifestEntry*> entries;
        Manifest() {} //TODO: Fill out manifest class
};
#endif