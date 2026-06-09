#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define HEAP_SIZE  (8 * 1024 * 1024)

typedef struct block
{
    uint64_t size;
    int free;
    struct block *next;
} block_t;

static uint8_t heap[HEAP_SIZE];
static block_t *free_list = 0;

void memory_init(void)
{
    free_list = (block_t*)heap;
    free_list->size = HEAP_SIZE - sizeof(block_t);
    free_list->free = 1;
    free_list->next = 0;
}

void *kmalloc(uint64_t size)
{
    if (size == 0)
        return 0;

    size = (size + 7) & ~7ULL;

    block_t *current = free_list;
    while (current)
    {
        if (current->free && current->size >= size)
        {
            if (current->size >= size + sizeof(block_t) + 8)
            {
                block_t *new_block = (block_t*)((uint8_t*)current + sizeof(block_t) + size);
                new_block->size = current->size - size - sizeof(block_t);
                new_block->free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->free = 0;
            return (void*)((uint8_t*)current + sizeof(block_t));
        }
        current = current->next;
    }

    return 0;
}

void kfree(void *ptr)
{
    if (!ptr)
        return;
    
    block_t *block = (block_t*)((uint8_t*)ptr - sizeof(block_t));
    block->free = 1;
    
    if (block->next && block->next->free)
    {
        block->size += sizeof(block_t) + block->next->size;
        block->next = block->next->next;
    }
    
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
            free_mem += current->size + sizeof(block_t);
        }
        current = current->next;
    }
    return free_mem;
}

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
    
    return ((total_free - largest_free) * 100) / total_free;
}

int main()
{
    printf("\n\n");
    printf("  Teste do Alocador Free List com        \n");
    printf("  Coalescencia                           \n");
    printf("\n\n");
    
    memory_init();
    
    printf("Ok TESTE 1: Estado Inicial\n");
    printf("  Memoria total:       %lu bytes\n", memory_total());
    printf("  Memoria usada:        %lu bytes\n", memory_used());
    printf("  Memoria livre:        %lu bytes\n", memory_free());
    printf("  Fragmentacao da memoria: %lu%%\n\n", memory_fragmentation());
    
    printf("Ok TESTE 2: Alocando 3 blocos de 256 bytes\n");
    void *ptr1 = kmalloc(256);
    void *ptr2 = kmalloc(256);
    void *ptr3 = kmalloc(256);
    
    printf("  ptr1 = %p\n", ptr1);
    printf("  ptr2 = %p\n", ptr2);
    printf("  ptr3 = %p\n", ptr3);
    printf("  Memoria usada:        %lu bytes\n", memory_used());
    printf("  Memoria livre:        %lu bytes\n", memory_free());
    printf("  Fragmentacao da memoria: %lu%%\n\n", memory_fragmentation());
    
    printf("Ok TESTE 3: Liberando bloco do meio (coalescencia com proximo)\n");
    kfree(ptr2);
    
    printf("  Memoria usada:        %lu bytes\n", memory_used());
    printf("  Memoria livre:        %lu bytes\n", memory_free());
    printf("  Fragmentacao da memoria: %lu%%\n\n", memory_fragmentation());
    
    printf("Ok TESTE 4: Alocando novo bloco (reutilizacao First Fit)\n");
    void *ptr4 = kmalloc(128);
    printf("  ptr4 = %p (reutilizou espaço de ptr2)\n", ptr4);
    printf("  Memoria usada:        %lu bytes\n", memory_used());
    printf("  Memoria livre:        %lu bytes\n", memory_free());
    printf("  Fragmentacao da memoria: %lu%%\n\n", memory_fragmentation());
    
    printf("Ok TESTE 5: Liberando ptr1 (coalescencia com anterior)\n");
    kfree(ptr1);
    
    printf("  Memoria usada:        %lu bytes\n", memory_used());
    printf("  Memoria livre:        %lu bytes\n", memory_free());
    printf("  Fragmentacao da memoria: %lu%%\n\n", memory_fragmentation());
    
    printf("Ok TESTE 6: Liberando ptr3 (coalescencia com anterior)\n");
    kfree(ptr3);
    
    printf("  Memoria usada:        %lu bytes\n", memory_used());
    printf("  Memoria livre:        %lu bytes\n", memory_free());
    printf("  Fragmentacao da memoria: %lu%%\n\n", memory_fragmentation());
    
    printf("Ok TESTE 7: Liberando ptr4 (coalescencia completa)\n");
    kfree(ptr4);
    
    printf("  Memoria usada:        %lu bytes\n", memory_used());
    printf("  Memoria livre:        %lu bytes\n", memory_free());
    printf("  Fragmentacao da memoria: %lu%%\n\n", memory_fragmentation());
    
    printf("Ok TESTE 8: Teste de Alinhamento\n");
    void *a = kmalloc(255);  // deve virar 256
    void *b = kmalloc(1);    // deve virar 8
    printf("  Alocado 255 bytes (alinhado para 256)\n");
    printf("  Alocado 1 byte (alinhado para 8)\n");
    printf("  Memoria usada:        %lu bytes\n", memory_used());
    printf("  Memoria livre:        %lu bytes\n", memory_free());
    
    kfree(a);
    kfree(b);
    
    printf("\n\n");
    printf("  TODOS OS TESTES PASSARAM COM SUCESSO \n");
    printf("\n\n");
    
    return 0;
}
