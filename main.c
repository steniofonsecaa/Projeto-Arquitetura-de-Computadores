#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int R[4]; // Registradores R0, R1, R2, R3
int memoria[256]; // Memória com 256 posições

// Exibe os valores dos registradores
void estado_registradores() {
    printf("Estado atual dos registradores:\n");
    for (int i = 0; i < 4; i++) {
        printf("R%d: %d\n", i, R[i]);
    }
    printf("\n");
}

// Exibe os valores de uma faixa de memória
void estado_memoria(int inicio, int fim) {
    printf("Estado atual da memoria (de %d a %d):\n", inicio, fim);
    for (int i = inicio; i <= fim && i < 256; i++) {
        printf("Memoria[%d]: %d\n", i, memoria[i]);
    }
    printf("\n");
}

// Processa uma instrução
void executa_instrucao(char *instrucao) {
    char operacao[5];
    int reg_dest, reg_fonte1, reg_fonte2, endereco;

    // Decodificação da instrução
    if (sscanf(instrucao, "%s R%d, R%d, R%d", operacao, &reg_dest, &reg_fonte1, &reg_fonte2) == 4) {
        if (strcmp(operacao, "SUB") == 0) {
            if (reg_dest >= 0 && reg_dest < 4 && reg_fonte1 >= 0 && reg_fonte1 < 4 && reg_fonte2 >= 0 && reg_fonte2 < 4) {
                R[reg_dest] = R[reg_fonte1] - R[reg_fonte2];
                printf("Resultado armazenado em R%d: %d\n", reg_dest, R[reg_dest]);
            } else {
                printf("Erro: Registrador invalido!\n");
            }
        } else if (strcmp(operacao, "ADD") == 0) {
            if (reg_dest >= 0 && reg_dest < 4 && reg_fonte1 >= 0 && reg_fonte1 < 4 && reg_fonte2 >= 0 && reg_fonte2 < 4) {
                R[reg_dest] = R[reg_fonte1] + R[reg_fonte2];
                printf("Resultado armazenado em R%d: %d\n", reg_dest, R[reg_dest]);
            } else {
                printf("Erro: Registrador invalido!\n");
            }
        } else {
            printf("Instrucao nao reconhecida: %s\n", operacao);
        }
    }
    else if (sscanf(instrucao, "%s R%d, %d", operacao, &reg_dest, &endereco) == 3 && strcmp(operacao, "LOAD") == 0) {
        if (reg_dest >= 0 && reg_dest < 4 && endereco >= 0 && endereco < 256) {
            R[reg_dest] = memoria[endereco];
            printf("Carregado da memoria[%d] para R%d: %d\n", endereco, reg_dest, R[reg_dest]);
        } else {
            printf("Erro: Registrador ou endereco de memoria invalido!\n");
        }
    } else {
        printf("Erro ao decodificar a instrucao: %s\n", instrucao);
    }

    // Mostra os valores dos registradores após cada instrução
    estado_registradores();
}

int main() {
    char instrucao[50];

    printf("Simulador de Assembly - MIPS32\n");
    printf("Digite instrucoes no formato:\n");
    printf("  OP Rdest, Rsrc1, Rsrc2 (e.g., ADD R0, R1, R2)\n");
    printf("  LOAD Rdest, Addr (e.g., LOAD R1, 10)\n");
    printf("Digite 'SAIR' para sair.\n\n");

    // Inicialização dos registradores
    R[0] = 0;
    R[1] = 10;
    R[2] = 20;
    R[3] = 30;
    // Inicialização da memória
    for (int i = 0; i < 256; i++) {
        memoria[i] = i * 10;
    }
    printf("Estado inicial dos registradores:\n");
    estado_registradores();
    printf("Estado inicial da memoria (primeiras 10 posicoes):\n");
    estado_memoria(0, 9);

    while (1) {
        printf("> ");
        fgets(instrucao, sizeof(instrucao), stdin);
        instrucao[strcspn(instrucao, "\n")] = '\0';

        if (strcmp(instrucao, "SAIR") == 0) {
            break;
        }

        executa_instrucao(instrucao);
    }
    printf("Encerrando o simulador...\n");
    return 0;
}