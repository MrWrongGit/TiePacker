#ifndef __LIBTIEX_H__
#define __LIBTIEX_H__

#include <fstream>

class LibTieX 
{
public:
    LibTieX();
    static bool folderUnpack(std::string tie_file_path, std::string folder_path);
    static bool folderUnpack(FILE* ifp, std::string root_path, uint64_t start_addr);
    static bool fileLocate(std::string tie_file_path, std::string file_path, uint64_t &start_addr, uint64_t &size);
    static bool fileRecovery(FILE* ifp, std::string file_path, uint64_t start_addr);
};
#endif