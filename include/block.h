#pragma once
#include <stdint.h>

#define BLOCK_SIZE 512
#define TOTAL_BLOCKS 1024

/* Inicializa o subsistema de blocos (ex: zera o bitmap) */
void block_init(void);

/* Aloca um bloco livre e retorna seu índice. Retorna -1 se cheio. */
int block_alloc(void);

/* Libera um bloco ocupado */
void block_free(uint32_t block);

/* Lê o conteúdo de um bloco para o buffer */
void block_read(uint32_t block, void *buffer);

/* Grava o conteúdo do buffer em um bloco */
void block_write(uint32_t block, const void *buffer);
