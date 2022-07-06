OPTIONS = -O3
OBJ_FILES = main.o Heuristic.o solution.o problem.o Data.o

WLPCI_MC: $(OBJ_FILES)
	g++ -o WLPCI_MC $(OBJ_FILES)

Data.o: Data.cpp Data.h
	g++ $(OPTIONS) -c Data.cpp

problem.o: problem.cpp problem.h
	g++ $(OPTIONS) -c problem.cpp

solution.o: solution.cpp solution.h Data.h
	g++ $(OPTIONS) -c solution.cpp

Heuristic.o: Heuristic.cpp Heuristic.h problem.h solution.h Data.h
	g++ $(OPTIONS) -c Heuristic.cpp

main.o: main.cpp Heuristic.h problem.h solution.h Data.h
	g++ $(OPTIONS) -c main.cpp

clean:
	rm -f WLPCI_MC $(OBJ_FILES)
