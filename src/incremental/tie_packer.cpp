#include "tie_packer.h"
#include <QString>
#include <QStringList>

TiePacker::TiePacker(std::string tie_path)
{
    _fp = fopen(tie_path.c_str(), "wb+");
    if(_fp == NULL)
        return;
    fwrite("tief", 1, 4, _fp);
    _empty = true;
}

TiePacker::~TiePacker()
{
    if(_fp == NULL)
        return;
    fclose(_fp);
}

bool TiePacker::write(std::string file_abs_path, const char *file_content, uint64_t file_size)
{
    if(_fp == NULL)
        return false;
    file_meta_t *file_meta_p = nullptr;
    file_meta_t file_meta_tmp;
    char meta_buf[sizeof(file_meta_t)];
    char name_buf[256];
    uint8_t rsize;

    // folder/folder/..../folder/file
    QStringList path_list = QString(file_abs_path.c_str()).split(QLatin1Char('/'), QString::SkipEmptyParts); // use QT::SkipEmptyParts instead if >QT5.14
    int path_idx = 0;
    if(path_list.size() < 1)
        return false;
    // init
    file_meta_tmp.file_type = (path_idx == (path_list.size() - 1)) ? F_TYPE_FILE : F_TYPE_DIR;
    file_meta_tmp.addr_parent = 0;
    file_meta_tmp.addr_pre = 0;

    // search and match
    fseek(_fp, 4, SEEK_SET);
    while(!_empty) {
        uint64_t init_seek = ftell(_fp);
        // search existence
        rsize = fread(meta_buf, 1, sizeof(file_meta_t), _fp);
        if(rsize != sizeof(file_meta_t))
            return false;
        file_meta_p = (file_meta_t *)meta_buf;
        // type check
        if(file_meta_tmp.file_type != file_meta_p->file_type)
            goto next_brother;
        // name len check
        if(path_list[path_idx].size() != file_meta_p->name_len)
            goto next_brother;
        // name string check
        rsize = fread(name_buf, 1, file_meta_p->name_len, _fp);
        if(rsize != file_meta_p->name_len)
            return false;
        name_buf[rsize] = '\0';
        if (path_list[path_idx] != QString::fromLatin1(name_buf, file_meta_p->name_len))
            goto next_brother;
        // file already exist!!
        if(file_meta_tmp.file_type == F_TYPE_FILE)
            return false; // file already exist, it will cost too much to handle, just return false
child_folder:
        // folder matched!
        path_idx ++;
        file_meta_tmp.file_type = (path_idx == (path_list.size() - 1)) ? F_TYPE_FILE : F_TYPE_DIR;
        if(file_meta_p->addr_child != 0) {
            // go deeper
            fseek(_fp, file_meta_p->addr_child, SEEK_SET);
            continue;
        } else {
            // child empty, create the rest path, may never reach here because file_abs_path ends with a file not a folder
            file_meta_tmp.addr_parent = init_seek;
            file_meta_tmp.addr_pre = 0;
            break;
        }
next_brother:
        if(file_meta_p->addr_next != 0) {
            // search next brother
            fseek(_fp, file_meta_p->addr_next, SEEK_SET);
            continue;
        } else {
            // no more brother to search, not exist, create the rest path!
            file_meta_tmp.addr_parent = file_meta_p->addr_parent;
            file_meta_tmp.addr_pre = init_seek;
            break;
        }
    }

    // create new folders and the file
    fseek(_fp, 0, SEEK_END);
    while(true) {
        uint64_t init_seek = ftell(_fp);
        file_meta_tmp.addr_next = 0;
        file_meta_tmp.name_len = path_list[path_idx].size(); 
        file_meta_tmp.addr_child = file_meta_tmp.file_type == F_TYPE_DIR ? 0 : uint64_t(init_seek + sizeof(file_meta_t) + file_meta_tmp.name_len);
        // write meta to file
        fwrite(&file_meta_tmp, 1, sizeof(file_meta_t), _fp);
        // write name to file
        fwrite(path_list[path_idx].toLatin1().data(), 1, file_meta_tmp.name_len, _fp);
        // update my brother's addr_next or my parent's addr_parent
        if(file_meta_tmp.addr_pre == 0) { // no brother, new child
            if(file_meta_tmp.addr_parent != 0) {
                fseek(_fp, file_meta_tmp.addr_parent + sizeof(uint64_t)*3, SEEK_SET);
                fwrite(&init_seek, 1, sizeof(uint64_t), _fp);
            }
        } else { // has brother
            fseek(_fp, file_meta_tmp.addr_pre + sizeof(uint64_t)*2, SEEK_SET);
            fwrite(&init_seek, 1, sizeof(uint64_t), _fp);
        }
        fseek(_fp, 0, SEEK_END);
        // this is folder type, has child to create
        if(file_meta_tmp.file_type == F_TYPE_DIR) {
            path_idx ++;
            file_meta_tmp.file_type = (path_idx == (path_list.size() - 1)) ? F_TYPE_FILE : F_TYPE_DIR;
            file_meta_tmp.addr_parent = init_seek;
            file_meta_tmp.addr_pre = 0;
        } else {
            // file type, reach the end! write file size and file content.
            fwrite(&file_size, 1, sizeof(uint64_t), _fp);
            fwrite(file_content, 1, file_size, _fp);
            break; // all done!
        }
    }
    _empty = false;
    return true;
}

void TiePacker::flush()
{
    if (_fp == NULL)
        return;
    fflush(_fp);
}