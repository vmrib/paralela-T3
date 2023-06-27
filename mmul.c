#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chrono.c"

void popular_matriz(double *matriz, int linhas, int colunas)
{
    for (int i = 0; i < linhas; i++)
    {
        for (int j = 0; j < colunas; j++)
        {
            matriz[i * linhas + j] = i + j;
        }
    }
}

void mult_paralela(double *A, double *B, double *C, int nla, int m, int ncb)
{
    int nproc, processId;

    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    // Nodo 0 faz scatter das matrizes A e braodcast da matriz B
    MPI_Scatter(A, nla * m / nproc, MPI_DOUBLE, A, nla * m / nproc, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, m * ncb, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Cada nodo calcula sua parte da matriz C
    for (int i = 0; i < nla / nproc; i++)
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

    // Nodo 0 faz gather das partes da matriz C
    MPI_Gather(C, nla * m / nproc, MPI_DOUBLE, C, nla * m / nproc, MPI_DOUBLE, 0, MPI_COMM_WORLD);
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

    int nproc, processId;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    double *A = (double *)malloc(nla * m * sizeof(double));
    double *B = (double *)malloc(m * ncb * sizeof(double));
    double *C = (double *)malloc(nla * ncb * sizeof(double));
    double *C2 = (double *)malloc(nla * ncb * sizeof(double));

    // Se for nodo 0
    if (processId == 0)
    {
        popular_matriz(A, nla, m);
        popular_matriz(B, m, ncb);
    }


    chronometer_t mult_chrono;
    MPI_Barrier(MPI_COMM_WORLD);
    if (processId == 0)
	{
		chrono_reset(&mult_chrono);
		chrono_start(&mult_chrono);
	}

    mult_paralela(A, B, C, nla, m, ncb);

    MPI_Barrier(MPI_COMM_WORLD);
    if (processId == 0)
    {
        chrono_stop(&mult_chrono);
        chrono_reportTime(&mult_chrono, "mult_chrono");

        double total_time_in_seconds = (double)chrono_gettotal(&mult_chrono) /
									   ((double)1000 * 1000 * 1000);

        printf("Tempo total: %lf s\n", total_time_in_seconds);

        if (sequencial)
        {
            mult_sequencial(A, B, C2, nla, m, ncb);

            if (comparar_matrizes(C, C2, nla, ncb) == 0)
                printf("Resultado paralelo OK!\n");
            else
                printf("ERRO: Resultado paralelo != resultado sequencial!\n");
        }
    }

    free(A);
    free(B);
    free(C);
    free(C2);
    MPI_Finalize();
    return 0;
}
