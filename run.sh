#!/bin/bash

# você roda assim:
#   sbatch --exclusive -N 8 run.sh

mpirun -N 1 ./mmul 2100 1000 800