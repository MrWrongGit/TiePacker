#include <fstream>
#include <QDir>
#include <QString>
#include <QFileInfoList>
#include "libtiedefs.h"
#include "libtiec.h"

// fseek ftell only suport 2GB size file.
// use _fseeki64/_ftelli64 for lager file in windows.
// or #define _FILE_OFFSET_BITS 64 for linux.(has no effect in windows)
#define fseek _fseeki64
#define ftell _ftelli64

LibTieC::LibTieC()
{

}

bool LibTieC::folderPack(std::string folder_path, std::string tie_file_path)
{
    FILE* ofp = fopen(tie_file_path.c_str(), "wb+");
    if (ofp == NULL)
        return false;
    // write magic
    fwrite("tief", 1, 4, ofp);
    uint64_t ret = folderPack(folder_path, ofp, 0);
    fclose(ofp);
    return ret >= 4;
}

/*
ret value:
    errors: 0 < and < 4
*/
uint64_t LibTieC::folderPack(std::string folder_path, FILE* ofp, uint64_t addr_parent)
{
    uint64_t addr_init =  ftell(ofp);
    uint64_t addr_pre = 0;

    QDir dir(QString(folder_path.c_str()));
    QFileInfoList file_info_list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs);
    // nothing under current folder
    if(file_info_list.size() < 1)
        return 0;
    
    foreach (auto file_info, file_info_list) {
        // init file_meta
        file_meta_t file_meta;
        file_meta.addr_parent  = addr_parent;
        file_meta.addr_pre = addr_pre;
        file_meta.addr_next = 0;
        file_meta.name_len = file_info.fileName().size() > 255 ? 255 : file_info.fileName().toLatin1().size();
        file_meta.addr_child = file_info.isDir() ? 0 : uint64_t(ftell(ofp)) + uint64_t(sizeof(file_meta_t)) + uint64_t(file_meta.name_len); // F_TYPE_FILE
        file_meta.file_type = file_info.isDir() ? F_TYPE_DIR : F_TYPE_FILE;
        // update for younger brother, now addr_pre is my address
        addr_pre = ftell(ofp);
        // wirite meta to file
        fwrite((char *)&file_meta, 1, sizeof(file_meta_t), ofp);
        // wiite name after meta
        fwrite(file_info.fileName().toLatin1().data(), 1, file_meta.name_len, ofp);
        // update elder brother's addr_next
        if(file_meta.addr_pre != 0) {
            uint64_t restore_addr = ftell(ofp);
            // jump to elder brother's addr_next and update
            fseek(ofp, file_meta.addr_pre + sizeof(uint64_t)*2, SEEK_SET);
            fwrite((char *)&addr_pre, 1, sizeof(uint64_t), ofp);
            // restore file pointer
            fseek(ofp, restore_addr, SEEK_SET);
        }

        if(file_info.isDir()) {
            // recursion
            uint64_t address = folderPack(file_info.absoluteFilePath().toStdString(), ofp, addr_pre);
            // error happend
            if((address > 0) && (address < 4))
                return address;
            // update my addr_child
            if(address >= 4) {
                uint64_t restore_addr = ftell(ofp);
                // jump to my addr_child and update
                fseek(ofp, addr_pre + sizeof(uint64_t)*3, SEEK_SET);
                fwrite((char *)&address, 1, sizeof(uint64_t), ofp);
                // restore file pointer
                fseek(ofp, restore_addr, SEEK_SET);
            }
        } else {
            // write file_size and file_content close to file_meta
            if(!wirteFileContent(ofp, file_info.absoluteFilePath().toStdString()))
                return 1;
        }
    }
    // my parent will use this address as addr_child
    return addr_init;
}

bool LibTieC::wirteFileContent(FILE* ofp, std::string fpath)
{
    char buf[4096];
    uint64_t total_size = 0;
    uint64_t start_offset = ftell(ofp);
    
    FILE* ifp = fopen(fpath.c_str(), "rb");
    if (ofp == NULL)
        return false;
    // place for size, fill it later
    fseek(ofp, start_offset + sizeof(uint64_t), SEEK_SET);
    // copy file
    while(true) {
        int16_t size = fread(buf, 1, 4096, ifp);
        if(size < 1)
            break;
        fwrite(buf, 1, size, ofp);
        total_size += size;
    }
    fclose(ifp);
    // write size
    fseek(ofp, start_offset, SEEK_SET);
    fwrite((char*)&total_size, 1, sizeof(uint64_t), ofp);
    // back to end
    fseek(ofp, 0, SEEK_END);
    return true;
}
