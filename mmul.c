#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void popularMatriz(int *matriz, int linhas, int colunas)
{
    int i, j;
    for (i = 0; i < linhas; i++)
    {
        for (j = 0; j < colunas; j++)
        {
            matriz[i * linhas + j] = i + j;
        }
    }
}

int main(int argc, char *argv[])
{
    // Verifica se foram passados os argumentos obrigatórios
    if (argc < 4)
    {
        printf("Uso: programa <arg1> <arg2> <arg3> [-v]\n");
        return 1;
    }

    // Argumentos obrigatórios
    int nla = atoi(argv[1]);
    int m = atoi(argv[2]);
    int ncb = atoi(argv[3]);

    // Verifica se o argumento opcional "-v" foi fornecido
    int sequencial = 0;
    if (argc > 4 && strcmp(argv[4], "-v") == 0)
    {
        sequencial = 1;
    }

    // Verifica se a opção "-v" foi fornecida
    if (sequencial)
        printf("Modo sequencial ativado.\n");

    return 0;
}
