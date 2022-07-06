#ifndef PROBLEM_H
#define PROBLEM_H
#include <set>
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <climits>   
#include <sstream>
#include <fstream>
#include <string>
#include <math.h>
#include <algorithm>
#include <list>
#include <sys/timeb.h>
#include <exception>
#include <stack>
//#include <direct.h>
#include <vector>
#include <iterator>
#include <unordered_map>

using namespace std;

#define LITTLE_P

typedef struct {
	double *fixedcost;
	int *capacity;
	int *goods;
	double **supplycost;
	int *inc_s;
	int **inc;
	int **best_warehouse_S;
	vector<vector<int>> best_store_W;
	int num_S;
	int num_W;
	int num_Inc;
	int Tmax;
	bool reduced;
} problem_t;


problem_t *create_problem(int, int);
void free_problem(problem_t *);

#endif /* !PROBLEM_H */
