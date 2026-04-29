/*
 * integral_image.h
 * Equipe: Leone Barbosa Hollanda, Joao Roberto Fernandes Magalhaes
 *
 * Modulo embarcavel - Integral Image (Summed Area Table)
 * Sem alocacao dinamica, sem recursividade.
 */

#ifndef INTEGRAL_IMAGE_H
#define INTEGRAL_IMAGE_H

#include <stdint.h>

#define H 45
#define W 45

/*
 * Constrói a Integral Image a partir de img[].
 * Cada posicao integral[r][c] recebe a soma de todos os
 * pixels de (0,0) ate (r,c).
 * Complexidade: O(H x W)
 */
void build_integral(int img[H][W], int32_t integral[H][W]);

/*
 * Constrói a Integral Image sobrescrevendo a propria matriz.
 *
 * Entrada/saida:
 *   img[r][c] deve conter o valor do pixel na entrada e sera substituido por
 *   integral[r][c] (soma acumulada de (0,0) ate (r,c)).
 *
 * Observacao: o tipo da matriz deve suportar o maior valor acumulado para
 * evitar overflow.
 *
 * Complexidade: O(H x W)
 */
void build_integral_inplace(int32_t img[H][W]);

/*
 * Retorna a soma dos pixels dentro do retangulo (r1,c1)->(r2,c2)
 * usando apenas 4 acessos a matriz integral.
 * Complexidade: O(1)
 * Retorna 0 se as coordenadas forem invalidas.
 */
int32_t sum_region(int32_t integral[H][W], int r1, int c1, int r2, int c2);

#endif /* INTEGRAL_IMAGE_H */