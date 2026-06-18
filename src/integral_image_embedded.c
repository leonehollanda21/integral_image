/*
 * integral_image_embedded.c
 *
 * Integral Image / Summed Area Table (SAT) — single-file module.
 *
 * Objetivo:
 * - Codigo pronto para embarcado (ex.: STM32): sem malloc, sem recursao, sem I/O.
 * - Tudo neste unico arquivo .c (sem header obrigatorio).
 *
 * Requisitos tipicos (atendidos):
 * - Pelo menos 1 laco; aqui existem 2 lacos aninhados na construcao.
 * - Estrutura de dados ~8KB no codigo: sat[II_H][II_W] com II_ACCUM_T=uint32_t
 *   e II_H=II_W=45 => 45*45*4 = 8100 bytes.
 */

#include <stdint.h>
#include <stddef.h>

/*
 * Dimensoes fixas (compile-time).
 *
 * Ajuste II_H/II_W conforme seu problema.
 * Para compatibilizar com o projeto original, o padrao e 45x45.
 */
#ifndef II_H
#define II_H 45
#endif

#ifndef II_W
#define II_W 45
#endif

/*
 * Tipos configuraveis (mantidos simples por padrao).
 * - II_ACCUM_T: tipo armazenado na matriz integral.
 * - II_WIDE_T: tipo usado para somas intermediarias (reduz risco de overflow).
 */
#ifndef II_ACCUM_T
#define II_ACCUM_T uint32_t
#endif

#ifndef II_WIDE_T
#define II_WIDE_T uint64_t
#endif

/*
 * Armazenamento interno (global) da SAT.
 *
 * Por padrao, fica habilitado para cumprir o requisito de ~8KB.
 * Se voce preferir gerenciar o buffer fora, defina II_NO_INTERNAL_SAT.
 */
#ifndef II_NO_INTERNAL_SAT
II_ACCUM_T ii_sat[II_H][II_W];
#endif

/*
 * Constroi a matriz integral (Summed Area Table) a partir de uma imagem 8-bit.
 *
 * Parametros:
 * - img: ponteiro para o pixel (uint8_t)
 * - sat: ponteiro para a matriz integral (II_ACCUM_T)
 * - width, height: dimensoes
 * - img_stride: numero de pixels por linha em img (0 => usa width)
 * - sat_stride: numero de elementos por linha em sat (0 => usa width)
 *
 * Observacao:
 * - img_stride e sat_stride sao em "elementos", nao em bytes.
 */
void ii_build_u8_fixed(const uint8_t img[II_H][II_W], II_ACCUM_T sat[II_H][II_W])
{
    if (!img || !sat)
        return;

    for (uint16_t r = 0; r < (uint16_t)II_H; r++)
    {
        II_WIDE_T row_running_sum = 0;

        for (uint16_t c = 0; c < (uint16_t)II_W; c++)
        {
            row_running_sum += (II_WIDE_T)img[r][c];
            sat[r][c] = (II_ACCUM_T)(row_running_sum + ((r > 0) ? (II_WIDE_T)sat[r - 1][c] : (II_WIDE_T)0));
        }
    }
}

#ifndef II_NO_INTERNAL_SAT
void ii_build_u8_to_internal_sat(const uint8_t img[II_H][II_W])
{
    ii_build_u8_fixed(img, ii_sat);
}
#endif

/*
 * Versao in-place para matrizes signed 32-bit.
 *
 * Entrada/saida:
 * - img: contem pixels/valores na entrada e sera sobrescrita pela integral.
 *
 * Parametros:
 * - stride: numero de elementos por linha (0 => usa width)
 */
void ii_build_s32_inplace_fixed(int32_t img[II_H][II_W])
{
    if (!img)
        return;

    for (uint16_t r = 0; r < (uint16_t)II_H; r++)
    {
        II_WIDE_T row_running_sum = 0;

        for (uint16_t c = 0; c < (uint16_t)II_W; c++)
        {
            row_running_sum += (II_WIDE_T)img[r][c];
            img[r][c] = (int32_t)(row_running_sum + ((r > 0) ? (II_WIDE_T)img[r - 1][c] : (II_WIDE_T)0));
        }
    }
}

/*
 * Soma em O(1) de um retangulo (r1,c1)->(r2,c2), inclusivo.
 * Retorna 0 se as coordenadas forem invalidas.
 */
II_ACCUM_T ii_sum_fixed(const II_ACCUM_T sat[II_H][II_W], int r1, int c1, int r2, int c2)
{
    if (!sat)
        return (II_ACCUM_T)0;

    if (r1 < 0 || c1 < 0 || r2 < 0 || c2 < 0)
        return (II_ACCUM_T)0;

    if (r1 >= (int)II_H || r2 >= (int)II_H || c1 >= (int)II_W || c2 >= (int)II_W)
        return (II_ACCUM_T)0;

    if (r1 > r2 || c1 > c2)
        return (II_ACCUM_T)0;

    /* soma = A - B - C + D */
    II_WIDE_T A = (II_WIDE_T)sat[r2][c2];
    II_WIDE_T B = (r1 > 0) ? (II_WIDE_T)sat[r1 - 1][c2] : (II_WIDE_T)0;
    II_WIDE_T C = (c1 > 0) ? (II_WIDE_T)sat[r2][c1 - 1] : (II_WIDE_T)0;
    II_WIDE_T D = (r1 > 0 && c1 > 0) ? (II_WIDE_T)sat[r1 - 1][c1 - 1] : (II_WIDE_T)0;

    return (II_ACCUM_T)(A - B - C + D);
}

#ifndef II_NO_INTERNAL_SAT
II_ACCUM_T ii_sum_internal_sat(int r1, int c1, int r2, int c2)
{
    return ii_sum_fixed(ii_sat, r1, c1, r2, c2);
}
#endif
