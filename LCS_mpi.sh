#!/bin/bash
fname='/homes/dan/625/wiki_dump.txt'
#problem='10000 100000 500000 1000000'

nthreads=$3
module load OpenMPI
echo$1
#for i in $problem
#do
	echo "---PROBLEM SIZE: $2---"
	echo "---THREAD COUNT: $3---"
	mpirun -np $nthreads /homes/vmramosa/MPI/LCS_mpi $fname $2
#done
		
