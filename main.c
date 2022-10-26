/*
Segundo Trabalho Prático - Organização e recuperação de dados
Alunos:
William Kenzo Tsutumi        RA: 124706
Nicolas Hess de Farina Alves RA: 125623
Matheus Hamada               RA: 124101
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define TAM_MAX_BUCKET 2
#define TAM_MAX_DIR 1024
#define TAM_MAX_CHAVE 255
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
void bk_add_key(char *key, Bucket *found_bucket);
void bk_split(Bucket *found_bucket);
void find_new_range(Bucket *old_bucket, int new_start, int new_end);
void dir_ins_bucket(int bucket_address, int start, int end);
void dir_double();
void inicializacao();
void finalizacao();
//******************************************************

DIR_CELL *dir_cell;
Bucket all_buckets[TAM_MAX_DIR];
int num_buckets = 0;
int dir_prof = 0;
FILE *diretorio;
FILE *buckets;

//******************************************************

int main(int argc, char *argv[])
{
    inicializacao();
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
    finalizacao();
    return 0;
}

//******************************************************

void importa_chaves(char *nome_arquivo)
{
    FILE *chaves;
    chaves = fopen(nome_arquivo, "r");

    char buffer_chave[TAM_MAX_CHAVE];
    // Colocando os buckets em memória
    while (!feof(chaves))
    {
        fgets(buffer_chave, TAM_MAX_CHAVE, chaves);
        op_add(buffer_chave);
    }
    // Colocando os buckets no arquivo
    int num_buckets = pow(2, dir_prof);
    printf("Escrevendo os buckets em buckets.dat\n");
    printf("Escrevendo o diretorio em dir.dat\n");
    printf("%d\n", num_buckets);
    for (int i = 0; i < num_buckets; i++)
    {
        fwrite(&(dir_cell[i].bucket_ref), sizeof(int), 1, diretorio);
        if (i == num_buckets - 1 || dir_cell[i].bucket_ref != dir_cell[i + 1].bucket_ref)
        {
            int idx = dir_cell[i].bucket_ref;
            fwrite(&idx, sizeof(int), 1, diretorio);
            fwrite(&(all_buckets[i].prof), sizeof(int), 1, buckets);
            fwrite(&(all_buckets[i].cont), sizeof(int), 1, buckets);
            for (int j = 0; j < all_buckets[i].cont; j++)
                fwrite(all_buckets[i].chave[j], sizeof(int), 1, buckets);
        }
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
        printf("%d\n", 1);
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

    return retval; //(profundidade)*bits de trás para frente(hashval)
}

//******************************************************

int op_add(char *key)
{
    Bucket found_bucket;
    if (op_find(key, &found_bucket) == SUCCESS)
        return FAILURE;

    bk_add_key(key, &found_bucket);
    return SUCCESS;
}

//******************************************************

int op_find(char *key, Bucket *found_bucket)
{
    int address = make_address(key, dir_prof);
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

//******************************************************

/**
 *  Adiciona key em found_bucket, caso haja espaço
 *  Caso contrário, nao sei
 *
 *
 *
 */
void bk_add_key(char *key, Bucket *found_bucket)
{
    int cont = found_bucket->cont;
    if (cont < TAM_MAX_BUCKET)
    {
        found_bucket->chave[cont] = key;
        found_bucket->cont++;
        // fwrite(key, sizeof(char), 1, buckets); certo?/errado? -> insira key em bucket
        // Acho que tem que mecher só em memória aqui
    }
    else
    {
        bk_split(found_bucket);
        op_add(key);
    }
}

//******************************************************

/*
 *
 */
void bk_split(Bucket *found_bucket)
{
    if (found_bucket->prof == dir_prof)
        dir_double();

    Bucket new_bucket;

    new_bucket.prof = found_bucket->prof + 1;

    int new_start = make_address(found_bucket->chave[0], new_bucket.prof);
    int new_end = make_address(found_bucket->chave[0], new_bucket.prof + 1);
    int end_new_bucket = new_end - 1;

    find_new_range(found_bucket, new_start, new_end);
    dir_ins_bucket(end_new_bucket, new_start, new_end);

    found_bucket->prof + 1;

    new_bucket.prof = found_bucket->prof;

    for (int i = 0; i < TAM_MAX_BUCKET; i++)
    {
        if (make_address(found_bucket->chave[i], found_bucket->prof) == new_start)
        {
            bk_add_key(found_bucket->chave[i], &new_bucket);
            found_bucket->chave[i] = NULL;
            found_bucket->cont--;
        }
    }
    all_buckets[num_buckets] = new_bucket;
    num_buckets++;
}

void dir_double()
{
    int tam_atual = pow(2, dir_prof);
    int tam_novo = tam_atual * 2;
    DIR_CELL *novo_dir = (DIR_CELL *)malloc(sizeof(DIR_CELL) * tam_novo);

    for (int i = 0; i < tam_atual; i++)
    {
        novo_dir[2 * i].bucket_ref = dir_cell[i].bucket_ref;
        novo_dir[2 * i + 1].bucket_ref = dir_cell[i].bucket_ref;
    }
    dir_cell = novo_dir;
    dir_prof++;
}

void find_new_range(Bucket *old_bucket, int new_start, int new_end)
{
    int mask = 1;
    int shared_address = make_address(old_bucket->chave[0], old_bucket->prof);

    shared_address = shared_address << 1;
    shared_address = shared_address | mask;

    int bits_to_fill = dir_prof - (old_bucket->prof + 1);

    new_start = new_end = shared_address;

    for (int i = 0; i < bits_to_fill; i++)
    {
        new_start = new_start << 1;
        new_end = new_end << 1;
        new_end = new_end | mask;
    }
}
//******************************************************

void dir_ins_bucket(int bucket_address, int start, int end)
{
    for (int i = start; i <= end; i++)
    {
        dir_cell[i].bucket_ref = bucket_address;
    }
}

/*
 *   Inicializa os arquivos dir.dat e buckets.dat
 *   Se existir dir.dat -> importa o vetor de diretórios(dir_cell)
 *   Senão, cria um vetor novo e os arquivos dir.dat e buckets.dat
 */
void inicializacao()
{
    diretorio = fopen("dir.dat", "rb+");

    if (diretorio != NULL)
    {
        buckets = fopen("buckets.dat", "rb+");

        if (buckets == NULL)
        {
            fprintf(stderr, ">> Erro: Falha na leitura do arquivo: buckets.dat\n");
            exit(1);
        }

        int i;

        for (i = 0; feof(diretorio); i++)
        {
            fread(&dir_cell[i], sizeof(DIR_CELL), 1, diretorio);
        }

        i++;

        dir_prof = log2(i);
    }
    else
    {
        DIR_CELL celula;
        celula.bucket_ref = 0;

        dir_cell[0] = celula;

        buckets = fopen("buckets.dat", "wb+");

        Bucket buck;
        buck.prof = 0;
        buck.cont = 0;

        fwrite(&buck, sizeof(Bucket), 1, buckets);

        diretorio = fopen("dir.dat", "wb+");
    }
}

//******************************************************

/*
 *   Escreve o vetor de diretórios(dir_cell) no arquivo dir.dat
 */
void finalizacao()
{
    int dir_cont = pow(2, dir_prof);

    fseek(diretorio, 0, SEEK_SET);

    for (int i = 0; i < dir_cont; i++)
    {
        fwrite(&dir_cell[i], sizeof(DIR_CELL), 1, diretorio);
    }

    fclose(diretorio);
    fclose(buckets);
}