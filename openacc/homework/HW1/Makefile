# Makefile for countSort examples
#
# On wulver, we have installed gcc version 7, which will be helpful for
# a particular OpenMP feature that you will need.
CC=gcc-7 -std=gnu99
OMP=-fopenmp

all: countSort_seq  countSort_omp

countSort_seq: countSort_seq.c seq_time.h
	${CC} -o countSort_seq countSort_seq.c

countSort_omp: countSort_omp.c
		${CC} ${OMP} -o countSort_omp countSort_omp.c

clean:
	rm -f countSort_seq countSort_omp
