#include <stdio.h>
#include <string>

// the struct to be passed in the write function.
typedef struct
{
    uint8_t *data;
    size_t data_size;
    u_int64_t offset;
    FILE *out;
} ntwrk_struct_t;


// folder stuff
bool mkpath(std::string path);
bool CreateSubfolder(char* cstringpath);

// networking stuff
int init_networking();
bool downloadFileToMemory(std::string path, ntwrk_struct_t *data_struct); // writes to disk in BUF_SIZE chunks.
bool downloadFileToDisk(std::string remote_path, std::string local_path); // saves file to local_path.

// callback for networking progress
// if set, will be invoked during the download
extern int (*networking_callback)(void*, double, double, double, double);

// helper methods
const char* plural(int amount);
void cp(const char* from, const char* to);
std::string toLower(const std::string& str);
int remove_empty_dirs(const char* name, int count);

const std::string dir_name(std::string file_path);
bool compareLen(const std::string& a, const std::string& b);
