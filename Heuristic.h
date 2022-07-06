#pragma once
#ifndef HEURISTIC_H_
#define HEURISTIC_H_
#include "Data.h"
#include "problem.h"
#include "solution.h"

class Heuristic
{
private:
	bool lpshort;
public:
	Heuristic(problem_t *);
	~Heuristic(void);

	list<int> openW;
	bool improve;
	problem_t *problem;
	csolution_t *current_s; //Mejor solución actual
	csolution_t *iter_s; //Mejor solución actual
	vector<int> ord_W;
	vector<int> ord_S;
	int *frec_W_best;
	int **best_warehouse_S;
	int top;

	void initialize();
	void order_W_vector(); //Los warehouses por frecuencia en el top5


	void solve_heuristic(struct timeb t_ini, struct timeb t1, solution_t *solution);

	void constructive_phase_w(double min_perc);

	void serve_warehouses_better_w(int store, int warehouse);
	void serve_w(int store, int warehouse);
	void service(int store, int warehouse, int asign);
	void save_sol(solution_t* solution, struct timeb);
	void constructive(struct timeb t_ini, struct timeb t1, solution_t *solution,int iter);
	void VND(struct timeb t_ini, struct timeb t1, solution_t *solution);
	void local_search_perc_s(struct timeb t_ini, struct timeb t1, solution_t *solution);
	void local_search_perc_w(struct timeb t_ini, struct timeb t1, solution_t *solution);
	void local_search_all_w(struct timeb t_ini, struct timeb t1, solution_t *solution);
	void ls_allocate_s(double);
	void allocate_s();
	double compute_cost(int s, int w);
	void ls_remove_all_w(int n_del, vector<int> &lista);
	void ls_remove_perc_w(int n_del, vector<int> &lista);
	void ls_remove_perc_s(int n);
	void delete_sw(int s, int w);
	void check_solution();
	bool buscar_compatibilidad(int s, int w);

	bool compatibilidad(int store, vector<int> sol);
	void clear_current_sol();
	void insertordered(int s, int w, list<int> &lista, int limit_size);
	void insertordered2(int s, int w, list<int> &lista);
	double cost_per_unit(int store, int warehouse);

	vector<int> complete_w(int warehouse,int addstore, double min_perc);
	void serve_s_complete_w(double min_per);
};
#endif