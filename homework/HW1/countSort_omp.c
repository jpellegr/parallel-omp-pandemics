/*
 * Count sort example
 *
 * Applicability: When the keys are integers drawn from a
 * small range, such as 8-bit numbers, and therefore contain
 * many duplicates.
 *
 * Author: Libby Shoop
 *        with inspiration from:
 *        https://www8.cs.umu.se/kurser/5DV011/VT13/F8.pdf
 *
 * compile with:
 *     gcc -std=gnu99 -o countSort_seq countSort_seq.c
 *
 * Usage example (designed to take in various problem sizes for input string):
 *     ./countSort -n 8388608
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <omp.h>

#include "seq_time.h"  // Libby's timing function that is similar to omp style

#define ASCII_START 32
#define ASCII_END 126
#define COUNTS_SIZE ASCII_END+1

// functions in this file
void seedThreads(int * seeds);
void getArguments(int argc, char *argv[], int * N, int * num_threads);
char* generateRandomString(int size, unsigned int * seeds);
void countEachLetter(int * counts, char * input, int N);
void createSortedStr(int * counts, char * output, int N);

int main(int argc, char *argv[]) {


    int N;  // size of input string
    int num_threads =1;

    getArguments(argc, argv, &N, &num_threads);  // should exit if -n has no value

    omp_set_num_threads(num_threads);

    unsigned int seeds[num_threads];
    double start, end;


    //generate input thread
    //start = omp_get_wtime();    // see seq_time.h if you want details
    // create fake sample data to sort
    seedThreads(seeds);
    char * input = generateRandomString(N, seeds);
    //end = omp_get_wtime();
    //printf("generate input: %f seconds\n", end - start);


    double total_start = omp_get_wtime();

    //debug
    //printf("input: %s\n", input);
    int counts[COUNTS_SIZE] = {0}; // indexes 0 - 31 should have zero counts.
                                 // we'll include them for simplicity


    //start = omp_get_wtime();
    countEachLetter(counts, input, N);
    //end = omp_get_wtime();
    //printf("generate counts: %f seconds\n", end - start);


    //start = omp_get_wtime();
    createSortedStr(counts, input, N);   //put the result back into the input
    //end = omp_get_wtime();
    //printf("generate output: %f seconds\n", end - start);


    //debug
    //printf("output: %s\n", input);
    double total_end = omp_get_wtime();
    double elapsed_time =total_end-total_start;
    printf("%lf\t",elapsed_time);
    free(input);
    return 0;
}

// process command line looking for the number of characters
// in the input string as our 'problem size'. Set the value of N
// of N to that number or generate error if not provided.
//   see:
// https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt
//
void getArguments(int argc, char *argv[], int * N, int * num_threads) {
    char *nvalue;
    char *tvalue;
    int c;        // result from getopt calls
    int d;
    int nflag = 0;
    int tflag = 0;

    while ((c = getopt (argc, argv, "n:t:")) != -1) {
      switch (c)
        {
        case 'n':
          nflag = 1;
          nvalue = optarg;
          *N = atoi(nvalue);
          break;
        case 't':
          tflag = 1;
          tvalue = optarg;
          *num_threads = atoi(tvalue);
          break;
        case '?':
          if (optopt == 'n') {
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
          } else if (isprint (optopt)) {
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
          } else {
            fprintf (stderr,
                     "Unknown option character `\\x%x'.\n",
                     optopt);
            exit(EXIT_FAILURE);
          }

        }
    }
    if (nflag == 0) {
      fprintf(stderr, "Usage: %s -n size\n", argv[0]);
      exit(EXIT_FAILURE);
    }
    if (tflag ==0) {
      printf("Number of threads left at 1.");
    }
}

void seedThreads(int * seeds) {
    int my_thread_id;
    unsigned int seed;
    #pragma omp parallel private (seed, my_thread_id)
    {
        my_thread_id = omp_get_thread_num();

        //create seed on thread using current time
        unsigned int seed = (unsigned) time(NULL);

        //munge the seed using our thread number so that each thread has its
        //own unique seed, therefore ensuring it will generate a different set of numbers
        seeds[my_thread_id] = (seed & 0xFFFFFFF0) | (my_thread_id + 1);

        //printf("Thread %d has seed %u\n", my_thread_id, seeds[my_thread_id]);
    }




}

//  Creation of string of random printable chars.
//  Normally this type of string would come from a data source.
//  Here we generate them for convenience.
//
char* generateRandomString(int size, unsigned int * seeds) {
    //unsigned int seed = time(NULL);
    int i;
    char *res = malloc(size + 1);
    int tid;
    int seed;
    #pragma omp parallel default(none) private(i,seed, tid) shared(seeds, size,res)
    {
      tid = omp_get_thread_num();
      seed = seeds[tid];
      srand(seed);
      #pragma omp for
      for(i = 0; i < size; i++) {
          res[i] = (char) ((rand_r(&seed) % (ASCII_END-ASCII_START)) + ASCII_START);
      }
    }
    res[size] = '\0';  //so it is a string we can print when debugging
    return res;
}

// The input called counts is designed so that an index into it
// represents tha code of an ascii character. The array input holds
// such ascii characters.
// More generally, the indices into counts are the set of keys we
// are sorting.
// We will first count how many times we see each key.
void countEachLetter(int * counts, char * input, int N) {
    // Count occurences of each key, which are characters in this case
    //int section_size =
    int k;
    #pragma omp parallel for default(none) shared(input,N) private(k) reduction(+:counts[:COUNTS_SIZE])
    for (k = 0; k < N ; ++ k ) {
        counts [ input [ k ] ] += 1;
        //printf("%d\n", input[k]);
    }


}

// Create the sorted string by adding chars in order to an output, using
// the count to create the correct number of them.
//
// We choose to save space by instead placing the final
// result back into input. See use above.
void createSortedStr(int * counts, char * output, int N) {
    // Construct prefix sum array from counts
    int prefixSum[COUNTS_SIZE];
    prefixSum[0] = counts[0];
    for (int i=1; i < COUNTS_SIZE; i++) {
      prefixSum[i] = prefixSum[i-1] + counts[i];
    }




    #pragma omp parallel for 
    for (int v = 0; v <= ASCII_END; ++ v ) {
      if (counts[v] !=0) {
        int end = prefixSum[v]-1;
        int start=end- counts[v] +1;
       for (int k= start; k <=  end; ++ k ) {
        output[k] = v;
       }
      }
    }

    output[N+1] = '\0';   // so it is a string we could print for debugging
}
