
#include <stdint.h>
#include "sfs.h"

static struct node nodes[MAX_NODES];
static int node_count;

static uint8_t fs_ram[FS_RAM_SIZE];
static uint32_t fs_ram_used;


static int copy_name(char *dst, const char *src)
{
    uint32_t i = 0;

    while (src[i] && i < MAX_NAME - 1) {
        dst[i] = src[i];
        i++;
    }

    if (src[i] != '\0')
        return 0;   // name too long

    dst[i] = '\0';
    return 1;
}


static uint32_t read_u32le(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static int streq(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return *a == 0 && *b == 0;
}
void archive_init(void) {
   const uint8_t *base = build_archive_sfs;
const uint8_t *limit = build_archive_sfs + build_archive_sfs_len;

    node_count = 0;

    if (build_archive_sfs_len < 8)
        return;

    if (base[0] != 'S' ||
        base[1] != 'F' ||
        base[2] != 'S' ||
        base[3] != '1')
        return;

    uint32_t count = read_u32le(base + 4);
    const uint8_t *p = base + 8;

    for (uint32_t i = 0;
         i < count && node_count < MAX_NODES;
         i++) {

        if ((uint32_t)(limit - p) < 4)
            break;

        uint32_t name_len = read_u32le(p);
        p += 4;

        if (name_len > (uint32_t)(limit - p))
            break;

        const char *name = (const char *)p;
        p += name_len;

        if ((uint32_t)(limit - p) < 4)
            break;

        uint32_t data_len = read_u32le(p);
        p += 4;

        if (data_len > (uint32_t)(limit - p))
            break;

        uint8_t *data = (uint8_t *)p;
        p += data_len;

        
        if (!copy_name(nodes[node_count].name, name))
    break;

nodes[node_count].data = data;
nodes[node_count].size = data_len;
node_count++;

    }
}


int fs_count(void) { return node_count; }

const struct node *fs_get(int i) {
    return (i >= 0 && i < node_count) ? &nodes[i] : 0;
}

struct node *fs_find(const char *name) {
    for (int i = 0; i < node_count; i++)
        if (streq(nodes[i].name, name)) return &nodes[i];
    return 0;
}

// Overwrites a file's content in place. Returns bytes actually written,
// clamped to the entry's original size -- there's no allocator to grow it.
uint32_t fs_write(struct node *n, const uint8_t *buf, uint32_t len) {
    if (!n) return 0;
    if (len > n->size) len = n->size;
    for (uint32_t i = 0; i < len; i++) n->data[i] = buf[i];
    return len;
}

int fs_create(const char *name, uint32_t size)
{
    if (!name || !*name)
        return 0;

    if (node_count >= MAX_NODES)
        return 0;

    if (fs_find(name))
        return 0;

    if (size > FS_RAM_SIZE - fs_ram_used)
        return 0;

    if (!copy_name(nodes[node_count].name, name))
        return 0;

    nodes[node_count].data = &fs_ram[fs_ram_used];
    nodes[node_count].size = size;

    /* Optional: zero the new file */
    for (uint32_t i = 0; i < size; i++)
        nodes[node_count].data[i] = 0;

    fs_ram_used += size;
    node_count++;

    return 1;
}
