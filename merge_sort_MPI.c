#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "mpi.h"

#define MAXLINE 25
#define DEBUG 0

// to read in file
float *read_input(FILE *inputFile, int n_items);
int cmpfloat(const void *a, const void *b);
float *merge(float *A, float *B, int new_size);
int compute_partner(int my_rank, int num_processes, int stride_size);

/* Main Program -------------- */
int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        printf("USE LIKE THIS: merge_sort_MPI n_items input.csv output.csv time.csv\n");
        return EXIT_FAILURE;
    }

    // input file and size
    FILE *inputFile = fopen(argv[2], "r");
    int n_items = strtol(argv[1], NULL, 10);

    // Start MPI
    int my_rank, comm_size;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    // arrays to use
    // TODO: initialize your arrays here
    float *og_array = NULL;                                     // dangling pointer (all processes must declare)
    int local_n_items = n_items / comm_size;                    // num elements in local array
    float *local_array = malloc(local_n_items * sizeof(float)); // each process allocates local array
    // Read a global array from an input file using process 0
    if (my_rank == 0)
    {
        og_array = read_input(inputFile, n_items); // only process 0 allocates array
    }

    // get start time
    double local_start, local_finish, local_elapsed, elapsed;
    MPI_Barrier(MPI_COMM_WORLD);
    local_start = MPI_Wtime();

    // TODO: implement solution here

    // Scatter the global array across all processes
    MPI_Scatter(og_array, local_n_items, MPI_FLOAT, local_array, n_items / comm_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Sort the local arrays by every process using the build-in quick sort function (qsort)
    qsort(local_array, local_n_items, sizeof(local_array[0]), cmpfloat);

    // Reduce the local sorted arrays on all processes to a global sorted array on process 0
    int num_processes = comm_size; // shrinks by factor of 2 each iteration
    for (int stride_size = num_processes / 2; stride_size >= 1; stride_size /= 2)
    {
        // process gets partner
        int partner = compute_partner(my_rank, num_processes, stride_size);
        num_processes /= 2;

        // if receiving process
        if (my_rank < stride_size)
        {
            float *partner_local_array = malloc(local_n_items * sizeof(float));                                     // array for partner's local array
            MPI_Recv(partner_local_array, local_n_items, MPI_FLOAT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // call mpi receive to get sending process's local array
            local_n_items *= 2;                                                                                     // every iteration, shall receive 2x amount of elements from sending partner

            // combine two local sorted arrays to a larger sorted array in each reduction operation
            local_array = merge(local_array, partner_local_array, local_n_items); // returns pointer to new sorted array with elements from both local arrays
            free(partner_local_array);
        }
        // sender process sends local array once and they are done (breaks)
        else
        {
            MPI_Send(local_array, local_n_items, MPI_FLOAT, partner, 0, MPI_COMM_WORLD);
            break;
        }
    }

    // get elapsed time
    local_finish = MPI_Wtime();
    local_elapsed = local_finish - local_start;

    // send time to main process
    MPI_Reduce(
        &local_elapsed,
        &elapsed,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD);

    // Write output (Step 5)
    if (my_rank == 0)
    {
        FILE *outputFile = fopen(argv[3], "w");
        FILE *timeFile = fopen(argv[4], "w");

        // TODO: output
        // Write the global sorted array to an output file using process 0
        for (int i = 0; i < n_items; i++)
        {
            fprintf(outputFile, "%f\n", local_array[i]);
        }
        fprintf(timeFile, "%f\n", elapsed);

        fclose(outputFile);
        fclose(timeFile);
    }

    MPI_Finalize();
    free(og_array);
    free(local_array);
    if (DEBUG)
        printf("Finished!\n");
    return 0;
} // End Main //

// merges 2 sorted lists based on psuedocode from wikipedia
float *merge(float *A, float *B, int new_size)
{
    int old_size = new_size / 2; // array size of A and B
    float *C = malloc(new_size * sizeof(float));

    int a_idx = 0, b_idx = 0, c_idx = 0; // once a_idx or b_idx = new_size/2 (old size) -> empty!
    // increment indexes to "drop head"
    while (a_idx != old_size && b_idx != old_size)
    {
        if (A[a_idx] <= B[b_idx])
        {

            C[c_idx] = A[a_idx]; // append head of A to C
            c_idx++;             // update idx position of C
            a_idx++;             // drop head of A
        }
        else
        {
            C[c_idx] = B[b_idx]; // append head of B to C
            c_idx++;
            b_idx++; // drop head of B
        }
    }
    // at this point, either A or B is empty. Thus empty the other input list
    while (a_idx != old_size)
    {
        C[c_idx] = A[a_idx]; // append head of A to C
        c_idx++;
        a_idx++; // drop head of A
    }
    while (b_idx != old_size)
    {
        C[c_idx] = B[b_idx]; // append head of B to C
        c_idx++;
        b_idx++; // drop head of B
    }

    free(A); // free old local_array for receiving process
    return C;
}

// function for calculating process's partner (based on diagram from assignment)
int compute_partner(int my_rank, int num_processes, int stride_size)
{
    // processes that receive
    if (my_rank < num_processes / 2)
    {
        return my_rank + stride_size;
    }
    else
    {
        return my_rank - stride_size; // partner for sending processes
    }
}

/* Read Input -------------------- */
float *read_input(FILE *inputFile, int n_items)
{
    float *arr = (float *)malloc(n_items * sizeof(float));
    char line[MAXLINE] = {0};
    int i = 0;
    // char *ptr;
    while (fgets(line, MAXLINE, inputFile))
    {
        sscanf(line, "%f", &(arr[i]));
        ++i;
    }
    return arr;
} // Read Input //

/* Cmp Int ----------------------------- */
// use this for qsort
// source: https://stackoverflow.com/questions/3886446/problem-trying-to-use-the-c-qsort-function
int cmpfloat(const void *a, const void *b)
{
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    return (fa > fb) - (fa < fb);
} // Cmp Int //
