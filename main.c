#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAM_BINARIO 32
#define TAM_MEM_DADOS 256
#define TAM_MEM_INSTRUCOES 256
#define TAM_INSTRUCAO 50

// Registradores
int R[4];
// Memória de dados
int memoria[TAM_MEM_DADOS];
// Memória de instruções
char instrucao_mem[TAM_MEM_INSTRUCOES][TAM_INSTRUCAO];
int num_instrucoes = 0;
int PC = 0;           // Usado em instruções do tipo J

int modo_visualizacao = 0;

// Registradores especiais HI e LO (para MUL e DIV)
int HI = 0, LO = 0;

// Converte um número inteiro para uma string de 32 bit
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

// Imprime o estado dos registradores (incluindo HI e LO)
void estado_registradores() {
    printf("Estado dos registradores:\n");
    for (int i = 0; i < 4; i++) {
        if (modo_visualizacao == 0)
            printf("R%d: %d\n", i, R[i]);
        else {
            char bin[36];
            int_para_binario(R[i], bin);
            printf("R%d: %s\n", i, bin);
        }
    }
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

// Imprime os valores das 10 primeiras posições da memória de dados
void estado_memoria() {
    printf("Estado da memoria de dados (primeiras 10 posicoes):\n");
    for (int i = 0; i < 10; i++) {
        if (modo_visualizacao == 0)
            printf("Memoria[%d]: %d\n", i, memoria[i]);
        else {
            char bin[36];
            int_para_binario(memoria[i], bin);
            printf("Memoria[%d]: %s\n", i, bin);
        }
    }
    printf("\n");
}

// Imprime todas as instruções armazenadas na memória de instruções 
void estado_instrucao_memoria() {
    printf("Estado da memoria de instrucoes:\n");
    for (int i = 0; i < num_instrucoes; i++) {
        printf("[%d]: %s\n", i, instrucao_mem[i]);
    }
    printf("\n");
}

// Imprime o estado completo: registradores, memória de dados e memória de instruções 
void imprime_estado_completo() {
    estado_memoria();
    estado_instrucao_memoria();
    estado_registradores();
}

// Permite ao usuário escolher o modo de visualização (decimal ou binário) 
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
        printf("Modo DECIMAL selecionado.\n\n");
        imprime_estado_completo();
    } else if (strcmp(escolha, "2") == 0) {
        modo_visualizacao = 1;
        printf("Modo BINARIO selecionado.\n\n");
        imprime_estado_completo();
    } else {
        printf("Opcao invalida! Mantendo modo atual.\n\n");
    }
}

int executa_instrucao_programa(char *instrucao) {
    char operacao[10];
    int reg_dest, reg_fonte1, reg_fonte2, endereco;

    // Decodifica uma instrução do tipo J, por exemplo: "J 3"
    if (sscanf(instrucao, "J %d", &endereco) == 1) {
        if (endereco >= 0 && endereco < num_instrucoes) {
            printf("Jump: Acessando a memoria de instrucoes no endereco %d...\n", endereco);
            PC = endereco;
            char instrucao_fetched[TAM_INSTRUCAO];
            strcpy(instrucao_fetched, instrucao_mem[PC]);
            printf("Instrucao encontrada: %s\n", instrucao_fetched);
            // Executa a instrução obtida da memória de instruções
            int dummy = executa_instrucao_programa(instrucao_fetched);
            return 1;
        } else {
            printf("Erro: Endereco invalido para jump!\n");
            imprime_estado_completo();
            return 0;
        }
    }

    // Decodifica instruções com 3 operandos, por exemplo: "ADD R0, R1, R2" 
    if (sscanf(instrucao, "%s R%d, R%d, R%d", operacao, &reg_dest, &reg_fonte1, &reg_fonte2) == 4) {
        if (strcmp(operacao, "ADD") == 0) {
            R[reg_dest] = R[reg_fonte1] + R[reg_fonte2];
            printf("ADD executado: R%d = %d\n", reg_dest, R[reg_dest]);
        } else if (strcmp(operacao, "SUB") == 0) {
            R[reg_dest] = R[reg_fonte1] - R[reg_fonte2];
            printf("SUB executado: R%d = %d\n", reg_dest, R[reg_dest]);
        } else if (strcmp(operacao, "MUL") == 0) {
            long long produto = (long long)R[reg_fonte1] * (long long)R[reg_fonte2];
            LO = (int)(produto & 0xFFFFFFFF);
            HI = (int)(((unsigned long long)produto) >> 32);
            R[reg_dest] = LO;
            printf("MUL executado: HI = %d, LO = %d, R%d = %d\n", HI, LO, reg_dest, R[reg_dest]);
        } else if (strcmp(operacao, "DIV") == 0) {
            if (R[reg_fonte2] != 0) {
                LO = R[reg_fonte1] / R[reg_fonte2];
                HI = R[reg_fonte1] % R[reg_fonte2];
                R[reg_dest] = LO;
                printf("DIV executado: HI = %d, LO = %d, R%d = %d\n", HI, LO, reg_dest, R[reg_dest]);
            } else {
                printf("Erro: Divisao por zero!\n");
            }
        }
        // Implementação da instrução de comparação SLT
        else if (strcmp(operacao, "SLT") == 0) {
            if (reg_dest >= 0 && reg_dest < 4 &&
                reg_fonte1 >= 0 && reg_fonte1 < 4 &&
                reg_fonte2 >= 0 && reg_fonte2 < 4) {
                R[reg_dest] = (R[reg_fonte1] < R[reg_fonte2]) ? 1 : 0;
                printf("SLT executado: R%d = %d  (1 se R%d < R%d, senao 0)\n\n",
                       reg_dest, R[reg_dest], reg_fonte1, reg_fonte2);
            } else {
                printf("Erro: Registrador invalido para SLT!\n");
            }
        } else {
            printf("Operacao nao reconhecida: %s\n", operacao);
        }
    }
    // Instrução de LOAD
    else if (sscanf(instrucao, "%s R%d, R%d", operacao, &reg_fonte1, &reg_fonte2) == 3) {
        if (strcmp(operacao, "MUL") == 0) {
            long long produto = (long long)R[reg_fonte1] * (long long)R[reg_fonte2];
            LO = (int)(produto & 0xFFFFFFFF);
            HI = (int)(((unsigned long long)produto) >> 32);
            printf("MUL executado: HI = %d, LO = %d\n", HI, LO);
        } else if (strcmp(operacao, "DIV") == 0) {
            if (R[reg_fonte2] != 0) {
                LO = R[reg_fonte1] / R[reg_fonte2];
                HI = R[reg_fonte1] % R[reg_fonte2];
                printf("DIV executado: HI = %d, LO = %d\n", HI, LO);
            } else {
                printf("Erro: Divisao por zero!\n");
            }
        } else if (strcmp(operacao, "LOAD") == 0) {
            if (sscanf(instrucao, "%s R%d, %d", operacao, &reg_dest, &endereco) == 3) {
                if (reg_dest >= 0 && reg_dest < 4 &&
                    endereco >= 0 && endereco < TAM_MEM_DADOS) {
                    R[reg_dest] = memoria[endereco];
                    printf("LOAD executado: R%d = %d\n", reg_dest, R[reg_dest]);
                } else {
                    printf("Erro: Registrador ou endereco invalido!\n");
                }
            } else {
                printf("Erro ao decodificar LOAD.\n");
            }
        } else {
            printf("Instrucao nao reconhecida: %s\n", operacao);
        }
    }
    else if (sscanf(instrucao, "%s R%d, %d", operacao, &reg_dest, &endereco) == 3 &&
             strcmp(operacao, "LOAD") == 0) {
        if (reg_dest >= 0 && reg_dest < 4 &&
            endereco >= 0 && endereco < TAM_MEM_DADOS) {
            R[reg_dest] = memoria[endereco];
            printf("LOAD executado: R%d = %d\n", reg_dest, R[reg_dest]);
        } else {
            printf("Erro: Registrador ou endereco invalido!\n");
        }
    }
    else {
        printf("Erro ao decodificar a instrucao: %s\n", instrucao);
    }
    imprime_estado_completo();
    return 0;
}


int main() {
    char comando[50];
    char instrucao[TAM_INSTRUCAO];

    // Inicializa registradores e memória de dados
    R[0] = 0; R[1] = 10; R[2] = 20; R[3] = 30;
    for (int i = 0; i < TAM_MEM_DADOS; i++) {
        memoria[i] = i * 1.5;
    }

    // Pré-carrega a memória de instruções com 10 instruções
    num_instrucoes = 10;
    strcpy(instrucao_mem[0], "ADD R0, R1, R2");
    strcpy(instrucao_mem[1], "SUB R3, R0, R1");
    strcpy(instrucao_mem[2], "MUL R0, R2, R3");
    strcpy(instrucao_mem[3], "DIV R1, R3, R2");
    strcpy(instrucao_mem[4], "ADD R2, R0, R3");
    strcpy(instrucao_mem[5], "SUB R1, R2, R0");
    strcpy(instrucao_mem[6], "MUL R3, R1, R2");
    strcpy(instrucao_mem[7], "DIV R0, R3, R1");
    strcpy(instrucao_mem[8], "ADD R3, R2, R1");
    strcpy(instrucao_mem[9], "SUB R0, R1, R3");

    printf("Simulador MIPS32 Interativo\n");
    escolher_modo_visualizacao();

    // Menu principal com somente INS, MODO e SAIR
    while (1) {
        printf("Comandos disponiveis:\n");
        printf("  INS  - Inserir instrucao interativa (inclui jump e demais operacoes)\n");
        printf("  MODO - Alterar modo de visualizacao\n");
        printf("  SAIR - Encerrar simulador\n\n");
        printf("> ");
        fgets(comando, sizeof(comando), stdin);
        comando[strcspn(comando, "\n")] = '\0';

        if (strcmp(comando, "SAIR") == 0) {
            break;
        } else if (strcmp(comando, "MODO") == 0) {
            escolher_modo_visualizacao();
        }
        else if (strcmp(comando, "INS") == 0) {
            // Modo INS: permite inserir instruções até que o usuário digite "." para sair.
            while (1) {
                printf("Digite a instrucao> ");
                fgets(instrucao, sizeof(instrucao), stdin);
                instrucao[strcspn(instrucao, "\n")] = '\0';
                if (strcmp(instrucao, ".") == 0) {
                    break;
                }
                executa_instrucao_programa(instrucao);
            }
        } else {
            printf("Comando nao reconhecido.\n");
        }
    }

    printf("Encerrando simulador...\n");
    return 0;
}
