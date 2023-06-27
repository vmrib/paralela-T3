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