#!/bin/bash

#	10 runs each
#	Type(3): pthreads, openmp, mpi
#	Cpu(10): 1,2,4,8,16,32,2x4, 4x4, 16x2, 2x16
#	Problem size(4): 10k, 100k, 500k, 1M
mem='8G'
cons='elves'
ptype='mpi'
capptype='MPI'
nthreads='1 2 4 8 16 32'
problem='10000 100000 500000 1000000'
ncores='1 2 4 8 16'

for i in $ncores
do
    for j in $problem
    do
        for k in $nthreads
        do
		for l in $mem
	   	do
			for((z = 0;z<3;z++))
			do	
		 	sbatch --constraint=$cons --mail-type=END --time=03:00:00 --mem=$l --ntasks-per-node=$i LCS_$ptype.sh "$capptype $cons CORES: $i" $j $k $z
       			done
		done
	 done
    done
done
#2x4, 4x4, 16x2, 2x16
for j in $problem
do
 sbatch --constraint=$cons --mail-type=END --time=03:00:00 --mem=$mem --nodes=4 LCS_$ptype.sh "$capptype $cons CONFIG: 2x4" $j 2
done

for j in $problem
do
 sbatch --constraint=$cons --mail-type=END --time=03:00:00 --mem=$mem --nodes=4 LCS_$ptype.sh "$capptype $cons CONFIG: 4x4" $j 4
done

for j in $problem
do
  sbatch --constraint=$cons --mail-type=END --time=03:00:00 --mem=$mem --nodes=2 LCS_$ptype.sh "$capptype $cons CONFIG: 16x2" $j 16
done

for j in $problem
do 
  sbatch --constraint=$cons --mail-type=END --time=03:00:00 --mem=$mem --nodes=16 LCS_$ptype.sh "$capptype $cons CONFIG: 2x16" $j 2
done
