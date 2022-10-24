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
#define TAM_MAX_DIR 1024
#define SUCCESS 1
#define FAILURE 0

typedef struct Bucket
{
    int prof;
    int cont;
    char *chave[TAM_MAX_BUCKET];
} Bucket;

typedef struct DIR_CELL
{
    int bucket_ref;
} DIR_CELL;

void importa_chaves(char *);
void imprime_buckets();
void imprime_diretorio();
int hash(char *key, int maxaddr);
int make_address(char *key, int profundidade);
int op_add(char *key);
int op_find(char *key, Bucket *found_bucket);

//******************************************************

DIR_CELL dir_cell[TAM_MAX_DIR];
int dir_prof = 1;
FILE *chaves;
FILE *diretorio;
FILE *buckets;

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

void importa_chaves(char *nome_arquivo)
{
    chaves = fopen(nome_arquivo, "rb+");

    while (!feof(chaves))
    {
        char key[3];
        fgets(key, 3, chaves);
        op_add(key);
    }

    fclose(chaves);
}

//******************************************************

void imprime_buckets()
{

}

//******************************************************

void imprime_diretorio()
{

}

//******************************************************

int hash(char *key, int maxaddr)
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

//******************************************************

int make_address(char *key, int profundidade)
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

//******************************************************

int op_add(char *key)
{
    Bucket found_bucket;
    if (op_find(key, &found_bucket) == SUCCESS)
        return FAILURE;

    bk_add_key(key, found_bucket);
    return SUCCESS;
}

//******************************************************

int op_find(char *key, Bucket *found_bucket)
{
    int size = sizeof(dir_cell) / sizeof(dir_cell[0]);
    // essa func tem que receber a profundidade do dir_cell?
    int address = make_address(key, size);
    // Lendo o bucket no arquivo
    fseek(buckets, dir_cell[address].bucket_ref, SEEK_SET);
    fread(&(found_bucket->prof), sizeof(int), 1, buckets);
    fread(&(found_bucket->cont), sizeof(int), 1, buckets);

    int num_chaves = found_bucket->cont;
    for (int i = 0; i < num_chaves; i++)
        fread(&(found_bucket->chave[i]), sizeof(char *), 1, buckets);

    // Procurando pela chave
    for (int i = 0; i < num_chaves; i++)
    {
        if (strcmp(found_bucket->chave[i], key) == 0)
            return FAILURE;
    }
    return SUCCESS;
}