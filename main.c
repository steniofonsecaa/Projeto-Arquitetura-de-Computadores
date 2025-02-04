#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int R[4];             // Registradores R0, R1, R2, R3
int memoria[256];     // Memória com 256 posições
int modo_visualizacao = 0;
#define TAM_BINARIO 32

// Registradores especiais HI e LO para operações MUL e DIV
int HI = 0, LO = 0;

// Converte um número inteiro para uma string binária com agrupamento a cada 8 bits
void int_para_binario(int numero, char *binario) {
    int j = 0;
    for (int i = 0; i < TAM_BINARIO; i++) {
        int mascara = 1 << (TAM_BINARIO - 1 - i);
        int bit = (numero & mascara) != 0;
        binario[j++] = bit ? '1' : '0';
        if ((i + 1) % 8 == 0 && i != TAM_BINARIO - 1) {
            binario[j++] = ' ';
        }
    }
    binario[j] = '\0';
}

// Exibe os valores dos registradores (incluindo HI e LO)
void estado_registradores() {
    printf("Estado atual dos registradores:\n");
    for (int i = 0; i < 4; i++) {
        if (modo_visualizacao == 0) {
            printf("R%d: %d\n", i, R[i]);
        } else {
            char binario[36];  // 32 bits + 3 espaços + '\0'
            int_para_binario(R[i], binario);
            printf("R%d: %s\n", i, binario);
        }
    }
    // Exibe os registradores especiais
    if (modo_visualizacao == 0) {
        printf("HI: %d\n", HI);
        printf("LO: %d\n", LO);
    } else {
        char binHI[36], binLO[36];
        int_para_binario(HI, binHI);
        int_para_binario(LO, binLO);
        printf("HI: %s\n", binHI);
        printf("LO: %s\n", binLO);
    }
    printf("\n");
}

// Exibe os valores de uma faixa de memória
void estado_memoria(int inicio, int fim) {
    printf("Estado atual da memoria (de %d a %d):\n", inicio, fim);
    for (int i = inicio; i <= fim && i < 256; i++) {
        if (modo_visualizacao == 0) {
            printf("Memoria[%d]: %d\n", i, memoria[i]);
        } else {
            char binario[36];
            int_para_binario(memoria[i], binario);
            printf("Memoria[%d]: %s\n", i, binario);
        }
    }
    printf("\n");
}

// Processa uma instrução
void executa_instrucao(char *instrucao) {
    char operacao[10];
    int reg_dest, reg_fonte1, reg_fonte2, endereco;

    /*
     * Aqui, implementamos duas formas:
     * 1. Formato de 3 registradores (pseudo-instrução) para ADD, SUB, MUL e DIV.
     *    No caso de MUL e DIV, o resultado é armazenado em HI/LO e, adicionalmente,
     *    o valor de LO é movido para o registrador de destino.
     * 2. Formato de 2 registradores (seguindo estritamente o MIPS) para MUL e DIV,
     *    onde o resultado é apenas colocado em HI e LO.
     */

    // Tenta decodificar instruções com 3 operandos (ex: "MUL R0, R1, R2")
    if (sscanf(instrucao, "%s R%d, R%d, R%d", operacao, &reg_dest, &reg_fonte1, &reg_fonte2) == 4) {
        if (strcmp(operacao, "ADD") == 0) {
            if (reg_dest >= 0 && reg_dest < 4 && reg_fonte1 >= 0 && reg_fonte1 < 4 && reg_fonte2 >= 0 && reg_fonte2 < 4) {
                R[reg_dest] = R[reg_fonte1] + R[reg_fonte2];
                printf("Resultado armazenado em R%d: %d\n", reg_dest, R[reg_dest]);
            } else {
                printf("Erro: Registrador invalido!\n");
            }
        } else if (strcmp(operacao, "SUB") == 0) {
            if (reg_dest >= 0 && reg_dest < 4 && reg_fonte1 >= 0 && reg_fonte1 < 4 && reg_fonte2 >= 0 && reg_fonte2 < 4) {
                R[reg_dest] = R[reg_fonte1] - R[reg_fonte2];
                printf("Resultado armazenado em R%d: %d\n", reg_dest, R[reg_dest]);
            } else {
                printf("Erro: Registrador invalido!\n");
            }
        } else if (strcmp(operacao, "MUL") == 0) {
            // Pseudo-instrução MUL: calcula o produto completo
            if (reg_dest >= 0 && reg_dest < 4 && reg_fonte1 >= 0 && reg_fonte1 < 4 && reg_fonte2 >= 0 && reg_fonte2 < 4) {
                long long produto = (long long) R[reg_fonte1] * (long long) R[reg_fonte2];
                LO = (int)(produto & 0xFFFFFFFF);
                HI = (int)(((unsigned long long) produto) >> 32);
                R[reg_dest] = LO;  // Armazena o resultado (parte menos significativa) em Rdest
                printf("MUL: HI = %d, LO = %d, valor movido para R%d: %d\n", HI, LO, reg_dest, R[reg_dest]);
            } else {
                printf("Erro: Registrador invalido para MUL!\n");
            }
        } else if (strcmp(operacao, "DIV") == 0) {
            // Pseudo-instrução DIV: realiza a divisão
            if (reg_dest >= 0 && reg_dest < 4 && reg_fonte1 >= 0 && reg_fonte1 < 4 && reg_fonte2 >= 0 && reg_fonte2 < 4) {
                if (R[reg_fonte2] != 0) {
                    LO = R[reg_fonte1] / R[reg_fonte2];   // Quociente
                    HI = R[reg_fonte1] % R[reg_fonte2];   // Resto
                    R[reg_dest] = LO;                     // Armazena o quociente em Rdest
                    printf("DIV: HI (resto) = %d, LO (quociente) = %d, valor movido para R%d: %d\n", HI, LO, reg_dest, R[reg_dest]);
                } else {
                    printf("Erro: Divisao por zero!\n");
                }
            } else {
                printf("Erro: Registrador invalido para DIV!\n");
            }
        } else {
            printf("Instrucao nao reconhecida: %s\n", operacao);
        }
    }
    // Tenta decodificar instruções com 2 operandos (formato MIPS para MUL e DIV, ex: "MUL R1, R2")
    else if (sscanf(instrucao, "%s R%d, R%d", operacao, &reg_fonte1, &reg_fonte2) == 3) {
        if (strcmp(operacao, "MUL") == 0) {
            long long produto = (long long) R[reg_fonte1] * (long long) R[reg_fonte2];
            LO = (int)(produto & 0xFFFFFFFF);
            HI = (int)(((unsigned long long) produto) >> 32);
            printf("MUL: HI = %d, LO = %d\n", HI, LO);
        } else if (strcmp(operacao, "DIV") == 0) {
            if (R[reg_fonte2] != 0) {
                LO = R[reg_fonte1] / R[reg_fonte2];
                HI = R[reg_fonte1] % R[reg_fonte2];
                printf("DIV: HI (resto) = %d, LO (quociente) = %d\n", HI, LO);
            } else {
                printf("Erro: Divisao por zero!\n");
            }
        }
        else if (strcmp(operacao, "LOAD") == 0) {
            // Se for LOAD, tenta ler no formato "LOAD Rdest, Addr"
            if (sscanf(instrucao, "%s R%d, %d", operacao, &reg_dest, &endereco) == 3) {
                if (reg_dest >= 0 && reg_dest < 4 && endereco >= 0 && endereco < 256) {
                    R[reg_dest] = memoria[endereco];
                    printf("Carregado da memoria[%d] para R%d: %d\n", endereco, reg_dest, R[reg_dest]);
                } else {
                    printf("Erro: Registrador ou endereco de memoria invalido!\n");
                }
            } else {
                printf("Erro ao decodificar a instrucao: %s\n", instrucao);
            }
        }
        else {
            printf("Erro ao decodificar a instrucao: %s\n", instrucao);
        }
    }
    // Tenta decodificar LOAD no formato "LOAD Rdest, Addr" (caso não tenha sido capturado acima)
    else if (sscanf(instrucao, "%s R%d, %d", operacao, &reg_dest, &endereco) == 3 && strcmp(operacao, "LOAD") == 0) {
        if (reg_dest >= 0 && reg_dest < 4 && endereco >= 0 && endereco < 256) {
            R[reg_dest] = memoria[endereco];
            printf("Carregado da memoria[%d] para R%d: %d\n", endereco, reg_dest, R[reg_dest]);
        } else {
            printf("Erro: Registrador ou endereco de memoria invalido!\n");
        }
    }
    else {
        printf("Erro ao decodificar a instrucao: %s\n", instrucao);
    }

    // Mostra os valores dos registradores após a execução da instrução
    estado_registradores();
}

void escolher_modo_visualizacao() {
    char escolha[10];
    printf("Escolha o modo de visualizacao:\n");
    printf("1 - Decimal\n");
    printf("2 - Binario\n");
    printf("> ");
    fgets(escolha, sizeof(escolha), stdin);
    escolha[strcspn(escolha, "\n")] = '\0';

    if (strcmp(escolha, "1") == 0) {
        modo_visualizacao = 0;
        printf("Modo de visualizacao alterado para DECIMAL.\n\n");
    } else if (strcmp(escolha, "2") == 0) {
        modo_visualizacao = 1;
        printf("Modo de visualizacao alterado para BINARIO.\n\n");
    } else {
        printf("Opcao invalida! Mantendo o modo atual.\n\n");
    }
}

int main() {
    char instrucao[50];

    printf("Simulador de Assembly - MIPS32\n");

    // Pergunta o modo de visualizacao ao iniciar o programa
    escolher_modo_visualizacao();

    printf("Digite instrucoes no formato:\n");
    printf("  OP Rdest, Rsrc1, Rsrc2 (ex: ADD R0, R1, R2 ou MUL R0, R1, R2 / DIV R0, R1, R2)\n");
    printf("  Para MUL e DIV, tambem e aceito o formato MIPS: MUL Rsrc1, Rsrc2 ou DIV Rsrc1, Rsrc2\n");
    printf("  LOAD Rdest, Addr (ex: LOAD R1, 10)\n");
    printf("Digite 'MODO' para alterar o modo de visualizacao.\n");
    printf("Digite 'SAIR' para sair.\n\n");

    // Inicializa os registradores
    R[0] = 0;
    R[1] = 10;
    R[2] = 20;
    R[3] = 30;
    // Inicializa a memória
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
        } else if (strcmp(instrucao, "MODO") == 0) {
            escolher_modo_visualizacao();
        } else {
            executa_instrucao(instrucao);
        }
    }
    printf("Encerrando o simulador...\n");
    return 0;
}
