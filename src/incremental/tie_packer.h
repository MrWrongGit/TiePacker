#ifndef __TIE_PACKER_H__
#define __TIE_PACKER_H__

#include <stdio.h>
#include <iostream>

#define F_TYPE_DIR  0
#define F_TYPE_FILE 1

typedef struct __attribute__((packed)) {
    uint64_t addr_parent;
    uint64_t addr_pre;
    uint64_t addr_next;
    uint64_t addr_child;

    uint8_t file_type;
    uint8_t name_len;
} file_meta_t;

class TiePacker {
public:
    TiePacker(std::string tie_path);
    ~TiePacker();
    bool write(std::string file_abs_path, const char *file_content, uint64_t file_size);
    void flush();
private:
    FILE *_fp;
    bool _empty;
};

#endif