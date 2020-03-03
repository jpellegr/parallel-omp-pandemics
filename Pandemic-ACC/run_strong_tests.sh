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
printf "trial \t#th\t1500000 \t6000000\t24000000 \t96000000 \t384000000 \t1536000000\n"

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
      for problem_size in 500000
      do
          command="./Pandemic  -p$num_threads -n$problem_size -w 1414 -h 1414"


        $command
        #printf "$command  "
      done
      printf "\n"
      ((counter++))
  done
  printf "\n"

done
