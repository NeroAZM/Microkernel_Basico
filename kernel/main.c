#include "task.h"
#include "scheduler.h"
#include "memory.h"
#include "uart.h"
#include "treefs.h"

void kernel_main(void)
{
    memory_init();

    uart_print("\n=== Inicializando TreeFS ===\n\n");
    if (fs_init() != 0) {
        uart_print("Erro ao inicializar o TreeFS\n");
        while(1);
    }
    uart_print("TreeFS inicializado com sucesso.\n\n");

    uart_print("Cenario 1: Listagem da raiz\n");
    ls("/");

    uart_print("\nCenario 2: Criacao de diretorio (/home/aluno)\n");
    if (mkdir("/home/aluno") == 0) {
        uart_print("Diretorio /home/aluno criado com sucesso.\n");
    } else {
        uart_print("Erro ao criar diretorio.\n");
    }

    uart_print("\nCenario 3: Criacao de arquivo (/home/aluno/notas.txt)\n");
    int fd = create("/home/aluno/notas.txt");
    if (fd != -1) {
        uart_print("Arquivo criado com sucesso. FD: ");
        uart_print_uint(fd);
        uart_print("\n");
    } else {
        uart_print("Erro ao criar arquivo.\n");
    }

    uart_print("\nCenario 4: Escrita\n");
    const char *text = "Sistemas Operacionais";
    int bytes_written = write(fd, text, 22);
    uart_print("Bytes escritos: ");
    uart_print_uint(bytes_written);
    uart_print("\n");

    uart_print("\nCenario 5: Leitura\n");
    char buffer[32];
    int bytes_read = read(fd, buffer, 22);
    uart_print("Bytes lidos: ");
    uart_print_uint(bytes_read);
    uart_print("\nConteudo: ");
    uart_print(buffer);
    uart_print("\n");

    uart_print("\nCenario 7: Navegacao hierarquica ls(/home)\n");
    ls("/home");
    
    uart_print("\nCenario 7 extra: Navegacao hierarquica ls(/home/aluno)\n");
    ls("/home/aluno");

    uart_print("\nCenario 6: Remocao\n");
    if (unlink("/home/aluno/notas.txt") == 0) {
        uart_print("Arquivo removido com sucesso.\n");
    } else {
        uart_print("Erro ao remover arquivo.\n");
    }
    
    uart_print("\nListagem apos remocao ls(/home/aluno)\n");
    ls("/home/aluno");

    uart_print("\nCenario 8: Reutilizacao de inodes e blocos\n");
    int fd2 = create("/home/aluno/teste2.txt");
    uart_print("Novo arquivo criado. FD: ");
    uart_print_uint(fd2);
    uart_print(" (esperado que reutilize o FD do arquivo removido)\n");

    uart_print("\nTestes M3 concluidos com sucesso!\n");

    while (1);
}
