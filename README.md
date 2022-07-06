# GRASP-solver
This repository stores the C++ source code of the Greedy Randomized Adaptive Search Procedures (GRASP) solver for the MS-CFLP-CI


Compile the solver with:

`makefile`

Run it with:

`./GRASP.exe <input_file> <solution_file> <timeout> <random_seed>`

For example, the command line:

`GRASP.exe ../Instances/Public/wlp01.dzn sol01.txt 20 1256786`

Runs for 20s the solver on instance `wlp01.dzn` stored in the directory `../Instances/Public/` and delivers the solution in the file `sol01.txt`.
