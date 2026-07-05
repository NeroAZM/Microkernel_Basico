#include "inode.h"
#include "string.h"

static inode_t inode_table[TOTAL_INODES];
static uint8_t inode_bitmap[TOTAL_INODES / 8];

void inode_system_init(void)
{
    memset(inode_table, 0, sizeof(inode_table));
    memset(inode_bitmap, 0, sizeof(inode_bitmap));
}

int inode_alloc(void)
{
    for (int i = 0; i < TOTAL_INODES; i++)
    {
        int byte = i / 8;
        int bit = i % 8;
        if ((inode_bitmap[byte] & (1 << bit)) == 0)
        {
            inode_bitmap[byte] |= (1 << bit);
            memset(&inode_table[i], 0, sizeof(inode_t));
            return i;
        }
    }
    return -1;
}

void inode_free(uint32_t inode_id)
{
    if (inode_id < TOTAL_INODES)
    {
        int byte = inode_id / 8;
        int bit = inode_id % 8;
        inode_bitmap[byte] &= ~(1 << bit);
    }
}

inode_t* inode_get(uint32_t inode_id)
{
    if (inode_id < TOTAL_INODES)
    {
        return &inode_table[inode_id];
    }
    return 0; // null
}
