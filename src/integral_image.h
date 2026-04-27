/*
 * integral_image.h
 * Equipe: Leone Barbosa Hollanda, Joao Roberto Fernandes Magalhaes
 *
 * Modulo embarcavel - Integral Image (Summed Area Table)
 * Sem alocacao dinamica, sem recursividade.
 */

#ifndef INTEGRAL_IMAGE_H
#define INTEGRAL_IMAGE_H

#define H 64
#define W 64

/*
 * Constrói a Integral Image a partir de img[].
 * Cada posicao integral[r][c] recebe a soma de todos os
 * pixels de (0,0) ate (r,c).
 * Complexidade: O(H x W)
 */
void build_integral(int img[H][W], long integral[H][W]);

/*
 * Retorna a soma dos pixels dentro do retangulo (r1,c1)->(r2,c2)
 * usando apenas 4 acessos a matriz integral.
 * Complexidade: O(1)
 * Retorna 0 se as coordenadas forem invalidas.
 */
long sum_region(long integral[H][W], int r1, int c1, int r2, int c2);

#endif /* INTEGRAL_IMAGE_H */