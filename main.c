/*
Segundo Trabalho Prático - Organização e recuperação de dados
Alunos:
William Kenzo Tsutumi        RA: 124706
Nicolas Hess de Farina Alves RA:
Mateus Hamada                RA:
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAM_MAX_BUCKET 2

void importa_chaves(char *);
void imprime_buckets();
void imprime_diretorio();

//******************************************************

int main(int argc, char *argv[])
{
    // Importação
    if (argc == 3 && strcmp(argv[1], "-i") == 0)
    {
        printf("Modo de importacao ativado ... nome do arquivo = %s\n", argv[2]);
        importa_chaves(argv[2]);
    }
    // Impressão das informações do diretório
    else if (argc == 2 && strcmp(argv[1], "-pd") == 0)
    {
        printf("Modo de impressao do diretorio ...\n");
        imprime_diretorio();
    }
    // Impressão das informações dos buckets
    else if (argc == 2 && strcmp(argv[1], "-pb") == 0)
    {
        printf("Modo de impressao dos buckets ...\n");
        imprime_buckets();
    }
    // Uso incorreto
    else
    {
        fprintf(stderr, "Argumentos incorretos!\n");
        fprintf(stderr, "Modo de uso:\n");
        fprintf(stderr, "$ ./%s -i nome_arquivo\n", argv[0]);
        fprintf(stderr, "$ ./%s -pd/-pb\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    return 0;
}

//******************************************************

void importa_chaves(char * nome_arquivo)
{
    FILE * arq = fopen(nome_arquivo, "rb+");

    //putaquepariutemmuitopseudocodigopraimplementar

    fclose(arq);
}

//******************************************************

void imprime_buckets()
{

}

//******************************************************

void imprime_diretorio()
{

}