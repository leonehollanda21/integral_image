#include <stdio.h>

/*
Integral Image (Summed Area Table)
=================================

Ideia principal
--------------
Dada uma matriz 2D "img" (pense como uma imagem em tons de cinza), o algoritmo
faz um pré-processamento que cria uma segunda matriz 2D "integral" onde:

    integral[r][c] = soma de todos os valores de img nas linhas 0..r e colunas 0..c

Ou seja, cada posição guarda a soma acumulada do retângulo desde a origem (0,0)
até o ponto (r,c).

Por que isso é útil?
-------------------
Depois que "integral" está pronta, a soma de QUALQUER retângulo dentro da matriz
pode ser obtida em tempo constante O(1) com apenas 4 leituras em "integral".

Complexidade
------------
- Construção (pré-processamento): O(H × W) porque percorre toda a matriz.
- Consulta de retângulo: O(1) porque usa apenas 4 acessos e poucas operações.

Restrições / adequação para embarcados
-------------------------------------
- Tudo é iterativo (sem recursão).
- Sem alocação dinâmica (sem malloc/free).
- Matrizes estáticas (memória previsível).

Observação de memória
---------------------
Este exemplo usa int. O consumo real depende de sizeof(int) (muito comum ser 4
bytes). Com H=W=64 temos 4096 elementos por matriz; duas matrizes = 8192 ints.
Então o total em bytes é 8192 * sizeof(int). (O requisito do trabalho menciona
"~8KB"; se precisar ajustar isso, a ideia é usar um tipo menor, ex.: int16_t,
desde que não haja overflow.)
*/

#define H 64
#define W 64

static void fill_img(int img[H][W])
{
    int r, c;

    /*
     * Preenche "img" com um padrão determinístico só para termos dados.
     * Em um uso real, "img" viria de um sensor, arquivo, frame de câmera etc.
     */
    for (r = 0; r < H; r++)
    {
        for (c = 0; c < W; c++)
        {
            img[r][c] = (r + c) & 0xFF;
        }
    }
}


static void build_integral(int img[H][W], long integral[H][W])
{
    int r, c;
    long row_running_sum;

    for (r = 0; r < H; r++)
    {
        row_running_sum = 0;

        for (c = 0; c < W; c++)
        {
            row_running_sum += (long)img[r][c];

            integral[r][c] = row_running_sum +
                             ((r > 0) ? integral[r - 1][c] : 0L);
        }
    }
}


static long sum_region(long integral[H][W], int r1, int c1, int r2, int c2)
{
    long A, B, C, D;

    if (r1 < 0 || c1 < 0 || r2 < 0 || c2 < 0)
        return 0L;

    if (r1 >= H || r2 >= H || c1 >= W || c2 >= W)
        return 0L;

    if (r1 > r2 || c1 > c2)
        return 0L;

    A = integral[r2][c2];
    B = (r1 > 0) ? integral[r1 - 1][c2] : 0L;
    C = (c1 > 0) ? integral[r2][c1 - 1] : 0L;
    D = (r1 > 0 && c1 > 0) ? integral[r1 - 1][c1 - 1] : 0L;

    return A - B - C + D;
}

static long naive_sum_region(int img[H][W], int r1, int c1, int r2, int c2)
{
    int r, c;
    long sum;

    sum = 0L;

    for (r = r1; r <= r2; r++)
    {
        for (c = c1; c <= c2; c++)
        {
            sum += (long)img[r][c];
        }
    }

    return sum;
}

static unsigned long lcg_next(unsigned long *state)
{
    *state = (*state * 1664525UL) + 1013904223UL;
    return *state;
}

int main(void)
{
    /* static para evitar estouro de pilha (stack) em sistemas pequenos */
    static int img[H][W];
    static long integral[H][W];

    int ok;
    int t;
    unsigned i;
    unsigned long rng;

    struct Query
    {
        int r1, c1, r2, c2;
    } queries[] = {
        {4, 1, 3, 0},
        {0, 0, 1, 1},
        {10, 10, 20, 20},
        {0, 0, H - 1, W - 1},
        {5, 7, 5, 20}
    };

    ok = 1;
    rng = 1UL;

    fill_img(img);
    build_integral(img, integral);

    printf("Integral Image (Summed Area Table) - %dx%d\n", H, W);

    for (i = 0; i < (unsigned)(sizeof(queries) / sizeof(queries[0])); i++)
    {
        int r1, c1, r2, c2;
        long fast, slow;

        r1 = queries[i].r1;
        c1 = queries[i].c1;
        r2 = queries[i].r2;
        c2 = queries[i].c2;

        fast = sum_region(integral, r1, c1, r2, c2);
        slow = naive_sum_region(img, r1, c1, r2, c2);

        /* %ld para imprimir o tipo long */
        printf("Q%u: (%d,%d) -> (%d,%d)  O(1)=%ld  naive=%ld  %s\n",
               i + 1, r1, c1, r2, c2, fast, slow,
               (fast == slow) ? "OK" : "ERRO");
    }

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

        if (fast != slow)
        {
            ok = 0;
            printf("Falha: (%d,%d)->(%d,%d) O(1)=%ld naive=%ld\n",
                   r1, c1, r2, c2, fast, slow);
            break;
        }
    }

    printf("Validacao aleatoria: %s\n", ok ? "OK" : "ERRO");

    return ok ? 0 : 1;
}