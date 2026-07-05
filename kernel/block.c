#include "block.h"
#include "memory.h"
#include "string.h"

static uint8_t *virtual_disk;
static uint8_t block_bitmap[TOTAL_BLOCKS / 8];

void block_init(void)
{
    virtual_disk = (uint8_t *)kmalloc(TOTAL_BLOCKS * BLOCK_SIZE);
    if (virtual_disk) {
        memset(virtual_disk, 0, TOTAL_BLOCKS * BLOCK_SIZE);
    }
    memset(block_bitmap, 0, sizeof(block_bitmap));
    
    // Reserva o bloco 0 e inode 0 (na logica, 0 eh usado como nulo/vazio)
    // Para o bloco, 0 significa ponteiro vazio nas structs de Inodes.
    block_bitmap[0] |= 1;
}

int block_alloc(void)
{
    for (int i = 0; i < TOTAL_BLOCKS; i++)
    {
        int byte = i / 8;
        int bit = i % 8;
        if ((block_bitmap[byte] & (1 << bit)) == 0)
        {
            block_bitmap[byte] |= (1 << bit);
            // Zera o bloco alocado
            memset(virtual_disk + (i * BLOCK_SIZE), 0, BLOCK_SIZE);
            return i;
        }
    }
    return -1; // disco cheio
}

void block_free(uint32_t block)
{
    if (block < TOTAL_BLOCKS)
    {
        int byte = block / 8;
        int bit = block % 8;
        block_bitmap[byte] &= ~(1 << bit);
    }
}

void block_read(uint32_t block, void *buffer)
{
    if (block < TOTAL_BLOCKS && virtual_disk)
    {
        memcpy(buffer, virtual_disk + (block * BLOCK_SIZE), BLOCK_SIZE);
    }
}

void block_write(uint32_t block, const void *buffer)
{
    if (block < TOTAL_BLOCKS && virtual_disk)
    {
        memcpy(virtual_disk + (block * BLOCK_SIZE), buffer, BLOCK_SIZE);
    }
}
