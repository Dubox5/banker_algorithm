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

void tira_quebra(char *s) {
    int n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

int linha_vazia(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    return *s == '\0';
}

void imprime_vetor(FILE *f, int *v) {
    for (int j = 0; j < num_recursos; j++) {
        if (j > 0) fprintf(f, " ");
        fprintf(f, "%d", v[j]);
    }
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

void le_clientes() {
    FILE *f = fopen("customer.txt", "r");
    if (f == NULL) erro("nao consegui abrir o customer.txt", 0);

    char linha[TAM_LINHA];
    int nlinha = 0;
    int teve_vazia = 0;

    while (fgets(linha, TAM_LINHA, f) != NULL) {
        nlinha++;
        tira_quebra(linha);

        if (linha_vazia(linha)) {
            teve_vazia = 1;
            continue;
        }
        if (teve_vazia) erro("customer.txt tem linha vazia no meio dos clientes", nlinha - 1);

        int virgulas = 0;
        for (int i = 0; linha[i] != '\0'; i++)
            if (linha[i] == ',') virgulas++;
        if (virgulas != num_recursos - 1)
            erro("quantidade de recursos no customer.txt nao bate com a quantidade de argumentos", nlinha);

        int *linha_max = malloc(num_recursos * sizeof(int));
        int qtd = 0;
        char *tok = strtok(linha, ",");
        while (tok != NULL) {
            if (!eh_numero(tok))
                erro("formato invalido no customer.txt, era pra ser so numeros separados por virgula", nlinha);
            linha_max[qtd] = atoi(tok);
            qtd++;
            tok = strtok(NULL, ",");
        }
        if (qtd != num_recursos)
            erro("formato invalido no customer.txt, tem campo vazio", nlinha);

        maximo = realloc(maximo, (num_clientes + 1) * sizeof(int *));
        alocado = realloc(alocado, (num_clientes + 1) * sizeof(int *));
        necessidade = realloc(necessidade, (num_clientes + 1) * sizeof(int *));

        maximo[num_clientes] = linha_max;
        alocado[num_clientes] = calloc(num_recursos, sizeof(int));
        necessidade[num_clientes] = malloc(num_recursos * sizeof(int));
        for (int j = 0; j < num_recursos; j++)
            necessidade[num_clientes][j] = linha_max[j];

        num_clientes++;
    }
    fclose(f);

    if (num_clientes == 0) erro("customer.txt esta vazio", 0);
}

void le_comandos() {
    FILE *f = fopen("commands.txt", "r");
    if (f == NULL) erro("nao consegui abrir o commands.txt", 0);

    char linha[TAM_LINHA];
    int nlinha = 0;

    while (fgets(linha, TAM_LINHA, f) != NULL) {
        nlinha++;
        tira_quebra(linha);
        if (linha_vazia(linha)) continue;

        Comando c;
        char *tok = strtok(linha, " \t");

        if (strcmp(tok, "*") == 0) {
            if (strtok(NULL, " \t") != NULL)
                erro("no commands.txt o * tem que ficar sozinho na linha", nlinha);
            c.tipo = '*';
            c.cliente = -1;
            c.valores = NULL;
        }
        else if (strcmp(tok, "RQ") == 0 || strcmp(tok, "RL") == 0) {
            if (strcmp(tok, "RQ") == 0) c.tipo = 'Q';
            else c.tipo = 'L';

            tok = strtok(NULL, " \t");
            if (!eh_numero(tok))
                erro("numero do cliente invalido no commands.txt", nlinha);
            c.cliente = atoi(tok);
            if (c.cliente >= num_clientes)
                erro("cliente do commands.txt nao existe no customer.txt", nlinha);

            c.valores = malloc(num_recursos * sizeof(int));
            int qtd = 0;
            tok = strtok(NULL, " \t");
            while (tok != NULL) {
                if (qtd == num_recursos) { qtd++; break; }
                if (!eh_numero(tok))
                    erro("recurso invalido no commands.txt, tem que ser inteiro >= 0", nlinha);
                c.valores[qtd] = atoi(tok);
                qtd++;
                tok = strtok(NULL, " \t");
            }
            if (qtd != num_recursos)
                erro("quantidade de recursos no commands.txt nao bate com a quantidade de argumentos", nlinha);
        }
        else {
            erro("comando desconhecido no commands.txt (so vale RQ, RL ou *)", nlinha);
        }

        comandos = realloc(comandos, (num_comandos + 1) * sizeof(Comando));
        comandos[num_comandos] = c;
        num_comandos++;
    }
    fclose(f);
}


int estado_seguro() {
    int *work = malloc(num_recursos * sizeof(int));
    int *terminou = calloc(num_clientes, sizeof(int));
    for (int j = 0; j < num_recursos; j++) work[j] = disponivel[j];

    int cont = 0;
    int achou = 1;
    while (achou) {
        achou = 0;
        for (int i = 0; i < num_clientes; i++) {
            if (terminou[i]) continue;

            int da = 1;
            for (int j = 0; j < num_recursos; j++) {
                if (necessidade[i][j] > work[j]) { da = 0; break; }
            }
            if (da) {
                for (int j = 0; j < num_recursos; j++) work[j] += alocado[i][j];
                terminou[i] = 1;
                cont++;
                achou = 1;
            }
        }
    }
    free(work);
    free(terminou);
    return cont == num_clientes;
}

void pedido(FILE *out, int c, int *req) {
    for (int j = 0; j < num_recursos; j++) {
        if (req[j] > necessidade[c][j]) {
            fprintf(out, "The customer %d request ", c);
            imprime_vetor(out, req);
            fprintf(out, " was denied because exceed its maximum allocation\n");
            return;
        }
    }
    for (int j = 0; j < num_recursos; j++) {
        if (req[j] > disponivel[j]) {
            fprintf(out, "The resources ");
            imprime_vetor(out, disponivel);
            fprintf(out, " are not enough to customer %d request ", c);
            imprime_vetor(out, req);
            fprintf(out, "\n");
            return;
        }
    }
    for (int j = 0; j < num_recursos; j++) {
        disponivel[j] -= req[j];
        alocado[c][j] += req[j];
        necessidade[c][j] -= req[j];
    }
    if (estado_seguro()) {
        fprintf(out, "Allocate to customer %d the resources ", c);
        imprime_vetor(out, req);
        fprintf(out, "\n");
    } else {
        for (int j = 0; j < num_recursos; j++) {
            disponivel[j] += req[j];
            alocado[c][j] -= req[j];
            necessidade[c][j] += req[j];
        }
        fprintf(out, "The customer %d request ", c);
        imprime_vetor(out, req);
        fprintf(out, " was denied because result in an unsafe state\n");
    }
}

void libera(FILE *out, int c, int *rel) {
    for (int j = 0; j < num_recursos; j++) {
        if (rel[j] > alocado[c][j]) {
            fprintf(out, "The customer %d release ", c);
            imprime_vetor(out, rel);
            fprintf(out, " was denied because exceed its allocation\n");
            return;
        }
    }
    for (int j = 0; j < num_recursos; j++) {
        disponivel[j] += rel[j];
        alocado[c][j] -= rel[j];
        necessidade[c][j] += rel[j];
    }
    fprintf(out, "Release from customer %d the resources ", c);
    imprime_vetor(out, rel);
    fprintf(out, "\n");
}

void imprime_estado(FILE *out) {
    fprintf(out, "MAXIMUM | ALLOCATION | NEED\n");
    for (int i = 0; i < num_clientes; i++) {
        imprime_vetor(out, maximo[i]);
        fprintf(out, " | ");
        imprime_vetor(out, alocado[i]);
        fprintf(out, " | ");
        imprime_vetor(out, necessidade[i]);
        fprintf(out, "\n");
    }
    fprintf(out, "AVAILABLE ");
    imprime_vetor(out, disponivel);
    fprintf(out, "\n");
}


int main(int argc, char *argv[]) {
    le_argumentos(argc, argv);
    le_clientes();
    le_comandos();

    FILE *out = fopen("result.txt", "w");
    if (out == NULL) erro("nao consegui criar o result.txt", 0);

    for (int k = 0; k < num_comandos; k++) {
        if (comandos[k].tipo == 'Q')
            pedido(out, comandos[k].cliente, comandos[k].valores);
        else if (comandos[k].tipo == 'L')
            libera(out, comandos[k].cliente, comandos[k].valores);
        else
            imprime_estado(out);
    }

    fclose(out);
    return 0;
}
