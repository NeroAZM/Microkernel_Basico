#include "task.h"
#include "scheduler.h"
#include "memory.h"
#include "uart.h"

static void print_heap_stats(void)
{
    uart_print("Heap total: ");
    uart_print_uint(memory_total());
    uart_print(" bytes\n");

    uart_print("Heap usado: ");
    uart_print_uint(memory_used());
    uart_print(" bytes\n");

    uart_print("Heap livre: ");
    uart_print_uint(memory_free());
    uart_print(" bytes\n\n");
}

void task1(void)
{
    while (1)
    {
        uart_print("Task 1 running\n");
        print_heap_stats();
        yield();
    }
}

void task2(void)
{
    while (1)
    {
        uart_print("Task 2 running\n");
        print_heap_stats();
        yield();
    }
}

void kernel_main(void)
{
    memory_init();

    uart_print("\n=== Gerencia de Memoria ===\n\n");
    print_heap_stats();

    xTaskCreate(task1, 2048, 1);
    uart_print("Task 1 criada\n");

    xTaskCreate(task2, 2048, 1);
    uart_print("Task 2 criada\n\n");
    print_heap_stats();

    xTaskDelete(0);
    uart_print("Task 1 removida\n\n");
    print_heap_stats();

    uart_print("=== Scheduler ===\n\n");
    scheduler_start();

    while (1);
}
