/*
 * main.c
 * Equipe: Leone Barbosa Hollanda, Joao Roberto Fernandes Magalhaes
 *
 * Arquivo de teste/validacao - roda apenas no PC.
 * Nao faz parte do modulo embarcado.
 *
 * Compilar: gcc -Wall -o teste main.c integral_image.c
 * Usar:     ./teste
 */

#include <stdio.h>
#include "integral_image.h"

/* ------------------------------------------------------------------ */
/* Geracao de dados de teste                                           */
/* ------------------------------------------------------------------ */

static void fill_img(int img[H][W])
{
    int r, c;

    /*
     * Imagem sintetica usada nos testes.
     *
     * Cada pixel recebe um valor deterministico, calculado apenas a partir
     * da linha e da coluna. Assim, o C e o Python conseguem reconstruir a
     * mesma imagem sem depender de arquivos externos.
     */
    for (r = 0; r < H; r++)
        for (c = 0; c < W; c++)
            img[r][c] = (r + c) & 0xFF;
}

/* ------------------------------------------------------------------ */
/* Validacao por forca bruta                                           */
/* ------------------------------------------------------------------ */

static long naive_sum_region(int img[H][W], int r1, int c1, int r2, int c2)
{
    int r, c;
    long sum = 0L;

    /*
     * Implementacao de referencia por forca bruta.
     *
     * Ela percorre todos os pixels dentro do retangulo e soma um por um.
     * E usada apenas para validar o resultado de sum_region(), que deve
     * chegar ao mesmo valor em O(1).
     *
     * Complexidade: O(area da regiao)
     */
    if (r1 < 0 || c1 < 0 || r2 < 0 || c2 < 0)
        return 0L;
    if (r1 >= H || r2 >= H || c1 >= W || c2 >= W)
        return 0L;
    if (r1 > r2 || c1 > c2)
        return 0L;

    for (r = r1; r <= r2; r++)
        for (c = c1; c <= c2; c++)
            sum += (long)img[r][c];

    return sum;
}

/* ------------------------------------------------------------------ */
/* Gerador de numeros pseudo-aleatorios (LCG)                         */
/* ------------------------------------------------------------------ */

static unsigned long lcg_next(unsigned long *state)
{
    /*
     * Gerador linear congruente simples.
     *
     * Mantem os testes pseudo-aleatorios reprodutiveis: como a semente
     * inicial e fixa, a sequencia de consultas sera sempre a mesma.
     */
    *state = (*state * 1664525UL) + 1013904223UL;
    return *state;
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

typedef struct {
    int r1, c1, r2, c2;
} Query;

int main(void)
{
    static int  img[H][W];
    static long integral[H][W];

    int ok = 1;
    int t;
    unsigned i;
    unsigned long rng = 1UL;

    Query queries[] = {
        { 4,  1,  3,  0},
        { 0,  0,  1,  1},
        {10, 10, 20, 20},
        { 0,  0, H-1, W-1},
        { 5,  7,  5, 20}
    };

    /*
     * Fluxo principal do teste:
     * 1. cria a imagem sintetica;
     * 2. calcula a Integral Image usando o modulo em C;
     * 3. exporta a matriz completa para validacao externa em Python.
     */
    fill_img(img);
    build_integral(img, integral);

    /* ---------------- EXPORTA MATRIZ COMPLETA ---------------- */
    FILE *f_mat = fopen("integral_c.txt", "w");
    if (!f_mat) {
        printf("Erro ao abrir integral_c.txt\n");
        return 1;
    }

    for (int r = 0; r < H; r++)
    {
        for (int c = 0; c < W; c++)
        {
            fprintf(f_mat, "%ld ", integral[r][c]);
        }
        fprintf(f_mat, "\n");
    }
    fclose(f_mat);

    /* ---------------- EXPORTA QUERIES ---------------- */
    FILE *f_q = fopen("queries.txt", "w");
    if (!f_q) {
        printf("Erro ao abrir queries.txt\n");
        return 1;
    }

    printf("Integral Image (Summed Area Table) — %dx%d\n\n", H, W);

    printf("%-5s %-22s %12s %12s %6s\n",
           "Query", "Regiao (r1,c1)->(r2,c2)", "O(1)", "Forca-bruta", "OK?");
    printf("--------------------------------------------------------------\n");

    /*
     * Consultas fixas.
     *
     * Estas regioes cobrem casos simples, regiao invalida e imagem inteira.
     * Cada resultado calculado por sum_region() e comparado com a soma por
     * forca bruta para detectar erros de borda.
     */
    for (i = 0; i < sizeof(queries) / sizeof(queries[0]); i++)
    {
        int r1 = queries[i].r1, c1 = queries[i].c1;
        int r2 = queries[i].r2, c2 = queries[i].c2;

        long fast = sum_region(integral, r1, c1, r2, c2);
        long slow = naive_sum_region(img, r1, c1, r2, c2);

        fprintf(f_q, "%d %d %d %d %ld\n", r1, c1, r2, c2, fast);

        printf("Q%-4u (%2d,%2d) -> (%2d,%2d)  %12ld %12ld %6s\n",
               i + 1, r1, c1, r2, c2, fast, slow,
               (fast == slow) ? "OK" : "ERRO");
    }

    printf("\nStress test (200 consultas aleatorias)...\n");

    /*
     * Consultas pseudo-aleatorias.
     *
     * Gera retangulos validos em posicoes variadas da imagem. O objetivo e
     * aumentar a cobertura dos testes sem perder reprodutibilidade.
     */
    for (t = 0; t < 200; t++)
    {
        int r1, r2, c1, c2, tmp;
        long fast, slow;

        r1 = (int)(lcg_next(&rng) % (unsigned long)H);
        r2 = (int)(lcg_next(&rng) % (unsigned long)H);
        c1 = (int)(lcg_next(&rng) % (unsigned long)W);
        c2 = (int)(lcg_next(&rng) % (unsigned long)W);

        if (r1 > r2) { tmp = r1; r1 = r2; r2 = tmp; }
        if (c1 > c2) { tmp = c1; c1 = c2; c2 = tmp; }

        fast = sum_region(integral, r1, c1, r2, c2);
        slow = naive_sum_region(img, r1, c1, r2, c2);

        fprintf(f_q, "%d %d %d %d %ld\n", r1, c1, r2, c2, fast);

        if (fast != slow)
        {
            ok = 0;
            printf("FALHA: (%d,%d)->(%d,%d)  O(1)=%ld  naive=%ld\n",
                   r1, c1, r2, c2, fast, slow);
            break;
        }
    }

    fclose(f_q);

    printf("Validacao aleatoria: %s\n", ok ? "OK" : "ERRO");
    printf("Arquivos gerados: integral_c.txt e queries.txt\n");

    return ok ? 0 : 1;
}
