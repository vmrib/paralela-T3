#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void popularMatriz(double *matriz, int linhas, int colunas)
{
    for (int i = 0; i < linhas; i++)
    {
        for (int j = 0; j < colunas; j++)
        {
            matriz[i * linhas + j] = i + j;
        }
    }
}

void mult_sequencial(double *A, double *B, double *C, int nla, int m, int ncb)
{
    for (int i = 0; i < nla; i++)
    {
        for (int j = 0; j < ncb; j++)
        {
            C[i * nla + j] = 0;
            for (int k = 0; k < m; k++)
            {
                C[i * nla + j] += A[i * nla + k] * B[k * m + j];
            }
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

    int nproc, processId;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    double A[nla * m], B[m * ncb], C[nla * ncb];

    // Se for nodo 0
    if (processId == 0)
    {
        popularMatriz(A, nla, m);
        popularMatriz(B, m, ncb);
        if (sequencial)
            mult_sequencial(A, B, C, nla, m, ncb);

    }

    return 0;
}
