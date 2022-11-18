#include <stdio.h>
#include <QDir>
#include <QString>
#include "libtiedefs.h"
#include "libtiex.h"

// fseek ftell only suport 2GB size file.
// use _fseeki64/_ftelli64 for lager file in windows.
// or #define _FILE_OFFSET_BITS 64 for linux.(has no effect in windows)
#define fseek _fseeki64
#define ftell _ftelli64

LibTieX::LibTieX()
{

}

bool LibTieX::folderUnpack(std::string tie_file_path, std::string folder_path)
{
    const char *magic = "tief";
    char buf[4];
    FILE* ifp = fopen(tie_file_path.c_str(), "rb");
    if(ifp == NULL)
    return false;
    // magic check
    fread(buf, 1, 4, ifp);
    for(uint8_t i=0; i<4 ;i++) {
        if(buf[i] != magic[i]) {
            fclose(ifp);
            return false;
        }
    }
    // start unpack
    bool ret = folderUnpack(ifp, folder_path, 4);
    fclose(ifp);
    return ret;
}

bool LibTieX::folderUnpack(FILE* ifp, std::string root_path, uint64_t start_addr)
{
    char meta_buf[sizeof(file_meta_t)];
    char name_buf[256];
    uint8_t rsize;
    file_meta_t *file_meta_p = nullptr;

    // root dir check
    QDir root_dir(QString(root_path.c_str()));
    if(!root_dir.exists())
        return false;
    
    do {
        // read file meta
        fseek(ifp, file_meta_p == nullptr ? start_addr : file_meta_p->addr_next, SEEK_SET);
        rsize = fread(meta_buf, 1, sizeof(file_meta_t), ifp);
        if(rsize != sizeof(file_meta_t))
            return false;
        file_meta_p = (file_meta_t *)meta_buf;
        // read file name
        rsize = fread(name_buf, 1, file_meta_p->name_len, ifp);
        if(rsize != file_meta_p->name_len)
            return false;
        name_buf[rsize] = '\0';
		
        QString file_path = QString::fromLatin1(name_buf, file_meta_p->name_len);
        QString meta_path = QDir::cleanPath(QString(root_path.c_str()) + QDir::separator() + file_path);
        if(file_meta_p->file_type == F_TYPE_DIR) {
            // create folder
            root_dir.mkdir(file_path.toLocal8Bit().data());
            // recover child
            if(file_meta_p->addr_child == 0)
                continue;
            if(!folderUnpack(ifp, meta_path.toStdString(), file_meta_p->addr_child))
                return false;
        } else if(file_meta_p->file_type == F_TYPE_FILE) {
            // create file
            fileRecovery(ifp, meta_path.toLocal8Bit().data(), file_meta_p->addr_child);
        } else {
            return false;
        }
    } while(file_meta_p->addr_next!=0); 
    return true;
}

bool LibTieX::fileRecovery(FILE* ifp, std::string file_path, uint64_t start_addr)
{
    char buf[4096];
    int16_t rsize;
    uint64_t restore_addr = ftell(ifp);
    uint64_t file_size = 0;
    bool ret = true;
    // praper output file
    FILE* ofp = fopen(file_path.c_str(), "wb+");
    if (ofp == NULL)
        return false;
    // get file size
    fseek(ifp, start_addr, SEEK_SET);
    rsize = fread((char *)&file_size, 1, sizeof(uint64_t), ifp);
    if(rsize != sizeof(uint64_t)) {
        ret = false;
        goto read_err;
    }
    // copy content to output file
    while(file_size != 0) {
        if(file_size > 4096)
            rsize = 4096;
        else
            rsize = file_size;
        // read to buf
        rsize = fread(buf, 1, rsize, ifp);
        if(rsize < 1)
            break;
        // write from buf
        fwrite(buf, 1, rsize, ofp);
        file_size -= rsize;
    }
    // make sure
    if(file_size != 0)
        ret = false;
read_err:
    fclose(ofp);
    fseek(ifp, restore_addr, SEEK_SET); // recovery file pointer
    return ret;
}

bool LibTieX::fileLocate(std::string tie_file_path, std::string file_path, uint64_t &start_addr, uint64_t &size)
{
    const char* magic = "tief";
    char meta_buf[sizeof(file_meta_t)];
    char name_buf[256];
    uint8_t rsize;
    uint64_t seek = 4;
    file_meta_t* file_meta_p = nullptr;
    QStringList path_list = QString(file_path.c_str()).split(QLatin1Char('/'), QString::SkipEmptyParts); // use QT::SkipEmptyParts instead if >QT5.14
    int path_idx = 0;
    bool ret = true;

    FILE* fp = fopen(tie_file_path.c_str(), "rb");
    if (fp == NULL)
        return false;
    // magic check
    rsize = fread(name_buf, 1, 4, fp);
    if (rsize != 4) {
        ret = false;
        goto locate_error;
    }
    for (uint8_t i = 0; i < 4; i++) {
        if (name_buf[i] != magic[i]) {
            ret = false;
            goto locate_error;
        }
    }
    // start search and match
    while (true) {
        fseek(fp, seek, SEEK_SET);
        // read meta
        rsize = fread(meta_buf, 1, sizeof(file_meta_t), fp);
        if (rsize != sizeof(file_meta_t)) {
            ret = false;
            goto locate_error;
        }
        file_meta_p = (file_meta_t*)meta_buf;
        // check
        if (((path_idx == (path_list.size() - 1)) && (file_meta_p->file_type != F_TYPE_FILE)) ||  // should be file type
            ((path_idx < (path_list.size() - 1)) && (file_meta_p->file_type != F_TYPE_DIR)) || // should be folder type
            (path_list[path_idx].size() != file_meta_p->name_len)) {// name len shold be same
            goto next_brother;
        }
        // name match
        rsize = fread(name_buf, 1, file_meta_p->name_len, fp);
        if (rsize != file_meta_p->name_len) {
            ret = false;
            goto locate_error;
        }
        name_buf[rsize] = '\0';
        if (path_list[path_idx] != QString::fromLatin1(name_buf, file_meta_p->name_len))
            goto next_brother;
        // matched !
        if (path_idx < (path_list.size() - 1))
            path_idx++;
        else
            break; // all done!
        // seach child folder
        seek = file_meta_p->addr_child;
        continue;
next_brother:
        // search next brother
        if (file_meta_p->addr_next == 0) {
            ret = false;
            goto locate_error; // no more brother to search
        }
        seek = file_meta_p->addr_next;
        continue;
    }
    // setup result
    fread(&size, 1, sizeof(uint64_t), fp);
    start_addr = ftell(fp);
locate_error:
    fclose(fp);
    return ret;
}
