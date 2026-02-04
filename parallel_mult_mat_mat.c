#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define DEBUG 0

/* ----------- Project 2 - Problem 1 - Matrix Mult -----------

    This file will multiply two matricies.
    Complete the TODOs in order to complete this program.
    Remember to make it parallelized!
*/
// ------------------------------------------------------ //

int main(int argc, char *argv[])
{
    // Catch console errors
    if (argc != 10)
    {
        printf("USE LIKE THIS: parallel_mult_mat_mat file_A.csv n_row_A n_col_A file_B.csv n_row_B n_col_B result_matrix.csv time.csv num_threads \n");
        return EXIT_FAILURE;
    }

    // Get the input files
    FILE *inputMatrix1 = fopen(argv[1], "r");
    FILE *inputMatrix2 = fopen(argv[4], "r");

    char *p1;
    char *p2;

    // Get matrix 1's dims
    int n_row1 = strtol(argv[2], &p1, 10);
    int n_col1 = strtol(argv[3], &p2, 10);

    // Get matrix 2's dims
    int n_row2 = strtol(argv[5], &p1, 10);
    int n_col2 = strtol(argv[6], &p2, 10);

    // Get num threads
    int thread_count = strtol(argv[9], NULL, 10);

    // Get output files
    FILE *outputFile = fopen(argv[7], "w");
    FILE *outputTime = fopen(argv[8], "w");

    // TODO: malloc the two input matrices and the output matrix
    // Please use long int as the variable type
    long int *A = (long int *)malloc((n_row1 * n_col1) * sizeof(long int));
    long int *B = (long int *)malloc((n_row2 * n_col2) * sizeof(long int));
    long int *C = (long int *)malloc((n_row1 * n_col2) * sizeof(long int)); // matrix C has dimension (n_row1 x n_col2)

    // TODO: Parse the input csv files and fill in the input matrices
    // used to keep track of where to insert integers as they are parsed from files
    long int index_A = 0, index_B = 0;

    // write values from inputMatrix1 into matrix A in row-major order
    // fscanf returns EOF when end of file is reached, and stops when reading new line
    // %ld, handles the long int followed by comma pattern present in csv file
    // in while condition, fscanf reads from file AND uses &A[i] (address of element at index i) to "insert" into array
    while (fscanf(inputMatrix1, "%ld,", &A[index_A]) != EOF)
    {
        index_A++; // move on to next array index
    }

    // write values from  into inputMatrix2 into matrix B in same manner as above
    while (fscanf(inputMatrix2, "%ld,", &B[index_B]) != EOF)
    {
        index_B++;
    }

    // We are interesting in timing the matrix-matrix multiplication only
    // Record the start time
    double start = omp_get_wtime();

    // TODO: Parallelize the matrix-matrix multiplication
    /*for each column vector in matrix B do
        multiply matrix A with this column vector
        save the resultant column vector to matrix C*/

#pragma omp parallel num_threads(thread_count) // parallel region with specified number of threads
    {
// each thread multiplies A with their assigned column vectors in B
// #pragma omp for: distributes columns in B between the threads (each thread computes a batch of independent column vectors for C)
#pragma omp for
        for (long int b = 0; b < n_col2; b++) // for each column vector in B
        {
            /*slightly modified matrix-vector mulitplication code from project 1: now, directly index into matrix B to get
            corresponding elements of column vector, instead of indexing with j like in project 1, where vector elements were in 1d array*/

            // perform matrix-vector multiplication (dot product)
            // go through each row of A
            for (long int i = 0; i < n_row1; i++)
            {
                long int dotProduct = 0; // running total that will be written to corresponding element in C
                // for each row, go through each column of A
                for (long int j = 0; j < n_col1; j++)
                {
                    // each element of current row in A is multiplied by corresponding element in B column vector
                    dotProduct += A[i * n_col1 + j] * B[j * n_col2 + b]; // get element from current column in B
                }
                C[i * n_col2 + b] = dotProduct; // after computing dot product, update corresponding element in C
            }
        }
    }

    // Record the finish time
    double end = omp_get_wtime();

    // Time calculation (in seconds)
    double time_passed = end - start;

    // Save time to file
    fprintf(outputTime, "%f", time_passed);

    // TODO: save the output matrix to the output csv file
    // iterate through each element of C
    for (long int i = 0; i < n_row1; i++)
    {
        for (long int j = 0; j < n_col2; j++)
        {
            // if last element in row, do NOT add comma
            if (j == n_col2 - 1)
                fprintf(outputFile, "%ld", C[i * n_col2 + j]);
            // otherwise, add comma after number
            else
            {
                fprintf(outputFile, "%ld,", C[i * n_col2 + j]); // write each element in outputVector to output file
            }
        }
        fprintf(outputFile, "\n"); // newline
    }

    // free memory
    free(A);
    free(B);
    free(C);

    // Cleanup
    fclose(inputMatrix1);
    fclose(inputMatrix2);
    fclose(outputFile);
    fclose(outputTime);
    // Remember to free your buffers!

    return 0;
}
