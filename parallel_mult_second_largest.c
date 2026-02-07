#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define DEBUG 0

/* ----------- Project 2 - Problem 2B -----------

    This file will multiply two matricies and find second maximum element using serial reduction
*/
// ------------------------------------------------------ //

int main(int argc, char *argv[])
{
    // Catch console errors
    if (argc != 10)
    {
        printf("USE LIKE THIS: parallel_mult_second_largest file_A.csv n_row_A n_col_A file_B.csv n_row_B n_col_B result_matrix.csv time.csv num_threads \n");
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
    // long int *C = (long int *)malloc((n_row1 * n_col2) * sizeof(long int)); // matrix C has dimension (n_row1 x n_col2)

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

    // TODO: Parallelize the matrix-matrix multiplication and find the max element

    // got idea from Google Gemini to use two dynamic arrays as shared variables (local solutions indexed by thread id)
    // learned from Gemini that both the largest and second largest elements from each thread must be considered to determine global second largest
    long int *localLargest = (long int *)malloc(thread_count * sizeof(long int));
    long int *localSecondLargest = (long int *)malloc(thread_count * sizeof(long int));

#pragma omp parallel num_threads(thread_count) // enter parallel region with specified number of threads
    {
        long int largest = 0;                   // tracks largest value for this thread
        long int secondLargest = 0;             // tracks second largest value for this thread
        int thread_rank = omp_get_thread_num(); // get thread id to store into shared arrays

// #pragma omp for: distributes columns (outer loop iterations) in B between the threads
// each thread computes a batch of independent column vectors for C
#pragma omp for
        for (long int b = 0; b < n_col2; b++) // for each column vector in B
        {
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

                // update local largest and secondLargest
                if (dotProduct > largest)
                {
                    secondLargest = largest; // second largest becomes old largest
                    largest = dotProduct;    // assign current value as new largest
                }
                else if (dotProduct > secondLargest)
                {
                    secondLargest = dotProduct; // update second largest
                }
            }
        }
        // write final results for each thread into arrays (using its rank/thread id)
        localLargest[thread_rank] = largest;
        localSecondLargest[thread_rank] = secondLargest;
    }

    // serial reduction: sequentially compare local solutions in shared arrays to determine globalSecondLargest
    long int globalLargest = 0, globalSecondLargest = 0;
    for (int i = 0; i < thread_count; i++)
    {
        // first, check local solution in localLargest
        if (localLargest[i] > globalLargest)
        {
            globalSecondLargest = globalLargest;
            globalLargest = localLargest[i];
        }
        else if (localLargest[i] > globalSecondLargest)
        {
            globalSecondLargest = localLargest[i];
        }

        // then, check if local solution of localSecondLargest is greater than globalSecondLargest
        // localSecondLargest[i] can NOT be bigger than localLargest[i], so we can skip this check
        if (localSecondLargest[i] > globalSecondLargest)
        {
            globalSecondLargest = localSecondLargest[i]; // set new globalSecondLargest
        }
    }

    // Record the finish time
    double end = omp_get_wtime();

    // Time calculation (in seconds)
    double time_passed = end - start;

    // Save time to file
    fprintf(outputTime, "%f", time_passed);

    // TODO: save the second largest element in C to the output csv file
    fprintf(outputFile, "%ld", globalSecondLargest);

    // free memory
    free(A);
    free(B);
    free(localLargest);
    free(localSecondLargest);

    // Cleanup
    fclose(inputMatrix1);
    fclose(inputMatrix2);
    fclose(outputFile);
    fclose(outputTime);
    // Remember to free your buffers!

    return 0;
}
