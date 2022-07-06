#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "problem.h"

problem_t *create_problem(int num_S, int num_W)
{
	problem_t *problem = new problem_t;
	problem->num_S = num_S;
	problem->num_W = num_W;

	problem->fixedcost = new double[num_W];
	problem->capacity = new int[num_W];
	problem->goods = new int[num_S];
	problem->inc_s = new int[num_S];

	problem->supplycost = new double*[num_S];
	problem->inc = new int*[num_S];

	problem->best_warehouse_S = new int*[num_S];
	for (int i = 0; i < num_S; i++)
	{
		problem->best_warehouse_S[i] = new int[5];
		problem->best_warehouse_S[i][0] = -1;
		problem->best_warehouse_S[i][1] = -1;
		problem->best_warehouse_S[i][2] = -1;
		problem->best_warehouse_S[i][3] = -1;
		problem->best_warehouse_S[i][4] = -1;
		
		problem->inc[i] = new int[num_S];
		problem->supplycost[i] = new double[num_W];
	}

	vector<int> temp;
	for (int i = 0; i < num_W; i++)
	{
		problem->best_store_W.push_back(temp);

	}

	return problem;
}


void free_problem(problem_t *problem)
{
	if (problem != NULL) {
		delete[] problem;
	}
}



//
