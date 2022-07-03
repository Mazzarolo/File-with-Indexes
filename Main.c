#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
    int matr;
    char nome[25];
    int nota1;
    int nota2;
    int excluido;
} reg_aluno;

// TODO: Implementar um indice (em memoria) para facilitar a busca dos registros do arquivo
// TODO: Alterar as funcoes de busca, insercao e exclusao para usar o indice
typedef struct
{
    int matr;
    int pos_seek;
} indice;

typedef struct no
{
    indice idx;
    struct no *prox;
} No;

No *listaLocalizaPos(No *inicio, int matr)
{
    No *p = inicio;
    No *ant = NULL;

    while (p != NULL && p->idx.matr < matr)
    {
        ant = p;
        p = p->prox;
    }
    return ant;
}

static void listaInsere(No **inicio, int matr, int pos_seek, No *ant)
{
    No *novo = (No *)malloc(sizeof(No));
    novo->idx.matr = matr;
    novo->idx.pos_seek = pos_seek;

    if (ant == NULL)
    {
        novo->prox = *inicio;
        *inicio = novo;
    }
    else
    {
        novo->prox = ant->prox;
        ant->prox = novo;
    }
}

void listaInsereOrdenado(No **inicio, int matr, int pos_seek)
{
    No *ant = listaLocalizaPos(*inicio, matr);
    listaInsere(inicio, matr, pos_seek, ant);
}

void criaLista(FILE *arq, No **inicio)
{
    *inicio = NULL;

    reg_aluno aluno;
    rewind(arq);
    while (fread(&aluno, sizeof(aluno), 1, arq))
        if (aluno.excluido == 0)
            listaInsereOrdenado(inicio, aluno.matr, ftell(arq) - sizeof(aluno));
}

void retirarLista(No **inicio, int matr)
{
    No *ant = NULL;
    No *p = *inicio;

    while (p != NULL && p->idx.matr != matr)
    {
        ant = p;
        p = p->prox;
    }

    if (p == NULL)
        return;

    if (ant == NULL)
        *inicio = p->prox;
    else
        ant->prox = p->prox;

    free(p);
}

void liberarLista(No **inicio)
{
    No *p = *inicio;
    while (p != NULL)
    {
        No *t = p->prox;
        free(p);
        p = t;
    }
    *inicio = NULL;
}

void mostra(FILE *arq)
{
    reg_aluno aluno;
    rewind(arq);
    while (fread(&aluno, sizeof(aluno), 1, arq))
        if (aluno.excluido == 0)
            printf("%d\t%s\t%d\t%d\n", aluno.matr, aluno.nome, aluno.nota1, aluno.nota2);
}

int pesquisa(FILE *arq, No *inicio, int matr, reg_aluno *al)
{
    reg_aluno aluno;
    No *p = inicio;

    while (p != NULL)
    {
        if (p->idx.matr == matr)
        {
            rewind(arq);
            fseek(arq, p->idx.pos_seek, SEEK_SET);
            fread(&aluno, sizeof(aluno), 1, arq);
            if (!aluno.excluido)
            {
                *al = aluno;
                return 1;
            }
        }
        p = p->prox;
    }

    return 0;
}

void exclui(FILE *arq, No **lst, int matr)
{
    reg_aluno aluno;
    if (pesquisa(arq, *lst, matr, &aluno))
    {
        int excl = 1;
        printf("Excluindo: %s\n", aluno.nome);
        fseek(arq, -1 * sizeof(int), SEEK_CUR);
        fwrite(&excl, sizeof(int), 1, arq);
        fflush(arq);
        retirarLista(lst, matr);
    }
}

void inclui(FILE *arq, No **lst)
{
    reg_aluno aluno;

    fseek(arq, 0, SEEK_END);
    // printf("Pos=%ld", ftell(arq));
    printf("Informe os dados do aluno (matr, nome, nota1 e nota2) \n");
    scanf("%d%s%d%d", &aluno.matr, aluno.nome, &aluno.nota1, &aluno.nota2);
    aluno.excluido = 0;
    listaInsereOrdenado(lst, aluno.matr, ftell(arq));
    fwrite(&aluno, sizeof(aluno), 1, arq);
}

void main()
{
    int matr, op;
    reg_aluno aluno;
    FILE *arq;
    No *lst;

    if (access("alunos.dat", F_OK) == 0)
        arq = fopen("alunos.dat", "r+"); // arquivo existe
    else
        arq = fopen("alunos.dat", "w+"); // arquivo nao existia
    criaLista(arq, &lst);
    do
    {
        printf("\nMenu\n 1. Mostrar todos\n 2. Pesquisar\n 3. Incluir\n 4. Excluir\n 5. Sair\nInforme uma opcao: ");
        scanf("%d", &op);
        switch (op)
        {
        case 1:
            printf("\nAlunos gravados no arquivo: \n");
            mostra(arq);
            break;
        case 2:
            printf("\nDigite a matricula a ser buscada: ");
            scanf("%d", &matr);
            if (pesquisa(arq, lst, matr, &aluno))
            {
                printf("\nAluno encontrado:\n");
                printf("%d\t%s\t%d\t%d\n", aluno.matr, aluno.nome, aluno.nota1, aluno.nota2);
            }
            else
                printf("\nA matricula %d nao foi encontrada!\n", matr);
            break;
        case 3:
            inclui(arq, &lst);
            break;
        case 4:
            printf("\nDigite a matricula a ser excluida: ");
            scanf("%d", &matr);
            exclui(arq, &lst, matr);
            break;
        case 5:
            printf("\nSaindo...\n\n");
            break;
        default:
            printf("\nOpcao invalida!\n");
            break;
        }
    } while (op != 5);
    liberarLista(&lst);
    fclose(arq);
}