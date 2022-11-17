#ifndef __LIBTIEC_H__
#define __LIBTIEC_H__

#include <fstream>

class LibTieC 
{
public:
    LibTieC();
    static bool folderPack(std::string folder_path, std::string tie_file_path);
private:
    static uint64_t folderPack(std::string folder_path, FILE* ofp, uint64_t addr_parent);
    static bool wirteFileContent(FILE* ofp, std::string fpath);
};

#endif