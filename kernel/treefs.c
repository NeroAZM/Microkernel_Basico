#include "treefs.h"
#include "block.h"
#include "inode.h"
#include "string.h"
#include "uart.h"

static superblock_t superblock;

static int split_path(const char *path, char *parent, char *name)
{
    int len = strlen(path);
    int last_slash = -1;
    for (int i = len - 1; i >= 0; i--) {
        if (path[i] == '/') {
            last_slash = i;
            break;
        }
    }

    if (last_slash == -1) return -1; // Caminho absoluto inválido

    if (last_slash == 0) {
        strcpy(parent, "/");
    } else {
        strncpy(parent, path, last_slash);
        parent[last_slash] = '\0';
    }

    strcpy(name, path + last_slash + 1);
    return 0;
}

int fs_init(void)
{
    block_init();
    inode_system_init();

    superblock.magic = TREEFS_MAGIC;
    superblock.total_blocks = TOTAL_BLOCKS;
    superblock.total_inodes = TOTAL_INODES;
    superblock.block_size = BLOCK_SIZE;

    // Criar o inode raiz (garante que seja o inode 0)
    int root_inode_id = inode_alloc();
    if (root_inode_id != 0) {
        uart_print("Erro: root inode deve ser 0\n");
        return -1;
    }
    
    inode_t *root = inode_get(root_inode_id);
    root->type = INODE_TYPE_DIR;
    root->size = 0;
    root->ref_count = 1;

    // Criar estrutura obrigatória inicial
    mkdir("/home");
    mkdir("/tmp");
    mkdir("/bin");

    return 0;
}

inode_t *path_lookup(const char *path)
{
    if (path[0] != '/') return 0; // Somente caminhos absolutos permitidos

    inode_t *current_inode = inode_get(0); // root
    if (strcmp(path, "/") == 0) return current_inode;

    char temp_path[256];
    strcpy(temp_path, path);

    char *token = temp_path + 1;
    char *next_slash;

    while (token && *token) {
        next_slash = strchr(token, '/');
        if (next_slash) {
            *next_slash = '\0';
        }

        if (current_inode->type != INODE_TYPE_DIR) return 0; // Não é diretório

        int found = 0;
        uint32_t num_dirents = BLOCK_SIZE / sizeof(dirent_t);

        for (int b = 0; b < MAX_DIRECT_BLOCKS && !found; b++) {
            uint32_t blk = current_inode->blocks[b];
            if (blk == 0) continue;

            dirent_t dirents[num_dirents];
            block_read(blk, dirents);

            for (uint32_t i = 0; i < num_dirents; i++) {
                if (dirents[i].inode_id != 0 && strcmp(dirents[i].name, token) == 0) {
                    current_inode = inode_get(dirents[i].inode_id);
                    found = 1;
                    break;
                }
            }
        }

        if (!found) return 0;

        if (next_slash) {
            token = next_slash + 1;
        } else {
            break;
        }
    }

    return current_inode;
}

static int add_dirent(inode_t *parent_inode, uint32_t child_inode_id, const char *name)
{
    uint32_t num_dirents = BLOCK_SIZE / sizeof(dirent_t);

    for (int b = 0; b < MAX_DIRECT_BLOCKS; b++) {
        if (parent_inode->blocks[b] == 0) {
            int new_block = block_alloc();
            if (new_block == -1) return -1;
            parent_inode->blocks[b] = new_block;
            
            dirent_t dirents[num_dirents];
            memset(dirents, 0, BLOCK_SIZE);
            block_write(new_block, dirents);
        }

        uint32_t blk = parent_inode->blocks[b];
        dirent_t dirents[num_dirents];
        block_read(blk, dirents);

        for (uint32_t i = 0; i < num_dirents; i++) {
            if (dirents[i].inode_id == 0) { // Slot livre
                dirents[i].inode_id = child_inode_id;
                strncpy(dirents[i].name, name, MAX_FILENAME - 1);
                dirents[i].name[MAX_FILENAME - 1] = '\0';
                block_write(blk, dirents);
                parent_inode->size += sizeof(dirent_t);
                return 0;
            }
        }
    }
    return -1; // Diretório cheio
}

int mkdir(const char *path)
{
    char parent_path[256];
    char name[256];
    if (split_path(path, parent_path, name) != 0) return -1;

    inode_t *parent_inode = path_lookup(parent_path);
    if (!parent_inode || parent_inode->type != INODE_TYPE_DIR) return -1;
    
    // Verificar duplicidade
    if (path_lookup(path) != 0) return -1; 

    int new_inode_id = inode_alloc();
    if (new_inode_id == -1) return -1;

    inode_t *new_inode = inode_get(new_inode_id);
    new_inode->type = INODE_TYPE_DIR;
    new_inode->size = 0;
    new_inode->ref_count = 1;

    if (add_dirent(parent_inode, new_inode_id, name) != 0) {
        inode_free(new_inode_id);
        return -1;
    }

    return 0;
}

int create(const char *path)
{
    char parent_path[256];
    char name[256];
    if (split_path(path, parent_path, name) != 0) return -1;

    inode_t *parent_inode = path_lookup(parent_path);
    if (!parent_inode || parent_inode->type != INODE_TYPE_DIR) return -1;
    
    if (path_lookup(path) != 0) return -1; // Arquivo já existe
    
    int new_inode_id = inode_alloc();
    if (new_inode_id == -1) return -1;

    inode_t *new_inode = inode_get(new_inode_id);
    new_inode->type = INODE_TYPE_FILE;
    new_inode->size = 0;
    new_inode->ref_count = 1;

    if (add_dirent(parent_inode, new_inode_id, name) != 0) {
        inode_free(new_inode_id);
        return -1;
    }

    // Retorna o "file descriptor", que mapeamos simplificadamente como o ID do inode
    return new_inode_id;
}

int unlink(const char *path)
{
    char parent_path[256];
    char name[256];
    if (split_path(path, parent_path, name) != 0) return -1;

    inode_t *parent_inode = path_lookup(parent_path);
    if (!parent_inode || parent_inode->type != INODE_TYPE_DIR) return -1;

    uint32_t num_dirents = BLOCK_SIZE / sizeof(dirent_t);

    for (int b = 0; b < MAX_DIRECT_BLOCKS; b++) {
        uint32_t blk = parent_inode->blocks[b];
        if (blk == 0) continue;

        dirent_t dirents[num_dirents];
        block_read(blk, dirents);

        for (uint32_t i = 0; i < num_dirents; i++) {
            if (dirents[i].inode_id != 0 && strcmp(dirents[i].name, name) == 0) {
                uint32_t target_inode_id = dirents[i].inode_id;
                
                // Remover entrada do diretório
                dirents[i].inode_id = 0; 
                block_write(blk, dirents);
                parent_inode->size -= sizeof(dirent_t);
                
                inode_t *target_inode = inode_get(target_inode_id);
                // Liberar blocos de dados
                for(int tb = 0; tb < MAX_DIRECT_BLOCKS; tb++) {
                    if (target_inode->blocks[tb] != 0) {
                        block_free(target_inode->blocks[tb]);
                        target_inode->blocks[tb] = 0;
                    }
                }
                // Liberar inode
                inode_free(target_inode_id);
                return 0;
            }
        }
    }
    return -1; // Não encontrado
}

int ls(const char *path)
{
    inode_t *dir_inode = path_lookup(path);
    if (!dir_inode || dir_inode->type != INODE_TYPE_DIR) return -1;

    uint32_t num_dirents = BLOCK_SIZE / sizeof(dirent_t);

    for (int b = 0; b < MAX_DIRECT_BLOCKS; b++) {
        uint32_t blk = dir_inode->blocks[b];
        if (blk == 0) continue;

        dirent_t dirents[num_dirents];
        block_read(blk, dirents);

        for (uint32_t i = 0; i < num_dirents; i++) {
            if (dirents[i].inode_id != 0) {
                uart_print(dirents[i].name);
                uart_print("\n");
            }
        }
    }
    return 0;
}

int read(int fd, void *buf, uint32_t size)
{
    inode_t *file_inode = inode_get(fd);
    if (!file_inode || file_inode->type != INODE_TYPE_FILE) return -1;

    uint32_t bytes_read = 0;
    uint8_t *byte_buf = (uint8_t*)buf;

    for (int b = 0; b < MAX_DIRECT_BLOCKS && bytes_read < size && bytes_read < file_inode->size; b++) {
        uint32_t blk = file_inode->blocks[b];
        if (blk == 0) break;

        uint8_t block_buf[BLOCK_SIZE];
        block_read(blk, block_buf);

        uint32_t to_read = BLOCK_SIZE;
        if (size - bytes_read < to_read) to_read = size - bytes_read;
        if (file_inode->size - bytes_read < to_read) to_read = file_inode->size - bytes_read;

        memcpy(byte_buf + bytes_read, block_buf, to_read);
        bytes_read += to_read;
    }
    return bytes_read;
}

int write(int fd, const void *buf, uint32_t size)
{
    inode_t *file_inode = inode_get(fd);
    if (!file_inode || file_inode->type != INODE_TYPE_FILE) return -1;

    uint32_t bytes_written = 0;
    const uint8_t *byte_buf = (const uint8_t*)buf;

    for (int b = 0; b < MAX_DIRECT_BLOCKS && bytes_written < size; b++) {
        if (file_inode->blocks[b] == 0) {
            int new_blk = block_alloc();
            if (new_blk == -1) break; // Disco cheio
            file_inode->blocks[b] = new_blk;
        }
        
        uint32_t blk = file_inode->blocks[b];
        uint8_t block_buf[BLOCK_SIZE];
        memset(block_buf, 0, BLOCK_SIZE); 

        uint32_t to_write = BLOCK_SIZE;
        if (size - bytes_written < to_write) to_write = size - bytes_written;

        memcpy(block_buf, byte_buf + bytes_written, to_write);
        block_write(blk, block_buf);

        bytes_written += to_write;
    }

    if (bytes_written > file_inode->size) {
        file_inode->size = bytes_written;
    }

    return bytes_written;
}
