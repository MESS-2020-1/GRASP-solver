#include "problem.h"
#include "solution.h"
#include "Data.h"
#include "Heuristic.h"

problem_t *readinstance(string fichero);
#ifdef PRINT2
void print_sol2(solution_t *solution, string name_sol, int, int);
#else
void print_sol(solution_t *solution, string name_sol,int, int);
#endif
void copy_sol(csolution_t *current_s, csolution_t *iter_s, problem_t *problem);
int main(int argc, char **argv)
{
	problem_t *problem;
	solution_t *solution;

	string name_inst = argv[1];
	string name_sol = argv[2];

	name_sol = name_sol;
	string name = name_inst;
	problem = readinstance(name);
	problem->Tmax = atoi(argv[3]);

	solution = create_solution(problem->num_S, problem->num_W);
	int timeinv;
	int seed = atoi(argv[4]);
	srand(seed);

	///Estructuras para los tiempos
	struct timeb t_ini, t1;
	double ttime=0;

	ftime(&t_ini);
	ftime(&t1);

	Heuristic heur(problem);
	problem->reduced = false;
	heur.solve_heuristic(t_ini, t1, solution);

	ftime(&t1);
	ttime = ((double)((t1.time - t_ini.time) * 1000 + t1.millitm - t_ini.millitm)) / 1000;

	if (solution->value != -1)
	{
		FILE *sol_file;
		//mkdir("Results");
		sol_file = fopen("results.txt", "a+");
#ifdef PRINT_S
		fprintf(stdout, "Instance:\t%s\tBestSol:\t%.2f\tTtime:\t%.2f\n", name_inst.c_str(), solution->value, ttime);
#endif
		fprintf(sol_file, "Instance:\t%s\tBestSol:\t%.2f\tTtime:\t%.2f\n", name_inst.c_str(),  solution->value, ttime);
		fclose(sol_file);
#ifdef PRINT2
		print_sol2(solution, name_sol, problem->num_S, problem->num_W);
#else
		print_sol(solution, name_sol, problem->num_S, problem->num_W);
#endif
	}
	//free_problem(problem);
	free_solution(solution);
}




problem_t *readinstance(string fichero)
{
	int goods;
	int goods_c;
	double ccost;
	double cost;
	int warehouse;
	int store;
	problem_t *problem;
	int W, S, nInc;
	ifstream in(fichero);
	if (!in)
	{
		fprintf(stdout, "Error Instance Reading");
	}
	else
	{
		char *dummy = new char[200];
		char d;
		//int num_read;

		in.getline(dummy, 200, '=');
		in >> W;
		in.getline(dummy, 200, '=');
		in >> S;

		/* Initialize pointers*/
		problem = create_problem(S, W);

		/* Initialize pointers - end */

		in.getline(dummy, 200, '[');
		for (int j = 0; j < W; j++)
		{
			in >> problem->capacity[j] >> d;
		}

		in.getline(dummy, 200, '[');
		for (int j = 0; j < W; j++)
		{
			in >> problem->fixedcost[j] >> d;
		}

		in.getline(dummy, 200, '[');
		for (int i = 0; i < S; i++)
		{
			in >> problem->goods[i] >> d;
		}
		in.getline(dummy, 200, '[');
		in.getline(dummy, 200, '|');

		//Leemos el coste de servicio de s a w y actualizamos las matrices a cero

		double ccopen;
		double copen;

		for (int i = 0; i < S; i++)
		{
			problem->inc_s[i] = 0;

			for (int ii = 0; ii < S; ii++)
			{
				problem->inc[i][ii] = 0;
			}
			//in.getline(dummy, 200, '|');
			for (int j = 0; j < W; j++)
			{
				in >> problem->supplycost[i][j] >> d;
				ccost = problem->supplycost[i][j];
				ccopen = problem->fixedcost[j];
				//Miro el bector de mejores warehouses para i

				for (int k = 0; k < 5; k++)
				{
					if (problem->best_warehouse_S[i][k] == -1)
					{
						problem->best_warehouse_S[i][k] = j;
						k = 5;
					}
					else
					{
						warehouse = problem->best_warehouse_S[i][k];
						cost = problem->supplycost[i][warehouse];
						copen = problem->fixedcost[warehouse];
						if (ccost < cost || (cost == ccost && ccopen < copen))
						{
							for (int kk = 4; kk >= k + 1; kk--)
							{
								problem->best_warehouse_S[i][kk] = problem->best_warehouse_S[i][kk-1];
							}
							problem->best_warehouse_S[i][k] = j;
							k = 5;
						}
					}
				}

			}
		}

		in.getline(dummy, 200, '=');
		in >> problem->num_Inc;

		in.getline(dummy, 200, '[');
		int ifirst, isecond;
		for (int i = 0; i < problem->num_Inc; i++)
		{
			in.getline(dummy, 200, '|');
			in >> ifirst >> d >> isecond;

			if (problem->inc[ifirst - 1][isecond - 1] == 0)
			{
				problem->inc_s[ifirst - 1]++; //Aquí me guardo el número de incompatibles que tiene un stack
				problem->inc_s[isecond - 1]++; //Aquí me guardo el número de incompatibles que tiene un stack
				problem->inc[ifirst - 1][isecond - 1] = 1;
				problem->inc[isecond - 1][ifirst - 1] = 1;
			}
		}

		in.close();
		delete[] dummy;

		double dif_warehouse;
		double dif_warehouse_c;

		int j;

		//Ordenar los stores por número de incompatibilidades
		double ccost;
		double cost;
		double cdif;
		double dif;
		int store;		
		bool insert_flag;

		for (int s = 0; s < S; s++)
		{
			j = problem->best_warehouse_S[s][0]; //Warehouse de mejor servicio para ese store

			if (problem->best_store_W[j].size() == 0)
			{
				problem->best_store_W[j].push_back(s);
			}
			else
			{
				insert_flag = false;
				ccost = problem->supplycost[s][j];
				cdif = (problem->supplycost[s][problem->best_warehouse_S[s][1]]) - ccost;
				goods_c = problem->goods[s];

				for (auto it = problem->best_store_W[j].begin(); it != problem->best_store_W[j].end() ; it++)
				{
					store= (*it);
					cost = problem->supplycost[store][j];
					dif = (problem->supplycost[store][problem->best_warehouse_S[store][1]]) - cost;
					goods= problem->goods[store];
					if (cdif > dif
						|| (cdif == dif && ((problem->supplycost[s][problem->best_warehouse_S[s][2]] - ccost) > (problem->supplycost[store][problem->best_warehouse_S[store][2]] - cost)))
						|| (cdif == dif && ((problem->supplycost[s][problem->best_warehouse_S[s][2]] - ccost) == (problem->supplycost[store][problem->best_warehouse_S[store][2]] - cost)) && ((problem->supplycost[s][problem->best_warehouse_S[s][3]] - ccost) > (problem->supplycost[store][problem->best_warehouse_S[store][3]] - cost)))
						|| (cdif == dif && ((problem->supplycost[s][problem->best_warehouse_S[s][2]] - ccost) == (problem->supplycost[store][problem->best_warehouse_S[store][2]] - cost)) && ((problem->supplycost[s][problem->best_warehouse_S[s][3]] - ccost) == (problem->supplycost[store][problem->best_warehouse_S[store][3]] - cost)) && ((problem->supplycost[s][problem->best_warehouse_S[s][4]] - ccost) > (problem->supplycost[store][problem->best_warehouse_S[store][4]] - cost)))
						|| (cdif == dif && ((problem->supplycost[s][problem->best_warehouse_S[s][2]] - ccost) == (problem->supplycost[store][problem->best_warehouse_S[store][2]] - cost)) && ((problem->supplycost[s][problem->best_warehouse_S[s][3]] - ccost) == (problem->supplycost[store][problem->best_warehouse_S[store][3]] - cost)) && ((problem->supplycost[s][problem->best_warehouse_S[s][4]] - ccost) == (problem->supplycost[store][problem->best_warehouse_S[store][4]] - cost)) && problem->goods[s] >= problem->goods[store]))
					{
						problem->best_store_W[j].insert(it, s);
						insert_flag = true;
						break;
					}
				}
				if(insert_flag==false)
					problem->best_store_W[j].push_back(s);

			}
		}
	}

	return(problem);
}



#ifdef PRINT2
void print_sol2(solution_t *solution, string name_sol, int S, int W)
{
	bool first = true;
	ofstream out(name_sol.c_str());
	out << "{";
	for (int s = 0; s < S; s++)
	{
		for (int w = 0; w < W; w++)
		{
			if (solution->solx[s][w]>0)
			{
				if (first)
				{
					out << "(";
					first = false;
				}
				else
				{
					out << ", (";
				}
				out << s+1 << "," << w+1 << "," << solution->solx[s][w];
				out << ")";
			}
		}
	}
	out << "}" << endl;
	out << "time to best: "<<solution->time;

	out.close();

}
#else
void print_sol(solution_t *solution, string name_sol, int S, int W)
{
	ofstream out(name_sol.c_str());
	out << "[";
	for (int s = 0; s < S - 1; s++)
	{
		out << "(";
		for (int w = 0; w < W - 1; w++)
		{
			out << solution->solx[s][w] << ",";
		}
		out << solution->solx[s][W - 1] << ")" << endl;
	}
	// El último
	out << "(";
	for (int w = 0; w < W - 1; w++)
	{
		out << solution->solx[S - 1][w] << ",";
	}
	out << solution->solx[S - 1][W - 1] << ")]";
	out.close();

}
#endif