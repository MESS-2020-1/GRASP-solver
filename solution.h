#ifndef SOLUTION_H
#define SOLUTION_H

#include "Data.h"

typedef struct {
	int **solx;
	double value;
	double time;
}solution_t;

typedef struct {
	int **solx; // Cantidad de Goods de cada S a cada W
	int *soly; // Bool: 1 Si el W está abierto
	int *disponible_W;//Capacidad disponibile de cada W
	int *sin_servir_S; // Para cada S, cantidad sin servir.
	int num_sin_servir_S;// Número de S que me quedan sin servir
	double value;
	double time;
}csolution_t;

solution_t *create_solution(int, int);
csolution_t *create_csolution(int, int);
void free_solution(solution_t *);
void free_csolution(csolution_t *);
#endif /* !SOLUTION_H */
