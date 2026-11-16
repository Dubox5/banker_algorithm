#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_LINHA 1024
int num_recursos = 0;
int num_clientes = 0;
int *disponivel;
int **maximo;
int **alocado;
int **necessidade;

typedef struct {
    char tipo;
    int cliente;
    int *valores;
} Comando;

Comando *comandos = NULL;
int num_comandos = 0;

void erro(char *msg, int linha) {
    if (linha > 0)
        fprintf(stderr, "Erro: %s (linha %d)\n", msg, linha);
    else
        fprintf(stderr, "Erro: %s\n", msg);
    exit(1);
}

int eh_numero(char *s) {
    if (s == NULL || s[0] == '\0') return 0;
    if (strlen(s) > 9) return 0;
    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] < '0' || s[i] > '9') return 0;
    }
    return 1;
}

void le_argumentos(int argc, char *argv[]) {
    if (argc < 2)
        erro("uso: ./banker <instancias de cada recurso>, ex: ./banker 10 5 7", 0);

    num_recursos = argc - 1;
    disponivel = malloc(num_recursos * sizeof(int));

    for (int j = 0; j < num_recursos; j++) {
        if (!eh_numero(argv[j+1])) {
            fprintf(stderr, "Erro: argumento '%s' invalido, tem que ser um inteiro >= 0\n", argv[j+1]);
            exit(1);
        }
        disponivel[j] = atoi(argv[j+1]);
    }
}

int main(int argc, char *argv[]) {
    le_argumentos(argc, argv);
    return 0;
}
