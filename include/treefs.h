#pragma once
#include <stdint.h>
#include "inode.h"

#define MAX_FILENAME 28
#define TREEFS_MAGIC 0x54524545 // "TREE"

typedef struct {
    uint32_t magic;
    uint32_t total_blocks;
    uint32_t total_inodes;
    uint32_t block_size;
} superblock_t;

typedef struct {
    uint32_t inode_id;
    char name[MAX_FILENAME];
} dirent_t;

/* Inicialização do sistema de arquivos */
int fs_init(void);

/* Resolução de caminhos absolutos */
inode_t *path_lookup(const char *path);

/* Operações de diretório e arquivo */
int mkdir(const char *path);
int create(const char *path);
int unlink(const char *path);
int ls(const char *path);

/* A interface do professor sugere o uso de um "int fd" (file descriptor). 
   Para simplificar neste trabalho, mapearemos esse 'fd' diretamente para o inode_id do arquivo aberto. */
int read(int fd, void *buf, uint32_t size);
int write(int fd, const void *buf, uint32_t size);
