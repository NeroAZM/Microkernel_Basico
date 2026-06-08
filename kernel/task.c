#include "task.h"
#include "memory.h"
#include <stdint.h>

TCB tasks[MAX_TASKS];
int task_count = 0;

void xTaskCreate(void (*task)(void),
                 uint32_t stack_size,
                 int priority)
{
    if (task_count >= MAX_TASKS)
        return;

    if (stack_size == 0)
        stack_size = DEFAULT_STACK_SIZE;

    TCB *t = &tasks[task_count];

    t->stack = (uint8_t*)kmalloc(stack_size);

    uint64_t *sp = (uint64_t*)(t->stack + stack_size);

    /* Configurar contexto inicial */

    t->regs[0] = (uint64_t)task;   // ra
    t->regs[1] = (uint64_t)sp;     // sp

    /* Captura o valor do gp atual e salva na task */
    register uint64_t gp asm("gp");
    t->regs[2] = gp;

    t->entry = task;
    t->priority = priority;

    task_count++;
}

void xTaskDelete(int index)
{
    if (index < 0 || index >= task_count)
        return;

    TCB *t = &tasks[index];

    if (t->stack)
    {
        kfree(t->stack);
        t->stack = 0;
    }

    for (int i = index; i < task_count - 1; i++)
        tasks[i] = tasks[i + 1];

    task_count--;
}
