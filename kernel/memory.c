#include "memory.h"

/*   Configuração do heap   */

#define HEAP_START 0x80400000UL
#define HEAP_SIZE  (8 * 1024 * 1024)   // 8 MB

typedef struct block
{
    uint64_t size;
    int free;
    struct block *next;
} block_t;

static uint8_t *heap_base = (uint8_t*)HEAP_START;
static block_t *free_list = 0;

/*   Inicialização   */

void memory_init(void)
{
    free_list = (block_t*)heap_base;
    free_list->size = HEAP_SIZE - sizeof(block_t);
    free_list->free = 1;
    free_list->next = 0;
}

/*   Alocador Free List (First Fit)   */

void *kmalloc(uint64_t size)
{
    if (size == 0)
        return 0;

    /* Alinhamento para 8 bytes */
    size = (size + 7) & ~7ULL;

    block_t *current = free_list;
    while (current)
    {
        /* verifica se é um bloco livre e tem tamanho suficiente */
        if (current->free && current->size >= size)
        {
            /* se sobrar espaço suficiente para outro bloco (cabeçalho + min de dados), divide */
            if (current->size >= size + sizeof(block_t) + 8)
            {
                block_t *new_block = (block_t*)((uint8_t*)current + sizeof(block_t) + size);
                new_block->size = current->size - size - sizeof(block_t);
                new_block->free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            /* marca como ocupado */
            current->free = 0;
            
            /* retorna o ponteiro após o cabeçalho */
            return (void*)((uint8_t*)current + sizeof(block_t));
        }
        current = current->next;
    }

    return 0;   // sem memória
}

/*   Free com Coalescência   */

void kfree(void *ptr)
{
    if (!ptr)
        return;
    
    /* Recuperar cabeçalho do bloco */
    block_t *block = (block_t*)((uint8_t*)ptr - sizeof(block_t));
    block->free = 1;
    
    /* Coalescência: unir com bloco próximo se estiver livre */
    if (block->next && block->next->free)
    {
        block->size += sizeof(block_t) + block->next->size;
        block->next = block->next->next;
    }
    
    /* Coalescência: unir com bloco anterior se estiver livre */
    if (block != free_list)
    {
        block_t *current = free_list;
        while (current && current->next != block)
        {
            current = current->next;
        }
        
        if (current && current->free)
        {
            current->size += sizeof(block_t) + block->size;
            current->next = block->next;
        }
    }
}

/*   Estatísticas   */

uint64_t memory_total(void)
{
    return HEAP_SIZE;
}

uint64_t memory_used(void)
{
    uint64_t used = 0;
    block_t *current = free_list;
    while (current)
    {
        if (!current->free)
        {
            /* soma também o cabeçalho, já que é memória "gasta" */
            used += current->size + sizeof(block_t);
        }
        current = current->next;
    }
    return used;
}

uint64_t memory_free(void)
{
    uint64_t free_mem = 0;
    block_t *current = free_list;
    while (current)
    {
        if (current->free)
        {
            /* soma também o cabeçalho para bater com a matemática do HEAP_TOTAL */
            free_mem += current->size + sizeof(block_t);
        }
        current = current->next;
    }
    return free_mem;
}

/*   Fragmentação do Heap   */

uint64_t memory_fragmentation(void)
{
    uint64_t total_free = memory_free();
    uint64_t largest_free = 0;
    
    block_t *current = free_list;
    while (current)
    {
        if (current->free && current->size > largest_free)
        {
            largest_free = current->size;
        }
        current = current->next;
    }
    
    if (total_free == 0)
        return 0;
    
    /* Fragmentação = (total free - maior bloco free) / total free * 100 */
    return ((total_free - largest_free) * 100) / total_free;
}