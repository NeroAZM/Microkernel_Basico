#pragma once
#include <stdint.h>

#define TOTAL_INODES 256
#define MAX_DIRECT_BLOCKS 12

#define INODE_TYPE_FILE 1
#define INODE_TYPE_DIR  2

typedef struct {
    uint32_t size;
    uint32_t type;
    uint32_t ref_count;
    uint32_t blocks[MAX_DIRECT_BLOCKS];
} inode_t;

/* Inicializa o subsistema de inodes */
void inode_system_init(void);

/* Aloca um inode livre e retorna seu id. Retorna -1 se cheio. */
int inode_alloc(void);

/* Libera um inode */
void inode_free(uint32_t inode_id);

/* Retorna um ponteiro para a estrutura do inode em memória */
inode_t* inode_get(uint32_t inode_id);
