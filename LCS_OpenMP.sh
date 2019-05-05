#!/bin/bash
fname='/homes/dan/625/wiki_dump.txt'
#problem='10000 100000 500000 1000000'

echo$1
#for i in $problem
#do
	echo "---PROBLEM SIZE: $2---"
	echo "---THREAD COUNT: $3---"
	./LCS_OpenMP $fname $2 $3
#done