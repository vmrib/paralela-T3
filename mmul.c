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
            matriz[i * colunas + j] = i + j;
        }
    }
}

void mult_paralela(double *A, double *B, double *C, int nla, int m, int ncb)
{
    int nproc, processId;

    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    // Nodo 0 faz scatter das matrizes A e braodcast da matriz B
    MPI_Scatter(A, (nla / nproc) * m , MPI_DOUBLE, A, (nla / nproc) * m, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, m * ncb, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Cada nodo calcula sua parte da matriz C
    for (int i = 0; i < nla / nproc; i++)
    {
        for (int j = 0; j < ncb; j++)
        {
            C[i * ncb + j] = 0;
            for (int k = 0; k < m; k++)
            {
                C[i * ncb + j] += A[i * m + k] * B[k * ncb + j];
            }
        }
    }

    // Nodo 0 faz gather das partes da matriz C
    MPI_Gather(C, (nla / nproc) * ncb, MPI_DOUBLE, C, (nla / nproc) * ncb, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void mult_sequencial(double *A, double *B, double *C, int nla, int m, int ncb)
{
    for (int i = 0; i < nla; i++)
    {
        for (int j = 0; j < ncb; j++)
        {
            C[i * ncb + j] = 0;
            for (int k = 0; k < m; k++)
            {
                C[i * ncb + j] += A[i * m + k] * B[k * ncb + j];
            }
        }
    }
}

int comparar_matrizes(const double *matriz1, const double *matriz2, int linhas, int colunas)
{
    for (int i = 0; i < linhas * colunas; i++)
    {
        if (matriz1[i] != matriz2[i])
        {
            return 0; // diferentes
        }
    }
    return 1; // iguais
}

void imprimir_matriz(const double *matriz, int linhas, int colunas)
{
    printf("Matriz %dx%d:\n", linhas, colunas);
    for (int i = 0; i < linhas; i++)
    {
        for (int j = 0; j < colunas; j++)
        {
            printf("%lf ", matriz[i * colunas + j]);
        }
        printf("\n");
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
    int verificador = 0;
    if (argc > 4 && strcmp(argv[4], "-v") == 0)
    {
        verificador = 1;
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

    if (nproc > 1)
        mult_paralela(A, B, C, nla, m, ncb);
    else
        mult_sequencial(A, B, C, nla, m, ncb);

    MPI_Barrier(MPI_COMM_WORLD);

    if (processId == 0)
    {
        chrono_stop(&mult_chrono);
    }

    if (processId == 0 && verificador && nproc > 1)
    {
        mult_sequencial(A, B, C2, nla, m, ncb);

        if (comparar_matrizes(C, C2, nla, ncb) == 1)
        {
            printf("Resultado paralelo OK!\n");
        }
        else
        {
            printf("ERRO: Resultado paralelo != resultado sequencial!\n");
        }
    }
    else if (processId == 0 && verificador && nproc == 1)
    {
        printf("Com 1 processo executamos apenas a versao sequencial!\n");
    }
    else if (processId == 0)
    {
        double total_time_in_seconds = (double)chrono_gettotal(&mult_chrono) /
                                    ((double)1000 * 1000 * 1000);

        printf("Tempo total: %lf s\n", total_time_in_seconds);
        
        double GFLOPS = (((double)nla * ncb * m) / ((double)total_time_in_seconds * 1000 * 1000 * 1000));
		printf("Vazao: %lf GFLOPS\n", GFLOPS);
    }

    // Get the name of the processor
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int name_len;
	MPI_Get_processor_name(processor_name, &name_len);

    // Print node info
    if (!verificador && nproc > 1)
	    printf("Rank %d no Host %s com %d processos do MPI\n", processId, processor_name, nproc);

    free(A);
    free(B);
    free(C);
    free(C2);
    MPI_Finalize();
    return 0;
}
