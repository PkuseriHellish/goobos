#pragma once

#include <stdint.h>

#define MAX_NODES 128
#define MAX_NAME  64
#define FS_RAM_SIZE (64 * 1024)

struct node {
    char name[MAX_NAME];
    uint8_t *data;
    uint32_t size;
    uint32_t capacity;   // allocated RAM
};

extern unsigned char build_archive_sfs[];
extern unsigned int build_archive_sfs_len;

void archive_init(void);

int fs_count(void);
const struct node *fs_get(int i);
struct node *fs_find(const char *name);

int fs_create(const char *name, uint32_t size);

uint32_t fs_write(struct node *n,
                  const uint8_t *buf,
                  uint32_t len);
