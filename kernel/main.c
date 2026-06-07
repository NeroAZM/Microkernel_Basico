#include "task.h"
#include "scheduler.h"
#include "memory.h"
#include "uart.h"

/*   Contador Global de Iterações   */
static int iteration_count = 0;
#define MAX_ITERATIONS 10  // Para após 5 ciclos

/*   Tasks   */

void task1()
{
    iteration_count++;
    
    if (iteration_count >= MAX_ITERATIONS)
    {
        uart_print("\n=== Tempo limite atingido ===\n");
        uart_print("Finalizando kernel...\n");
        while(1);  // Parar aqui
    }
    
    uart_print("Task 1 running\n");

    uart_print("Memory used: ");
    uart_print_uint(memory_used());
    uart_print(" bytes\n");

    uart_print("Memory free: ");
    uart_print_uint(memory_free());
    uart_print(" bytes\n");

    uart_print("Memory fragmentation: ");
    uart_print_uint(memory_fragmentation());
    uart_print("%\n\n");

    yield();
}

void task2()
{
    uart_print("Task 2 running\n");

    uart_print("Memory used: ");
    uart_print_uint(memory_used());
    uart_print(" bytes\n");

    uart_print("Memory free: ");
    uart_print_uint(memory_free());
    uart_print(" bytes\n");

    uart_print("Memory fragmentation: ");
    uart_print_uint(memory_fragmentation());
    uart_print("%\n\n");

    yield();
}

/*   Kernel   */

void kernel_main()
{
    memory_init();   // OBRIGATÓRIO

    uart_print("\n=== Kernel ===\n");
    uart_print("Executando por ");
    uart_print_uint(MAX_ITERATIONS);
    uart_print(" iteracoes...\n\n");

    xTaskCreate(task1, 2048, 1);
    xTaskCreate(task2, 2048, 1);

    scheduler_start();

    /* Nunca chega aqui (parado no task1) */
    while (1);
}