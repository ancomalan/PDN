#include <stdio.h>
#include <stdlib.h> // for strtol
#include <string.h>
#include <time.h>
#include <mpi.h>
#define MAXCHAR 25
#define BILLION 1000000000.0

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        printf("USE LIKE THIS: dot_product_MPI n_items vec_1.csv vec_2.csv result_prob2_MPI.csv time_prob2_MPI.csv \n");
        return EXIT_FAILURE;
    }

    // start up MPI
    MPI_Init(NULL, NULL);

    // get number of processes
    int comm_sz = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // get rank among all processes
    int my_rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Size
    int vec_size = strtol(argv[1], NULL, 10);

    // all processes declare vec_1 and vec_2 but only process 0 allocates it
    double *vec_1 = NULL;
    double *vec_2 = NULL;

    // each process allocates memory for local vectors
    double *local_vec_1 = malloc((vec_size / comm_sz) * sizeof(double));
    double *local_vec_2 = malloc((vec_size / comm_sz) * sizeof(double));

    // for process 0 timing
    double starttime;
    double endtime;
    // Process 0 will read in the two input arrays
    if (my_rank == 0)
    {
        // Input files
        FILE *inputFile1 = fopen(argv[2], "r");
        FILE *inputFile2 = fopen(argv[3], "r");
        if (inputFile1 == NULL)
            printf("Could not open file %s", argv[2]);
        if (inputFile2 == NULL)
            printf("Could not open file %s", argv[3]);

        // To read in
        vec_1 = malloc(vec_size * sizeof(double));
        vec_2 = malloc(vec_size * sizeof(double));

        // Store values of vector
        int k = 0;
        char str[MAXCHAR];
        while (fgets(str, MAXCHAR, inputFile1) != NULL)
        {
            sscanf(str, "%lf", &(vec_1[k]));
            k++;
        }
        fclose(inputFile1);

        // Store values of vector
        k = 0;
        while (fgets(str, MAXCHAR, inputFile2) != NULL)
        {
            sscanf(str, "%lf", &(vec_2[k]));
            k++;
        }
        fclose(inputFile2);

        // Start the timer after reading in the two arrays
        starttime = MPI_Wtime(); // don't time the array getting populated
    }

    
    // all processes call MPI scatter to get partitions of input vectors from process 0
    MPI_Scatter(vec_1, vec_size / comm_sz, MPI_DOUBLE, local_vec_1, vec_size / comm_sz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(vec_2, vec_size / comm_sz, MPI_DOUBLE, local_vec_2, vec_size / comm_sz, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // each process computes local dot products
    double local_dot_product = 0;
    for (int i = 0; i < vec_size / comm_sz; i++)
    {
        local_dot_product += local_vec_1[i] * local_vec_2[i];
    }

    // MPI reduce local_dot_product using addition
    double final_dot_product = 0; // to receive reduction output
    if (my_rank == 0)
    {
        MPI_Reduce(&local_dot_product, &final_dot_product, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        // stop the timer after the dot product is computed
        endtime = MPI_Wtime();
        double elapsed = endtime - starttime;

        // Output files
        FILE *outputFile = fopen(argv[4], "w");
        FILE *timeFile = fopen(argv[5], "w");

        // process 0 prints final dot product
        fprintf(outputFile, "%lf", final_dot_product);
        fprintf(timeFile, "%.20f", elapsed);

        // Cleanup
        fclose(outputFile);
        fclose(timeFile);

        // free original vectors
        free(vec_1);
        free(vec_2);
    }
    else
    {
        MPI_Reduce(&local_dot_product, &final_dot_product, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    free(local_vec_1);
    free(local_vec_2);

    return 0;
}