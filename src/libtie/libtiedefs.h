#ifndef __LIBTIEDEFS_H__
#define __LIBTIEDEFS_H__

#define F_TYPE_DIR  0
#define F_TYPE_FILE 1

typedef struct __packed {
    uint64_t addr_parent;
    uint64_t addr_pre;
    uint64_t addr_next;
    uint64_t addr_child;

    uint8_t file_type;
    uint8_t name_len;
} file_meta_t;

#endif