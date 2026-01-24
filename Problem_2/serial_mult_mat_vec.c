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
    int n_row1 = strtol(argv[2], &p1, 10);
    int n_col1 = strtol(argv[3], &p2, 10);

    // Get dim of the vector
    char *p3;
    int n_row2 = strtol(argv[5], &p3, 10);

    // Get the output file
    FILE *outputFile = fopen(argv[6], "w");

    // TODO: Use malloc to allocate memory for the matrices
    int *inputMatrix = malloc((n_row1 * n_col1) * sizeof(int)); // input matrix
    int *inputVector = malloc(n_row2 * sizeof(int));            // input vector
    int *outputVector = malloc(n_row1 * sizeof(int));           // output vector has dim (n_row1 x 1)

    // TODO: Parse the input CSV files
    // used to keep track of where to insert element as we get integer from files
    int inputMatrixIndex = 0, inputVectorIndex = 0;

    // write values from matFile into inputMatrix
    // fscanf will return EOF when end of file is reached, and stop when reading new line
    // %d, handles int followed by comma pattern in csv file
    // in while condition, fscanf reads from file and inserts into array using &inputMatrix[i] as address of element at index i
    while (fscanf(matFile, "%d,", &inputMatrix[inputMatrixIndex]) != EOF)
    {
        inputMatrixIndex++; // move on to next array index
    }

    // write values from vecFile into inputVector in same manner as above
    while (fscanf(vecFile, "%d,", &inputVector[inputVectorIndex]) != EOF)
    {
        inputVectorIndex++; // update index to know where to insert next integer into the array
    }

    // TODO: Perform the matrix-vector multiplication

    // TODO: Write the output CSV file
    // use fputs(outputVector[i] /n, outputFile)
    // for (int i = 0; i < n_row1; i++){
    //     fprintf(outputFile, "%d\n", outputVector[i]);// write each element in row of outputVector to output file
    // }

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
