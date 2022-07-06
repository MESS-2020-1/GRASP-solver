#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "solution.h"

solution_t *create_solution(int num_S, int num_W)
{
	solution_t *solution = new solution_t;

	solution->solx = new int*[num_S];
	for (int i = 0; i < num_S; i++)
		solution->solx[i] = new int[num_W];

	solution->value = -1;
	solution->time = -1;
	return(solution);
}

csolution_t *create_csolution(int num_S, int num_W)
{
	csolution_t *csolution = new csolution_t;

	csolution->solx = new int*[num_S];
	for (int i = 0; i < num_S; i++)
		csolution->solx[i] = new int[num_W];

	csolution->value = INT_MAX;
	csolution->time = -1;
	csolution->soly = new int[num_W];
	csolution->disponible_W = new int[num_W];
	csolution->sin_servir_S = new int[num_S];
	csolution->num_sin_servir_S = num_S;

	return(csolution);
}

void free_solution(solution_t *solution)
{
	if (solution != NULL) {
		delete[] solution;
	}
}
void free_csolution(csolution_t *csolution)
{
	if (csolution != NULL) {
		delete[] csolution;
	}
}


