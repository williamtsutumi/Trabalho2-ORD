/*
 *   Segundo Trabalho Prático - Organização e recuperação de dados
 *   Alunos:
 *   William Kenzo Tsutumi        RA: 124706
 *   Nicolas Hess de Farina Alves RA: 125623
 *   Matheus Hamada               RA: 124101
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define TAM_MAX_BUCKET 5
#define TAM_MAX_DIR 1024
#define TAM_MAX_CHAVE 255
#define SUCCESS 1
#define FAILURE 0
#define LOW_VALUE -1

typedef struct Bucket
{
    int prof;
    int cont;
    int chave[TAM_MAX_BUCKET];
} Bucket;

typedef struct DIR_CzELL
{
    int bucket_ref;
} DIR_CELL;

DIR_CELL dir_cell[TAM_MAX_DIR];
int dir_prof = 0;

FILE *diretorio;
FILE *buckets;

/*
 *   Funções
 */
void importa_chaves(char *);
void imprime_buckets();
void imprime_diretorio();
int hash(int);
int make_address(int, int);
int op_add(int);
int op_find(int, Bucket *);
void bk_add_key(int, Bucket *);
void bk_split(Bucket *);
void find_new_range(Bucket *, int *, int *);
void dir_ins_bucket(int, int, int);
void dir_double();
void inicializacao();
void finalizacao();

/*
 *   Main
 */
int main(int argc, char *argv[])
{
    // Importação
    if (argc == 3 && strcmp(argv[1], "-i") == 0)
    {
        inicializacao();
        printf("Modo de importacao ativado ... nome do arquivo = %s\n", argv[2]);
        importa_chaves(argv[2]);
    }
    // Impressão das informações do diretório
    else if (argc == 2 && strcmp(argv[1], "-pd") == 0)
    {
        inicializacao();
        printf("Modo de impressao do diretorio ...\n");
        imprime_diretorio();
    }
    // Impressão das informações dos buckets
    else if (argc == 2 && strcmp(argv[1], "-pb") == 0)
    {
        inicializacao();
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

/*
 *   Lê as chaves do arquivo "chaves.txt" e chama a função "op_add"
 */
void importa_chaves(char *nome_arquivo)
{
    FILE *chaves;
    chaves = fopen(nome_arquivo, "r");

    char buffer_chave[TAM_MAX_CHAVE];
    int chave;
    // Colocando as chaves
    while (!feof(chaves))
    {
        fgets(buffer_chave, TAM_MAX_CHAVE, chaves);
        chave = atoi(buffer_chave);
        printf("Importar: %d \n\n", chave);
        op_add(chave);
    }

    fclose(chaves);
}

/*
 *   Lê e imprime os dados armazenados no arquivo "buckets.dat"
 */
void imprime_buckets()
{
    Bucket n;

    while (!feof(buckets))
    {
        if (fread(&n, sizeof(Bucket), 1, buckets) == 0)
            break;

        printf("Bucket %d (Prof = %d)\n", ftell(buckets) / sizeof(Bucket) - 1, n.prof);
        for (int i = 0; i < TAM_MAX_BUCKET; i++)
        {
            printf("Chave [%d] = %d\n", i, n.chave[i]);
        }
        printf("\n");
    }
}

/*
 *   Imprime os dados armazenados no vetor de diretório
 */
void imprime_diretorio()
{
    int num_buckets = 1;
    int last_bucket_ref = dir_cell[0].bucket_ref;
    for (int i = 0; i < pow(2, dir_prof); i++)
    {
        printf("dir[%d] = bucket(%d)\n", i, dir_cell[i].bucket_ref);

        if (dir_cell[i].bucket_ref != last_bucket_ref)
            num_buckets++;

        last_bucket_ref = dir_cell[i].bucket_ref;
    }
    printf("\n");
    printf("Profundidade: %d\n", dir_prof);
    printf("Tamanho atual: %.0lf\n", pow(2, dir_prof));
    printf("Total de buckets: %d\n", num_buckets);
    printf("\n");
}

/*
 *   Função Hash
 */
int hash(int key)
{
    return (key);
}

/*
 *   Inverte os bits da função hash de acordo com a profundidade
 */
int make_address(int key, int profundidade)
{
    int lowbit;
    int retval = 0;
    int mask = 1;
    int hashval = hash(key);

    for (int i = 0; i < profundidade; i++)
    {
        retval = retval << 1;
        lowbit = hashval & mask;
        retval = retval | lowbit;
        hashval = hashval >> 1;
    }
    return retval; //(profundidade)bits de trás para frente(hashval)
}

/*
 *   Encontra o bucket onde a chave deve ser adicionada.
 *   Se a chave não existir no bucket a chave é repassada com o bucket encontrado
 *   para a função "bk_add_key"
 */
int op_add(int key)
{
    Bucket found_bucket;
    if (op_find(key, &found_bucket) == SUCCESS)
        return FAILURE;

    bk_add_key(key, &found_bucket);
    return SUCCESS;
}

/*
 *   Procura pelo bucket para a chave.
 *   Verifica se a chave existe no bucket.
 */
int op_find(int key, Bucket *found_bucket)
{
    int address = make_address(key, dir_prof);
    // Lendo o bucket no arquivo
    fseek(buckets, (dir_cell[address].bucket_ref * sizeof(Bucket)), SEEK_SET);
    fread(found_bucket, sizeof(Bucket), 1, buckets);
    // Procurando pela chave

    for (int i = 0; i < (*found_bucket).cont; i++)
    {
        if ((*found_bucket).chave[i] == key)
            return SUCCESS;
    }
    return FAILURE;
}

/*
 *  Caso exista espaço no bucket adiciona a chave e reescreve os dados
 *  no arquivo "buckets.dat".
 *  Caso contrário, chama a função "bk_split" para criar/redistribuir buckets.
 *  Chama "op_add" ocasionando uma recursão indireta.
 */
void bk_add_key(int key, Bucket *found_bucket)
{
    if ((*found_bucket).cont < TAM_MAX_BUCKET)
    {
        // Encontra a primeira posição vazia;
        for (int i = 0; i < TAM_MAX_BUCKET; i++)
        {
            if ((*found_bucket).chave[i] == LOW_VALUE)
            {
                (*found_bucket).chave[i] = key;
                (*found_bucket).cont++;
                break;
            }
        }

        fseek(buckets, (long)-sizeof(Bucket), SEEK_CUR);
        fwrite(found_bucket, sizeof(Bucket), 1, buckets);
    }
    else
    {
        bk_split(found_bucket);
        op_add(key);
    }
}

/*
 *  Caso a profundidade do bucket encontrado seja a mesma do diretório chama
 *  a função "dir_double" para aumentar a profundidade do diretório.
 *  Atribui o RRN do novo bucket para os diretórios necessários.
 *  Redistribui as chaves entre o bucket encontrado e o novo bucket.
 *  Reescreve os dados alterados no arquivo "buckets.dat".
 */
void bk_split(Bucket *found_bucket)
{
    if ((*found_bucket).prof == dir_prof)
        dir_double();

    Bucket new_bucket;
    new_bucket.cont = 0;

    // Atribui como 0 o valor inicial das chaves de um novo bucket
    for (int i = 0; i < TAM_MAX_BUCKET; i++)
        new_bucket.chave[i] = LOW_VALUE;

    long int cur_bucket = ftell(buckets) - sizeof(Bucket);

    fseek(buckets, 0, SEEK_END);

    // RRN
    int end_new_bucket = (ftell(buckets)) / sizeof(Bucket);

    int new_start;
    int new_end;

    find_new_range(found_bucket, &new_start, &new_end);
    dir_ins_bucket(end_new_bucket, new_start, new_end);

    (*found_bucket).prof++;
    new_bucket.prof = found_bucket->prof;

    // Redistribuição de chaves
    for (int i = 0; i < TAM_MAX_BUCKET; i++)
    {
        if ((*found_bucket).chave[i] != LOW_VALUE)
        {
            int address = make_address((*found_bucket).chave[i], (*found_bucket).prof);

            if (address >= new_start && address <= new_end)
            {
                new_bucket.chave[new_bucket.cont] = (*found_bucket).chave[i];
                new_bucket.cont++;
                (*found_bucket).chave[i] = LOW_VALUE;
                (*found_bucket).cont--;
            }
        }
    }

    fwrite(&new_bucket, sizeof(Bucket), 1, buckets);

    fseek(buckets, cur_bucket, SEEK_SET);
    fwrite(found_bucket, sizeof(Bucket), 1, buckets);
}

/*
 *   Duplica o tamanho do diretório, ou seja, aumenta a profundidade do diretório.
 *   Obs.: Mantém as referências prévias.
 */
void dir_double()
{
    int tam_atual = pow(2, dir_prof);
    int tam_novo = tam_atual * 2;
    DIR_CELL new_dir_cell[tam_novo];

    for (int i = 0; i < tam_atual; i++)
    {
        new_dir_cell[2 * i].bucket_ref = dir_cell[i].bucket_ref;
        new_dir_cell[2 * i + 1].bucket_ref = dir_cell[i].bucket_ref;
    }

    for (int i = 0; i < tam_novo; i++)
    {
        dir_cell[i].bucket_ref = new_dir_cell[i].bucket_ref;
    }
    dir_prof++;
}

/*
 *   Encontra o intervalo de indices do diretório em que o RRN do novo bucket será atribuído.
 */
void find_new_range(Bucket *old_bucket, int *new_start, int *new_end)
{
    int mask = 1;
    int shared_address;

    for (int i = 0; i < TAM_MAX_BUCKET; i++)
    {
        if ((*old_bucket).chave[i] != LOW_VALUE)
        {
            shared_address = make_address((*old_bucket).chave[i], (*old_bucket).prof);
            break;
        }   
    }

    shared_address = shared_address << 1;
    shared_address = shared_address | mask;

    int bits_to_fill = dir_prof - ((*old_bucket).prof + 1);

    *new_start = *new_end = shared_address;

    for (int i = 0; i < bits_to_fill; i++)
    {
        *new_start = *new_start << 1;
        *new_end = *new_end << 1;
        *new_end = *new_end | mask;
    }
}

/*
 *   Insere o RRN do novo bucket no intervalo encontrado do vetor diretório.
 */
void dir_ins_bucket(int bucket_address, int start, int end)
{
    for (int i = start; i <= end; i++)
    {
        dir_cell[i].bucket_ref = bucket_address;
    }
}

/*
 *   Inicializa os arquivos "dir.dat" e "buckets.dat"
 *   Se existir "dir.dat" -> importa o vetor de diretórios(dir_cell)
 *   Senão, cria um vetor novo e os arquivos "dir.dat" e "buckets.dat"
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

        fseek(diretorio, 0, SEEK_SET);

        for (i = 0; !feof(diretorio); i++)
        {
            fread(&dir_cell[i], sizeof(dir_cell[i]), 1, diretorio);
        }

        i--;

        dir_prof = log2(i);
    }
    else
    {
        DIR_CELL celula;
        celula.bucket_ref = 0;

        dir_cell[0] = celula;

        buckets = fopen("buckets.dat", "wb+");
        fseek(buckets, 0, SEEK_SET);

        Bucket buck;
        buck.prof = 0;
        buck.cont = 0;

        for (int i = 0; i < TAM_MAX_BUCKET; i++)
            buck.chave[i] = LOW_VALUE;


        fwrite(&buck, sizeof(Bucket), 1, buckets);

        diretorio = fopen("dir.dat", "wb+");
    }
}

/*
 *   Escreve o vetor de diretórios(dir_cell) no arquivo "dir.dat".
 */
void finalizacao()
{
    int dir_cont = pow(2, dir_prof);

    fseek(diretorio, 0, SEEK_SET);

    int i;

    for (i = 0; i < dir_cont; i++)
    {
        fwrite(&dir_cell[i], sizeof(DIR_CELL), 1, diretorio);
    }

    fclose(diretorio);
    fclose(buckets);
}