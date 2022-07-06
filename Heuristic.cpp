#include "Heuristic.h"

Heuristic::Heuristic(problem_t *problem)
{
	this->problem = problem;
	top = 5;
}

Heuristic::~Heuristic(void)
{
}


void Heuristic::initialize()
{
	/* Inicializar variables y vectores distancias*/
	current_s = create_csolution(problem->num_S, problem->num_W);
	iter_s = create_csolution(problem->num_S, problem->num_W);

	frec_W_best = new int[problem->num_W];
	for (int s = 0; s < problem->num_S; s++)
	{
		ord_S.push_back(s);
	}
	for (int w = 0; w < problem->num_W; w++)
	{
		ord_W.push_back(-1);
		frec_W_best[w] = 0;
	}

	//Ordenamos los W por "moda", es decir primero los que son mejor (en el top 5) para un número mayor de tiendas.
	//Si hay empate, por coste de apertura
	order_W_vector();

}

void Heuristic::solve_heuristic(struct timeb t_ini, struct timeb t1, solution_t *solution)
{
	ftime(&t1);
	double time = ((double)((t1.time - t_ini.time) * 1000 + t1.millitm - t_ini.millitm)) / 1000;

	//Inicializamos parámetros
	initialize(); 
	double best_improvement = 0;
	double sol_prev;
	double sol_post;
	int it = -1;
	
	while (time < problem->Tmax)
	{
		it++;
		constructive(t_ini, t1, solution,10);
#ifdef PRINT_S
			fprintf(stdout, "CPhase:\t%.2f\t%d\n", current_s->value, it);
#endif


		if (solution->value == -1 || (solution->value > current_s->value && current_s->num_sin_servir_S == 0))
			save_sol(solution, t_ini);
		

		sol_prev = current_s->value;

		if ((double(current_s->value) - 1.05*double(best_improvement)) <= solution->value)
		{
			//VND(t_ini, t1, solution);
			improve = true;
			while (improve == true)
			{
				improve = false;
				local_search_all_w(t_ini, t1, solution);
				local_search_perc_s(t_ini, t1, solution);
				if (improve == false)
					local_search_perc_w(t_ini, t1, solution);
					
			}
		}

		sol_post = current_s->value;

		if (sol_prev - sol_post > best_improvement) 
			best_improvement = sol_prev - sol_post;

		ftime(&t1);
		time = ((double)((t1.time - t_ini.time) * 1000 + t1.millitm - t_ini.millitm)) / 1000;
	}

}

void Heuristic::serve_s_complete_w(double min_perc)
{

	int warehouse;
	int store;
	int substore;
	vector<int> sol;

	//Recorro los stores y abro el mejor warehouse que lo puede servir y ese warehouse intento completarlo con sus mejores
	for (int ss = 0; ss < problem->num_S; ss++)
	{
		store = ord_S[ss];
		if (current_s->sin_servir_S[store] > 0)
		{
			for (int k = 0; k < top && current_s->sin_servir_S[store] > 0; k++)
			{
				warehouse = problem->best_warehouse_S[store][k]; //Supuestamente el mejor warehouse para ese store
				if (current_s->disponible_W[warehouse] > 0 && buscar_compatibilidad(store, warehouse))
				{
					sol = complete_w(warehouse, store, min_perc); //Si sol sale con tamaño es porque hay una solución que lo completa al 99%
					if (sol.size() > 0)
					{
						for (int s = 0; s < sol.size() && current_s->disponible_W[warehouse] > 0; s++) //Recorremos los stores desde menor a mayor coste 
						{
							substore = sol[s];
							serve_w(substore, warehouse);
						}
					}
				}
			}
		}
	}
}

void Heuristic::constructive_phase_w(double min_perc)
{
	//Se recorre el vector de best warehouse. Se intenta abrir y asignar primero los mejores stores para ese warehouse. Si hay otro sitio mejor donde podrían ir esos stores y, el coste de abrir el otro sitio y enviarlos, no es mayor que enviarlo al actual, se hace.

	int warehouse, store;
	int w, s;


	//Recorro los stores de mayor a menor coste y abro el mejor warehouse que lo puede servir
	random_shuffle(ord_S.begin(), ord_S.end());
	serve_s_complete_w(min_perc);

	for (w = 0; w < problem->num_W && current_s->num_sin_servir_S>0; w++) // Solo mientras nos queden Stores sin servir.
	{
		warehouse = ord_W[w]; //warehouse que abrir

		if (current_s->disponible_W[warehouse] > 0) //hay capacidad en el warehouse
		{
			//Primero recorremos los mejores stores para ese warehouse
			for (int k = 0; k < problem->best_store_W[warehouse].size() && current_s->disponible_W[warehouse] > 0; k++) //Recorremos los stores desde menor a mayor coste 
			{
				store = problem->best_store_W[warehouse][k];
				if (current_s->sin_servir_S[store] > 0) //Si está sin servir, miramos si ese store tiene otro mejor (si lo tiene y es más barato servirlo ahí, se sirve)
				{
					serve_warehouses_better_w(store, warehouse); //Miramos si puede servirse la tienda con sus almacenes tops o, si no, con otros abiertos si tiene coste menor que enviarlo al warehouse
					serve_w(store, warehouse); //Si queda algo sin servir, se sirve con el almacén w
				}
			}
		}
	}

	random_shuffle(ord_S.begin(), ord_S.end());
	allocate_s();

	return;
}

double Heuristic::cost_per_unit(int store, int warehouse)
{
	return (((double(problem->fixedcost[warehouse]) / double(problem->capacity[store])) + problem->supplycost[store][warehouse])*double(min(problem->capacity[warehouse], problem->goods[store])));
}

vector<int> Heuristic::complete_w(int warehouse, int addstore,double min_perc)
{
	vector<int> sol;
	bool presence;
	int capacity = current_s->disponible_W[warehouse];
	int store = 0;

	if(addstore!=-1)
		sol.push_back(addstore);


	for (int k = 0; k < problem->best_store_W[warehouse].size() && capacity > 0; k++) //Recorremos los stores desde menor a mayor coste 
	{
		store = problem->best_store_W[warehouse][k];
		if (current_s->sin_servir_S[store] > 0 && compatibilidad(store, sol)) //Hay store
		{
			capacity = capacity - current_s->sin_servir_S[store];
			sol.push_back(store);
		}
	}

	double perc = 100 * (double(problem->capacity[warehouse] - capacity) / double(problem->capacity[warehouse]));
	if (perc < min_perc)
	{
		sol.clear();
	}
	return sol;
}

void Heuristic::serve_w(int store, int warehouse)
{
	if (current_s->sin_servir_S[store] > 0 && current_s->disponible_W[warehouse] > 0 && buscar_compatibilidad(store, warehouse))
	{
		service(store, warehouse, 0); //Cuando aquí pongo 0, se calcula dentro
		if (current_s->soly[warehouse] == false)
		{
			current_s->soly[warehouse] = true;
			current_s->value += problem->fixedcost[warehouse];
			if(current_s->disponible_W[warehouse]>0)
				openW.push_back(warehouse);
		}
	}
}

void Heuristic::serve_warehouses_better_w(int store, int warehouse)
{
	int warehouse_s;
	int assign;
	for (int kk = 0; kk < top; kk++)
	{
		if (current_s->sin_servir_S[store] > 0) //¿Hay bienes sin servir de ese store?
		{
			warehouse_s = problem->best_warehouse_S[store][kk]; //El mejor desde el que podría servirse el store

			if (current_s->disponible_W[warehouse_s] > 0) 
			{
				if (current_s->soly[warehouse_s]) //Si ese warehouse está abierto y es compatible, se sirve ahí.
				{
					if (buscar_compatibilidad(store, warehouse_s))
					{
						service(store, warehouse_s, 0); //Cuando aquí pongo 0, se calcula dentro
					}
				}
			}

		}
		else
			return;
	}
}

void Heuristic::clear_current_sol()
{
	// Inicializamos la solución

	openW.clear();
	current_s->value = 0;
	current_s->num_sin_servir_S = problem->num_S;
	for (int w = 0; w < problem->num_W; ++w)
	{
		current_s->soly[w] = false;
		for (int s = 0; s < problem->num_S; s++)
		{
			current_s->solx[s][w] = 0;
		}
	}
	for (int w = 0; w < problem->num_W; ++w) current_s->disponible_W[w] = problem->capacity[w];
	for (int s = 0; s < problem->num_S; s++) current_s->sin_servir_S[s] = problem->goods[s];

}

void Heuristic::save_sol(solution_t* solution, struct timeb t_ini)
{
	struct timeb t1;
	ftime(&t1);
	double time = ((double)((t1.time - t_ini.time) * 1000 + t1.millitm - t_ini.millitm)) / 1000;

	solution->value = current_s->value;
	for (int s = 0; s < problem->num_S; s++)
	{
		for (int w = 0; w < problem->num_W; w++)
		{
			solution->solx[s][w] = current_s->solx[s][w];
		}
	}
	solution->time = time;
}

double Heuristic::compute_cost(int s, int w)
{
	double cost=0;
	double assign = min(problem->capacity[w], current_s->sin_servir_S[s]);
	if (current_s->soly[w] == 0)
	{
		cost += (problem->fixedcost[w]/assign);
	}
	cost += problem->supplycost[s][w];
	return cost;
}

void Heuristic::ls_allocate_s(double value_it)
{
	list<int> best;
	double cost_w;

	int warehouse;
	int store;
	int store_w; //best store for warehouse s
	int warehouse_s; //best warehouse for store s

	bool flag;
	double cant1;
	double cant2;
	int asign;
	int w, s;
	bool repeat;
	vector<pair<int,double>> best_w(5);
	random_shuffle(ord_S.begin(), ord_S.end());

	for (s = 0; s < problem->num_S && current_s->num_sin_servir_S>0; s++) // Solo mientras nos queden Stores sin servir.
	{
		store = ord_S[s];

		if (current_s->sin_servir_S[store] > 0) //me quedan cosas que mandar a ese store
		{
			repeat = true;
			while (repeat)
			{
				repeat = false;
				best.clear();
				for (int w = 0; w < problem->num_W; w++)
				{
					warehouse = ord_W[w];
					if (current_s->disponible_W[warehouse] > 0 && buscar_compatibilidad(store, warehouse))
						insertordered(store, warehouse, best, 5);
				}

				if (best.size() > 0)
					repeat = true;

				for (auto it = best.begin(); it != best.end() && current_s->sin_servir_S[store] > 0; it++)
				{
					serve_w(store, (*it)); //Asigno el store al warehouse
					if (current_s->value >= value_it)
						return;
				}
				if (current_s->sin_servir_S[store] == 0)
				{
					repeat = false;
				}
			}
		}
	}

}

void Heuristic::insertordered(int s, int w, list<int> &lista, int limit_size)
{
	//Primero los abiertos o los cerrados pero que abriéndolos satisfagan la demanda.

	if (lista.size() == 0)
	{
		lista.push_back(w);
		return;
	}
	int wh;

	int indexh;
	int index;

	double costh;
	index = 1;

	if (current_s->soly[w] == 0 && (problem->capacity[w] < current_s->sin_servir_S[s]))
	{
		index = 0;
	}
	double cost = compute_cost(s, w);

	for (auto it = lista.begin(); it != lista.end(); it++)
	{
		wh = (*it);

		indexh = 1;
		if (current_s->soly[wh] == 0 && (problem->capacity[wh] < current_s->sin_servir_S[s]))
		{
			indexh = 0;
		}
		costh = compute_cost(s, wh);


		if (index > indexh || (index == indexh && cost <= costh))
		{
			lista.insert(it, w);
			if (limit_size != -1 && lista.size() > limit_size)
			{
				lista.pop_back();
			}
			return;
		}
	}

	lista.push_back(w);
	if (limit_size != -1 && lista.size() > limit_size)
	{
		lista.pop_back();
	}
	return;
}
void Heuristic::insertordered2(int s, int w, list<int> &lista)
{
	//Primero los abiertos o los cerrados pero que abriéndolos satisfagan la demanda.

	if (lista.size() == 0)
	{
		lista.push_back(w);
		return;
	}
	int wh;

	double costh;
	double cost = problem->supplycost[s][w];


	for (auto it = lista.begin(); it != lista.end(); it++)
	{
		wh = (*it);
		costh = problem->supplycost[s][wh];

		if (cost <= costh)
		{
			lista.insert(it, w);
			return;
		}
	}

	lista.push_back(w); 
	return;
}
void Heuristic::allocate_s()
{

	int cost_w;
	int warehouse;
	int store;
	list<int> openOs;

	vector<int> order_h_w;


	for (int s = 0; s < problem->num_S && current_s->num_sin_servir_S>0; s++) // Solo mientras nos queden Stores sin servir.
	{
		store = ord_S[s];
		if (current_s->sin_servir_S[store] > 0) //me quedan cosas que mandar a ese store
		{
			openOs.clear();
			//Ordeno 
			for (auto it =openW.begin(); it !=openW.end(); it++)
			{
				if(current_s->disponible_W[(*it)]>0)
					insertordered2(store, (*it), openOs);
			}

			for (auto it = openOs.begin(); it != openOs.end() && current_s->sin_servir_S[store] > 0; it++)
			{
				serve_w(store, (*it)); //Asigno el store al warehouse
			}

			if (current_s->sin_servir_S[store] > 0)
			{
				order_h_w = ord_W;
				//Ordenamos aleatoriamente el vector de Stores
				random_shuffle(order_h_w.begin(), order_h_w.end());

				for (int kk = 0; kk < top && current_s->sin_servir_S[store] > 0; kk++)
				{
					warehouse = problem->best_warehouse_S[store][kk]; //El mejor desde el que podría servirse el store

					if (current_s->disponible_W[warehouse] > 0 && current_s->soly[warehouse] == 0)
						serve_w(store, warehouse); //Asigno el store al warehouse
				}

				//Ordenamos aleatoriamente el vector de Stores
				for (int w = 0; w < problem->num_W && current_s->sin_servir_S[store] > 0; w++)
				{
					warehouse = order_h_w[w];
					if (current_s->disponible_W[warehouse] > 0 && (current_s->soly[warehouse] == false || (current_s->soly[warehouse] == true && buscar_compatibilidad(store, warehouse))))
					{
						serve_w(store, warehouse); //Asigno el store al warehouse
					}
				}
			}
		}
	}
}


void Heuristic::ls_remove_perc_s(int n)
{
	int store;
	int num_s;
	vector<int> s_del;
	bool insertar;
	int n_remv;

	// Para ordenar aleatoriamente los S
	random_shuffle(ord_S.begin(), ord_S.end());

	//Si podemos saber los W que están abiertos, no hace falta ir hasta los num_W
	for (int s = 0; s < problem->num_S&& n>0; ++s)
	{
		store = ord_S[s];
		n_remv = 0;
		for (int w = 0; w < problem->num_W && current_s->sin_servir_S[store] < problem->goods[store]; w++)
		{
			if (current_s->solx[store][w] > 0 && (problem->best_warehouse_S[store][0]!=w || (problem->best_warehouse_S[store][0] == w  && current_s->solx[store][w] == (problem->capacity[w]-current_s->disponible_W[w]))))
			{
				delete_sw(store, w);
				n_remv++;
			}
		}
		if (n_remv > 0)
			n--;
	}
	return;
}

void Heuristic::ls_remove_all_w(int n, vector<int> &Open)
{
	int warehouse;
	random_shuffle(Open.begin(), Open.end());
	for (int w = 0; w < n; w++)
	{
		warehouse = Open[w];
		for (int s = 0; s < problem->num_S && current_s->disponible_W[warehouse]<problem->capacity[warehouse]; s++)
		{
			if (current_s->solx[s][warehouse] > 0)
			{
				delete_sw(s, warehouse);
			}
		}
	}
	return;
}



void Heuristic::ls_remove_perc_w(int n, vector<int> &Open)
{
	int warehouse;
	random_shuffle(Open.begin(), Open.end());
	int n_removed;
	int r_min;
	int r_max;
	int r_del;
	vector<int> remov_s;

	for (int w = 0; w < n; w++)
	{
		warehouse = Open[w];
		n_removed = 0;
		remov_s.clear();
		for (int s = 0; s < problem->num_S; s++)
		{
			if (current_s->solx[s][warehouse] > 0)
			{
				n_removed++;
				remov_s.push_back(s);
			}
		}
		if (n_removed == 0 && n<Open.size())
		{
			n++;
		}

		if (remov_s.size() > 0)
		{
			r_min = 1;
			r_max = int(remov_s.size() / 2);

			if (r_max == 0)
				r_del = 1;
			else
				r_del = r_min + int(rand() % (r_max + 1 - r_min));

			random_shuffle(remov_s.begin(), remov_s.end());
			for (int i=0; i<remov_s.size(); i++)
			{
				delete_sw(remov_s[i], warehouse);
			}
		}
	}
	return;
}



void Heuristic::delete_sw(int s, int w)
{

	int dem_del= current_s->solx[s][w]; // Cantidad a eliminar
	//Si el Store estaba totalmente servido, pasa a estar sin servir
	if (current_s->sin_servir_S[s] == 0)
		current_s->num_sin_servir_S++;

	current_s->solx[s][w] = 0;
	current_s->sin_servir_S[s] += dem_del;

	current_s->disponible_W[w] += dem_del;
	current_s->value -= dem_del * problem->supplycost[s][w];

	// Si disponible_W se queda a capacity[w](se eliminan todos los stores) -> Se tiene que cerrar el W
	if (current_s->disponible_W[w] == problem->capacity[w])
	{
		current_s->value -= problem->fixedcost[w];
		current_s->soly[w] = 0;
	}


}

void copy_sol(csolution_t *sol_d, csolution_t *sol_o, problem_t *problem)
{
	for (int s = 0; s < problem->num_S; s++)
	{
		sol_d->sin_servir_S[s] = sol_o->sin_servir_S[s];
		for (int w = 0; w < problem->num_W; w++)
		{
			sol_d->solx[s][w] = sol_o->solx[s][w];
		}
	}
	for (int w = 0; w < problem->num_W; w++)
	{
		sol_d->soly[w] = sol_o->soly[w];
		sol_d->disponible_W[w] = sol_o->disponible_W[w];
	}
	sol_d->value = sol_o->value;
	sol_d->num_sin_servir_S = sol_o->num_sin_servir_S;
}

void Heuristic::constructive(struct timeb t_ini, struct timeb t1, solution_t *solution,int max_iter)
{
	int n_iter_WI = 0;
	double timet;
	double timetotal;
	double timem;
	double perc = 100.0;

	ftime(&t1);
	timem = ((double)((t1.time - t_ini.time) * 1000 + t1.millitm - t_ini.millitm)) / 1000;
	timetotal = timem;
	timet = 0;

	// Nos generamos la copia en iter_s
	clear_current_sol();
	copy_sol(iter_s, current_s, problem);

	while (timetotal < problem->Tmax && n_iter_WI < max_iter)
	{
		clear_current_sol(); //Se actualiza la solución a cero
		constructive_phase_w(perc); //Con la fase constructiva, se obtiene solución factible
		
#ifdef PRINT_S
		fprintf(stdout, "CPhase:\t%.2f\n", current_s->value);
#endif

		if (current_s->num_sin_servir_S == 0 && (n_iter_WI == 0 || current_s->value < iter_s->value))
		{
			copy_sol(iter_s, current_s, problem);
		}

		if (n_iter_WI % 2 == 1)
			perc = perc - 10;

		n_iter_WI++;
		ftime(&t1);
		timetotal = (((double)((t1.time - t_ini.time) * 1000 + t1.millitm - t_ini.millitm)) / 1000);
	}

	copy_sol(current_s, iter_s, problem);
	openW.clear();
}

void Heuristic::local_search_perc_s(struct timeb t_ini, struct timeb t1, solution_t *solution)
{
	//Entro en esta búsqueda local, como máximo hago 10 iteraciones sin mejora.

	int max_iter_WI = 20; //Número máximo de iteraciones sin mejora
	int n_iter_WI = 0;
	double maxtimeh;
	if (problem->reduced == true)
	{
		maxtimeh = (problem->Tmax);
	}
	else
	{
		maxtimeh = (problem->Tmax) / 10;
	}
	double timet;
	double timetotal;
	double timem;

#if 0
	int ini_rem = 1;
	int c_rem = ini_rem;
#else
	int s_del_min;
	int s_del_max;
#endif
	int s_del; //Stores que eliminar
	vector<int> wOpen;

	ftime(&t1);

	timem = ((double)((t1.time - t_ini.time) * 1000 + t1.millitm - t_ini.millitm)) / 1000;

	timetotal = timem;



	copy_sol(iter_s, current_s, problem);

#if 0
	s_del = (problem->num_S*c_rem) / 100;
#else
	s_del_min = int((problem->num_S*1) / 100);
	s_del_max = int((problem->num_S * 5) / 100);
#endif

	while (timet < maxtimeh && timetotal < problem->Tmax && n_iter_WI < max_iter_WI)
	{
#if 1
		s_del = s_del_min + int(rand() % (s_del_max + 1 - s_del_min));
#endif

		//Eliminamos un 5% de los abiertos.
		ls_remove_perc_s(s_del); // 
		ls_allocate_s(iter_s->value); // Recolocamos todos los S que han quedado sin asignar.

		if (current_s->num_sin_servir_S == 0 && current_s->value < iter_s->value)
		{

			improve = true;
			if (current_s->value < solution->value || solution->value == -1)
			{
#ifdef PRINT_S
				fprintf(stdout, "LS_perc_s:\t%.2f\tPrevious:\t%.2f\tniterWI\t%d minWI\t%d maxWI\t%d\tremWI\t%d\n", current_s->value, iter_s->value, n_iter_WI, s_del_min, s_del_max, s_del);
#endif
				save_sol(solution,t_ini);
			}
			n_iter_WI = 0; //He mejorado, actualizo a cero el número de iteraciones sin mejora.
#if 0
			c_rem = ini_rem; //Eliminamos pocos al principio
#endif

			copy_sol(iter_s, current_s, problem);
		}
		else
		{
			//No se ha terminado de completar la solución
			n_iter_WI++; //Aumenta el número de iteraciones sin mejora
#if 0
			if (c_rem < 5)
				c_rem = c_rem++; //Eliminamos un poco más
			else
				c_rem = 1;
#endif

			copy_sol(current_s, iter_s, problem);
		}
		//Chequeamos como vamos de tiempo
		ftime(&t1);
		timetotal = (((double)((t1.time - t_ini.time) * 1000 + t1.millitm - t_ini.millitm)) / 1000);
		timet = timetotal - timem;
	}
	return;
}



void Heuristic::local_search_perc_w(struct timeb t_ini, struct timeb t1, solution_t *solution)
{
	//Entro en esta búsqueda local, como máximo hago 10 iteraciones sin mejora.

	int max_iter_WI = 20; //Número máximo de iteraciones sin mejora
	int n_iter_WI = 0;
	double maxtimeh;
	if (problem->reduced == true)
	{
		maxtimeh = (problem->Tmax);
	}
	else
	{
		maxtimeh = (problem->Tmax) / 10;
	}
	double timet;
	double timetotal;
	double timem;

#if 0
	int ini_rem = 1;
	int c_rem = ini_rem;
#else
	int W_del_min;
	int W_del_max;
#endif
	int w_opened;
	int W_del;

	vector<int> wOpen;

	ftime(&t1);

	timem = ((double)((t1.time - t_ini.time) * 1000 + t1.millitm - t_ini.millitm)) / 1000;

	timetotal = timem;



	copy_sol(iter_s, current_s, problem);

	for (int w = 0; w < problem->num_W; w++)
	{
		if (current_s->soly[w] == 1)
			wOpen.push_back(w);
	}


	while (timet < maxtimeh && timetotal < problem->Tmax && n_iter_WI < max_iter_WI)
	{
		
		//¿Cuántos hay abiertos?
		w_opened = wOpen.size();
		int hola;
		hola = 10;
		openW;
		//Eliminamos un 5% de los abiertos.
#if 0
//Eliminamos un 5% de los abiertos.
		W_del = (w_opened*c_rem) / 100;
#else
		//Eliminamos un 5% de los abiertos.
		W_del_min = int((w_opened * 1) / 100);
		W_del_max = int((w_opened * 5) / 100);
		W_del = W_del_min + int(rand() % (W_del_max + 1 - W_del_min));
#endif

		ls_remove_perc_w(W_del, wOpen); // Eliminamos los W_del W abiertos
		ls_allocate_s(iter_s->value); // Recolocamos todos los S que han quedado sin asignar.

		if (current_s->num_sin_servir_S == 0 && current_s->value < iter_s->value)
		{
			improve = true;
			if (current_s->value < solution->value || solution->value == -1)
			{
#ifdef PRINT_S
				fprintf(stdout, "LS_perc_w:\t%.2f\tPrevious:\t%.2f\tniterWI\t%d\t min\t%d\t max\t%d rem\t%d\n", current_s->value, iter_s->value, n_iter_WI, W_del_min, W_del_max, W_del);
#endif
				save_sol(solution,t_ini);
			}
			n_iter_WI = 0; //He mejorado, actualizo a cero el número de iteraciones sin mejora.
#if 0
			c_rem = ini_rem; //Eliminamos pocos al principio
#endif

			copy_sol(iter_s, current_s, problem);

			wOpen.clear();
			for (int w = 0; w < problem->num_W; w++)
			{
				if (current_s->soly[w] == 1)
					wOpen.push_back(w);
			}

		}
		else
		{
			//No se ha terminado de completar la solución
			n_iter_WI++; //Aumenta el número de iteraciones sin mejora
#if 0
			if (c_rem < 5)
				c_rem = c_rem++; //Eliminamos un poco más
			else
				c_rem = 1;
#endif

			copy_sol(current_s, iter_s, problem);

		}
		//Chequeamos como vamos de tiempo
		ftime(&t1);
		timetotal = (((double)((t1.time - t_ini.time) * 1000 + t1.millitm - t_ini.millitm)) / 1000);
		timet = timetotal - timem;
	}
	return;

}
void Heuristic::local_search_all_w(struct timeb t_ini, struct timeb t1, solution_t *solution)
{
	//Entro en esta búsqueda local, como máximo hago 10 iteraciones sin mejora.

	int max_iter_WI = 20; //Número máximo de iteraciones sin mejora
	int n_iter_WI = 0;
	double maxtimeh;
	if (problem->reduced == true)
	{
		maxtimeh = (problem->Tmax);
	}
	else
	{
		maxtimeh = (problem->Tmax)/10;
	}
	double timet;
	double timetotal;
	double timem; 
#if 0
	int ini_rem = 1;
	int c_rem = ini_rem;
#else
	int W_del_min;
	int W_del_max;
#endif
	int w_opened;
	int W_del;

	vector<int> wOpen;

	ftime(&t1);

	timem = ((double)((t1.time - t_ini.time) * 1000 + t1.millitm - t_ini.millitm)) / 1000;

	timetotal = timem;



	copy_sol(iter_s, current_s, problem);
	for (int w = 0; w < problem->num_W; w++)
	{
		if (current_s->soly[w] == 1)
			wOpen.push_back(w);
	}

	while (timet< maxtimeh && timetotal< problem->Tmax && n_iter_WI < max_iter_WI)
	{
		//¿Cuántos hay abiertos?
		w_opened = wOpen.size();

#if 0
		//Eliminamos un 5% de los abiertos.
		W_del = (w_opened*c_rem) / 100;
#else
		//Eliminamos un 5% de los abiertos.
		W_del_min = int((w_opened * 1) / 100);
		W_del_max = int((w_opened * 5) / 100);
		W_del = W_del_min + int(rand() % (W_del_max + 1 - W_del_min));
#endif

		
		ls_remove_all_w(W_del, wOpen); // Eliminamos los W_del W abiertos
		ls_allocate_s(iter_s->value); // Recolocamos todos los S que han quedado sin asignar.

		if (current_s->num_sin_servir_S == 0 && current_s->value < iter_s->value)
		{

			improve = true;
			if (current_s->value < solution->value || solution->value == -1)
			{
#ifdef PRINT_S
				fprintf(stdout, "LS_all_w:\t%.2f\tPrevious:\t%.2f\tniterWI\t%d\tmin\t%d\tmax\t%d rem\t%d\n", current_s->value, iter_s->value, n_iter_WI, W_del_min, W_del_max, W_del);
#endif
				save_sol(solution,t_ini);
			}
			n_iter_WI = 0; //He mejorado, actualizo a cero el número de iteraciones sin mejora.
#if 0
			c_rem = ini_rem; //Eliminamos pocos al principio
#endif
			copy_sol(iter_s, current_s, problem);

			wOpen.clear();
			for (int w = 0; w < problem->num_W; w++)
			{
				if (current_s->soly[w] == 1)
					wOpen.push_back(w);
			}

		}
		else
		{
			//No se ha terminado de completar la solución
			n_iter_WI++; //Aumenta el número de iteraciones sin mejora
#if 0
			if (c_rem < 5)
				c_rem = c_rem++; //Eliminamos un poco más
			else
				c_rem = 1;
#endif

			copy_sol(current_s, iter_s, problem);
		}
		//Chequeamos como vamos de tiempo
		ftime(&t1);
		timetotal = (((double)((t1.time - t_ini.time) * 1000 + t1.millitm - t_ini.millitm)) / 1000);
		timet = timetotal - timem;
	}
	return;

}
bool Heuristic::buscar_compatibilidad(int store, int WH)
//Retorna true si el Store 'store' puede entrar en el Warehouse WH;
{
	bool compatible = true;
	for (int s = 0; s < problem->num_S; s++)
	{
		if (current_s->solx[s][WH] > 0)
		{
			if (problem->inc[s][store] == 1)
			{
				compatible = false;
				break;
			}
		}
	}

	return compatible;
}


bool Heuristic::compatibilidad(int store, vector<int> sol)
//Retorna true si el Store 'store' puede entrar en el Warehouse WH y false si no o si ya había entrado;
{
	bool compatible = true;
	for (int s = 0; s < sol.size(); s++)
	{
			if (problem->inc[sol[s]][store] == 1)
			{
				compatible = false;
				break;
			}
	}

	return compatible;
}

#if 0
void Heuristic::check_solution()
{
	double c_sol_value=0;
	for (int w = 0; w < problem->num_W; w++)
	{
		c_sol_value += current_s->soly[w] * problem->fixedcost[w];
		for (int s = 0; s < problem->num_S; s++) {
			c_sol_value += current_s->solx[s][w] * problem->supplycost[s][w];
		}
	}
	if (current_s->value - c_sol_value != 0) {
		cout << "### ERROR AL CALCULAR EL VALOR DE LA F.O. DE LA SOLUCIÓN ###" << endl;
		cout << "------------------------------------------------------------" << endl;
		cout << "El valor de la solucion es: " << current_s->value << " cuando deberia ser: " << c_sol_value << endl;
		cout << "Stores que quedan sin servir: " ;
		for (int s = 0; s < problem->num_S; s++) {
			cout << current_s->sin_servir_S[s] << " ";
		}
		cout << ""<< endl;

		for (int w = 0; w < problem->num_W; w++)
		{
			/*cout << current_s->soly[w] << " ";
			cout << problem->fixedcost[w] << endl;

			for (int s = 0; s < problem->num_S; s++)
			{
				cout << current_s->solx[s][w] << " ";
				cout << problem->supplycost[s][w] << " ";
				cout << current_s->solx[s][w] * problem->supplycost[s][w] << " || ";
				cout << problem->goods[s] << " " << endl;
			}*/
			double incumbent_sol_value = 0;
			incumbent_sol_value += current_s->soly[w] * problem->fixedcost[w];
			for (int s = 0; s < problem->num_S; s++) {
				incumbent_sol_value += current_s->solx[s][w] * problem->supplycost[s][w];
			}
			cout << "El Warehouse: " << w+1 << " tiene un coste asociado " << incumbent_sol_value << endl;
			
		}
	}

	bool fallo_compatibilidad = false;
	for (int w = 0; w < problem->num_W; w++)
	{
		for (int s = 0; s < problem->num_S - 1 && !fallo_compatibilidad; s++)
		{

			for (int s1 = s; s1 < problem->num_S; s1++)
			{
				if (current_s->solx[s][w] > 0 && current_s->solx[s1][w] > 0)
				{
					if (problem->inc[s][s1] == 1)
					{
						fallo_compatibilidad = true;
						cout << "### ERROR EN COMPATIBILIDAD DE LOS WAREHOUSES ###" << endl;
						cout << "------------------------------------------------------------" << endl;
						cout << "Error de compatibilidad en el Warehouse " << w+1 << " entre los Stores (" << s+1 << ", " << s1+1 << ")" << endl;
						break;
					}
				}
			}
		}
	}

}
#endif
void Heuristic::service(int store, int warehouse, int asign)
{
	if(asign==0)	
		asign = min(current_s->sin_servir_S[store], current_s->disponible_W[warehouse]);
	current_s->sin_servir_S[store] -= asign;

	if (current_s->sin_servir_S[store] == 0)
		current_s->num_sin_servir_S--;

	current_s->solx[store][warehouse] += asign;
	current_s->disponible_W[warehouse] -= asign;
	current_s->value += asign * problem->supplycost[store][warehouse];
	/*cout << "Añado a w " << warehouse << " el s " << store << ". Disponible en w " << current_s->disponible_W[warehouse] << " y en s " << current_s->sin_servir_S[store] << endl;*/
}

void Heuristic::order_W_vector()
{
	//Ordenamos los almacenes. En primer lugar los que sean el mejor para una tienda un mayor número de veces y, en caso de empate, aquellos para los que haya una mayor diferencia en coste entre servir una tienda con su mejor y con su segundo mejor.
	for (int s = 0; s < problem->num_S; s++)
	{
		frec_W_best[problem->best_warehouse_S[s][0]]= frec_W_best[problem->best_warehouse_S[s][0]] +1;
	}
	double ccost_open;
	double cost_open;
	int warehouse;
	for (int w = 0; w < problem->num_W; w++)
	{
		ccost_open = double(problem->fixedcost[w]) / double(problem->capacity[w]);
		for (int w2 = 0; w2 < problem->num_W; w2++)
		{
			warehouse = ord_W[w2];
			if (warehouse == -1)
			{
				ord_W[w2] = w;
				w2 = problem->num_W;
			}
			else
			{
				cost_open = double(problem->fixedcost[warehouse])/ double(problem->capacity[warehouse]);
				if (ccost_open < cost_open || (ccost_open == cost_open && problem->capacity[w]> problem->capacity[warehouse]))
				{
					for (int w3 = problem->num_W - 1; w3 >= w2 + 1; w3--)
						ord_W[w3] = ord_W[w3 - 1];
					ord_W[w2] = w;
					w2 = problem->num_W;
				}
			}
		}
	}
}

