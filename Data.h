#ifndef DATA_H_
#define DATA_H_
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
//#define PRINT_S
#define PRINT2

class Data
{
private:



public:
	Data();
	~Data(void);

	void leer_Instancia(string fichero); //Read the graph from the file

	string nombre_instancia; // Name of the Instance
	string nombre_sol;

	int num_W; // Number of Warehouses;
	int num_S; // Number of Stores;

	int *FixedCost; // Fixed Cost per Warehause open
	int *Capacity; // Capacity per Warehause
	int *Goods; // Demand per Store
	int **SupplyCost; //Distancia <Stores, Warehouses>
	
	int num_Inc; // Number of Incompatibilities;
	vector<int> Inc_s;
	vector<vector<int>> Inc; // Incompatibility matrix
	pair<int, int> *IncPairs; // Pairs of stores that are incompatibilities
	list<vector<int>> Incompatibilities;


	vector<vector<int>> bestsol; // Solution matrix
	bool reduced;

	int **solx;
	int *soly;
	int **solz;

	int optimal;  //0 : Infeasible || 1 : Óptimo || 2 : Termina por sol. enteras
	int opt_sol; // Value of the optimal solution
	int nnodes; //número de nodos del B&C
	double Tmax; //Maximum time
	double time_MIP; //tiempo total de ejecución del B&C
	void createlist(list<int> &, bool &);
};
#endif