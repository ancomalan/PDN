// Alan Vo
// OU Spring 2026
// PDN: project 1, problem 2

#include <stdio.h>
#include <stdlib.h>
// Use more libraries as necessary

#define DEBUG 0

/* ---------- Project 1 - Problem 2 - Mat-Vec Mult ----------
    This file will multiply a matrix and vector.
    Complete the TODOs left in this file.
*/
// ------------------------------------------------------ //

int main(int argc, char *argv[])
{
    // Catch console errors
    if (argc != 7)
    {
        printf("USE LIKE THIS: serial_mult_mat_vec in_mat.csv n_row_1 n_col_1 in_vec.csv n_row_2 output_file.csv \n");
        return EXIT_FAILURE;
    }

    // Get the input files
    FILE *matFile = fopen(argv[1], "r");
    FILE *vecFile = fopen(argv[4], "r");

    // Get dim of the matrix
    char *p1;
    char *p2;
    long int n_row1 = strtol(argv[2], &p1, 10);
    long int n_col1 = strtol(argv[3], &p2, 10);

    // Get dim of the vector
    char *p3;
    long int n_row2 = strtol(argv[5], &p3, 10);

    // Get the output file
    FILE *outputFile = fopen(argv[6], "w");

    // TODO: Use malloc to allocate memory for the matrices
    long int* inputMatrix = (long int*)malloc((n_row1 * n_col1) * sizeof(long int)); // typecast since malloc returns void pointer
    long int* inputVector = (long int*)malloc(n_row2 * sizeof(long int));
    long int* outputVector = (long int*)malloc(n_row1 * sizeof(long int)); // output vector has dim (n_row1 x 1)

    // TODO: Parse the input CSV files
    // used to keep track of where to insert integers as they are parsed from files
    long int inputMatrixIndex = 0, inputVectorIndex = 0;

    // write values from matFile into inputMatrix in row-major order
    // fscanf returns EOF when end of file is reached, and stops when reading new line
    // %ld, handles the long int followed by comma pattern present in csv file
    // in while condition, fscanf reads from file AND uses &inputMatrix[i] (address of element at index i) to "insert" into array
    while (fscanf(matFile, "%ld,", &inputMatrix[inputMatrixIndex]) != EOF)
    {
        inputMatrixIndex++; // move on to next array index
    }

    // write values from vecFile into inputVector in same manner as above
    while (fscanf(vecFile, "%ld,", &inputVector[inputVectorIndex]) != EOF)
    {
        inputVectorIndex++; // update index to know where to insert next integer into the array
    }

    // TODO: Perform the matrix-vector multiplication
    // go through each row of input matrix
    for (long int i = 0; i < n_row1; i++)
    {
        long int dotProduct = 0; // running total that will be written to corresponding spot in output vector
        // for each row, go through each column
        for (long int j = 0; j < n_col1; j++)
        {
            // perform matrix multiplication (dot product)
            // each element of current row is multiplied by corresponding element in column vector
            dotProduct += inputMatrix[i * n_col1 + j] * inputVector[j];
        }
        outputVector[i] = dotProduct; // after computing dot product, update output vector
    }

    // TODO: Write the output CSV file
    for (long int i = 0; i < n_row1; i++)
    {
        fprintf(outputFile, "%ld\n", outputVector[i]); // write each element in outputVector to output file
    }

    // TODO: Free memory
    free(inputMatrix);
    free(inputVector);
    free(outputVector);

    // Cleanup
    fclose(matFile);
    fclose(vecFile);
    fclose(outputFile);
    // Free buffers here as well!

    return 0;
}
