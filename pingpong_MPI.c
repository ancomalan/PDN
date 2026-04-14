#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
    // Catch console errors
    if (argc != 3)
    {
        printf("USE LIKE THIS: pingpong_MPI n_items time_prob1_MPI.csv\n");
        return EXIT_FAILURE;
    }

    /* Read in command line items */
    int n_items = strtol(argv[1], NULL, 10);
    FILE *outputFile = fopen(argv[2], "w");

    /* Start up MPI */
    int my_rank;
    // TODO: finish setting up MPI
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // get rank among all processes

    int *ping_array = malloc(n_items * sizeof(int)); // buffer for both processes

    // Start time
    double starttime;

    // TODO: Create your MPI program.
    if (my_rank == 0)
    {

        // Fill array with incremental values
        for (int i = 0; i < n_items; i++)
            ping_array[i] = i;

        starttime = MPI_Wtime(); // don't time the array getting populated

        // TODO: if myrank is 0
        // back and forth 1000 times
        for (int j = 0; j < 1000; j++)
        {
            MPI_Send(ping_array, n_items, MPI_INT, 1, 0, MPI_COMM_WORLD);                    // step 1: send to process 1
            MPI_Recv(ping_array, n_items, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // step 4: receive from process 1
        }

        // End time
        double endtime = MPI_Wtime();
        // TODO: output
        double elapsed = endtime - starttime;
        double average_one_way = elapsed / 2000.0; // there were 2000 one way trips (2 sends in each iteration of for loops)

        // write to output file
        fprintf(outputFile, "%d,%f\n", n_items, average_one_way);
        fclose(outputFile);
    }
    else
    {

        // TODO: if my rank not 0
        for (int k = 0; k < 1000; k++)
        {
            MPI_Recv(ping_array, n_items, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // step 2: receive from process 0
            MPI_Send(ping_array, n_items, MPI_INT, 0, 0, MPI_COMM_WORLD);                    // step 3: send back to process 0
        }
    }

    free(ping_array);
    MPI_Finalize(); // shut down MPI
    return 0;
} /* main */
