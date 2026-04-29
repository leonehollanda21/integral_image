/*
 * integral_image.c
 * Equipe: Leone Barbosa Hollanda, Joao Roberto Fernandes Magalhaes
 *
 * Modulo embarcavel - Integral Image (Summed Area Table)
 * Sem alocacao dinamica, sem recursividade.
 */

#include "integral_image.h"

/*
 * Constrói a Integral Image a partir de img[].
 *
 * Formula de recorrencia:
 *   integral[r][c] = img[r][c]
 *                  + integral[r-1][c]    (soma acima)
 *                  + integral[r][c-1]    (soma a esquerda)
 *                  - integral[r-1][c-1]  (canto subtraido duas vezes)
 *
 * Complexidade: O(H x W)
 */
void build_integral(int img[H][W], int32_t integral[H][W])
{
    int r, c;
    int64_t row_running_sum;

    for (r = 0; r < H; r++)
    {
        row_running_sum = 0;

        for (c = 0; c < W; c++)
        {
            row_running_sum += (int64_t)img[r][c];

            integral[r][c] = (int32_t)(row_running_sum +
                                       ((r > 0) ? (int64_t)integral[r - 1][c] : 0));
        }
    }
}

/*
 * Versao in-place: sobrescreve img[][] com a Integral Image.
 *
 * Recorrencia equivalente (com soma acumulada por linha):
 *   row_running_sum += img[r][c]         (img[r][c] ainda e o pixel original)
 *   img[r][c] = row_running_sum + img[r-1][c]
 */
void build_integral_inplace(int32_t img[H][W])
{
    int r, c;
    int64_t row_running_sum;

    for (r = 0; r < H; r++)
    {
        row_running_sum = 0;

        for (c = 0; c < W; c++)
        {
            row_running_sum += (int64_t)img[r][c];
            img[r][c] = (int32_t)(row_running_sum + ((r > 0) ? (int64_t)img[r - 1][c] : 0));
        }
    }
}

/*
 * Retorna a soma dos pixels dentro do retangulo (r1,c1)->(r2,c2).
 *
 * Identidade usada:
 *   soma = A - B - C + D
 * onde:
 *   A = integral[r2][c2]         (area total ate o canto inferior direito)
 *   B = integral[r1-1][c2]       (area acima da regiao)
 *   C = integral[r2][c1-1]       (area a esquerda da regiao)
 *   D = integral[r1-1][c1-1]     (canto superior esquerdo, subtraido duas vezes)
 *
 * Complexidade: O(1)
 */
int32_t sum_region(int32_t integral[H][W], int r1, int c1, int r2, int c2)
{
    int64_t A, B, C, D;

    if (r1 < 0 || c1 < 0 || r2 < 0 || c2 < 0)
        return 0;

    if (r1 >= H || r2 >= H || c1 >= W || c2 >= W)
        return 0;

    if (r1 > r2 || c1 > c2)
        return 0;

    A = (int64_t)integral[r2][c2];
    B = (r1 > 0) ? (int64_t)integral[r1 - 1][c2] : 0;
    C = (c1 > 0) ? (int64_t)integral[r2][c1 - 1] : 0;
    D = (r1 > 0 && c1 > 0) ? (int64_t)integral[r1 - 1][c1 - 1] : 0;

    return (int32_t)(A - B - C + D);
}