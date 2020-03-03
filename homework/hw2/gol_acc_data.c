#include <stdio.h>
#include <stdlib.h>
#include "seq_time.h"

#define SRAND_VALUE 1985

#define dim 1024 // grid dimension excluding ghost cells

void gol(int *grid, int *newGrid)
{
    int i,j;

    // ghost rows
    for (i = 1; i <= dim; i++) {
      // copy first row to bottom ghost row
      grid[(dim+2)*(dim+1)+i] = grid[(dim+2)+i];

      // copy last row to top ghost row
      grid[i] = grid[(dim+2)*dim + i];
    }

    // ghost columns
    for (i = 0; i <= dim+1; i++) {
      // copy first column to right most ghost column
      grid[i*(dim+2)+dim+1] = grid[i*(dim+2)+1];

      // copy last column to left most ghost column
      grid[i*(dim+2)] = grid[i*(dim+2) + dim];
    }

    // iterate over the grid
    #pragma acc parallel loop copyin(grid[:(dim+2)*(dim+2)]) copy(newGrid[:(dim+2)*(dim+2)])
    for (i = 1; i <= dim; i++) {
       #pragma acc loop
        for (j = 1; j <= dim; j++) {
            int id = i*(dim+2) + j;

            int numNeighbors =
                grid[id+(dim+2)] + grid[id-(dim+2)]   // lower + upper
                + grid[id+1] + grid[id-1]             // right + left
                + grid[id+(dim+3)] + grid[id-(dim+3)] // diagonal lower + upper right
                + grid[id-(dim+1)] + grid[id+(dim+1)];// diagonal lower + upper left

            // the game rules
            if (grid[id] == 1 && numNeighbors < 2)
                newGrid[id] = 0;
            else if (grid[id] == 1 && (numNeighbors == 2 || numNeighbors == 3))
                newGrid[id] = 1;
            else if (grid[id] == 1 && numNeighbors > 3)
                newGrid[id] = 0;
            else if (grid[id] == 0 && numNeighbors == 3)
                newGrid[id] = 1;
            else
                newGrid[id] = grid[id];
        }
    }

    // copy new grid over, as pointers cannot be switched on the device
    #pragma acc parallel loop copyin(newGrid[:(dim+2)*(dim+2)]) copyout(grid[:(dim+2)*(dim+2)])
    for(i = 1; i <= dim; i++) {
       #pragma acc loop 
        for(j = 1; j <= dim; j++) {
            int id = i*(dim+2) + j;
            grid[id] = newGrid[id];
        }
    }
}

int main(int argc, char* argv[])
{
    int i, j;

    // number of game steps
    int itEnd = 1 << 11; // 2^11 = 2048
    if(argc > 1){
        itEnd = atoi(argv[1]);
    }

    // grid array with dimension dim + ghost columns and rows
    int    arraySize = (dim+2) * (dim+2);
    size_t bytes     = arraySize * sizeof(int);
    int    *grid     = (int*)malloc(bytes);

    // allocate result grid
    int */*restrict*/newGrid = (int*) malloc(bytes);

    // assign initial population randomly
    srand(SRAND_VALUE);
    for(i = 1; i <= dim; i++) {
        for(j = 1; j <= dim; j++) {
            grid[i*(dim+2)+j] = rand() % 2;
        }
    }

    int total = 0; // total number of alive cells


    double start = c_get_wtime();
    int it;
    #pragma acc data copyin(grid[:arraySize]) copy(newGrid[:arraySize])
    for(it = 0; it < itEnd; it++){
        gol( grid, newGrid );
    }

    // sum up alive cells
    for (i = 1; i <= dim; i++) {
        for (j = 1; j <= dim; j++) {
            total += grid[i*(dim+2) + j];
        }
    }
    double end = c_get_wtime();
    double total_time = end - start;
    printf("Time: %f seconds\n", total_time);
    printf("Total Alive: %d\n", total);

    return 0;
}
