#!/bin/bash

# Runs a series of tests that can fairly easily be copied into the spreadsheet.

# Author: Libby Shoop

# Usage:
#          bash ./run_strong_tests.sh 4
#    will run each problem size set below 4 times using a variety of thread counts

# Notes: 1. you must set the number of times you want to run each test
#           by including it on the command line.
#       2. both trap-seq.c and trap-omp.c need to be updated to print out only
#          this line and ALL OTHER printf lines should be commented:
#         printf("%lf\t",elapsed_time);
num_times=$1

# print a header for the trial, #threads, and set of probelm sizes
# Note: you should set the problem sizes that you want to run for a problem.
#       The following are fairly good for the trapezoidal rule problem
#       and for the strong scalability problem sizes and number of threads.
printf "trial \t#th\t1048576 \t2097152 \t4194304 \t8388608 \t16777216 \t33664432\n"

# each trial will run num_times using a cretain number of threads
for num_threads in 1 2 4 6 8 12 16
do

  # run the series of problem sizes with the current value of num_threads
  counter=1
  while [ $counter -le $num_times ]
  do
     # $counter is the trial number
      printf "$counter\t$num_threads\t"

     # run each problem size once
      for problem_size in 781250 3125000 12500000 50000000 200000000 800000000
      do
        if [  "$num_threads" == "1"  ]; then
          command="./countSort_seq -n$problem_size"
        else
          command="./countSort_omp -n$problem_size -t$num_threads"
        fi

        $command
        #printf "$command  "
      done
      printf "\n"
      ((counter++))
  done
  printf "\n"

done
