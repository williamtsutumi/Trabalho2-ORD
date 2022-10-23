/*
Segundo Trabalho Prático - Organização e recuperação de dados
Alunos:
William Kenzo Tsutumi        RA: 124706
Nicolas Hess de Farina Alves RA:
Matheus Hamada               RA: 124101
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAM_MAX_BUCKET 2

typedef struct Bucket{
    int prof;
    int cont;
    char *chave[TAM_MAX_BUCKET];
} Bucket;

typedef struct DIR_CELL{
    int bucket_ref;
} DIR_CELL;

void importa_chaves(char *);
void imprime_buckets();
void imprime_diretorio();
int hash(char *key[], int maxaddr);
void make_address(char *key[], int profundidade);
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

int hash(char *key[], int maxaddr)
{
    short int sum = 0;
    int i = 0;

    while (key[i] != '\r')
    {
        sum = (sum + 100 * (int)key[i] + (int)key[i + 1]) % maxaddr;
        i = i + 2;
    }

    return (sum % maxaddr);
}

void make_address(char *key[], int profundidade)
{
    int retval = 0;
    int lowbit = 0;
    int mask = 1;
    short int hashval = hash(key, profundidade);

    for (int i = 0; i < profundidade; i++)
    {
        retval = retval << 1;
        lowbit = hashval & mask;
        retval = retval | lowbit;
        hashval = hashval >> 1;
    }

    return retval;
}