#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define BILLION 1000000000.0
#define MAX_LINE_LENGTH 25000

#define BLUR_SIZE 2
#include "kernel.cu"

int main(int argc, char *argv[])
{
    // Check console errors
    if (argc != 6)
    {
        printf("USE LIKE THIS: convolution_serial n_row n_col mat_input.csv mat_output.csv time.csv\n");
        return EXIT_FAILURE;
    }

    // Get dims
    int n_row = strtol(argv[1], NULL, 10);
    int n_col = strtol(argv[2], NULL, 10);

    // Get files to read/write
    FILE *inputFile1 = fopen(argv[3], "r");
    if (inputFile1 == NULL)
    {
        printf("Could not open file %s", argv[2]);
        return EXIT_FAILURE;
    }
    FILE *outputFile = fopen(argv[4], "w");
    FILE *timeFile = fopen(argv[5], "w");

    // Matrices to use
    int *filterMatrix_h = (int *)malloc(5 * 5 * sizeof(int));
    int *inputMatrix_h = (int *)malloc(n_row * n_col * sizeof(int));
    int *outputMatrix_h = (int *)malloc(n_row * n_col * sizeof(int));

    // read the data from the file
    int row_count = 0;
    char line[MAX_LINE_LENGTH] = {0};
    while (fgets(line, MAX_LINE_LENGTH, inputFile1))
    {
        if (line[strlen(line) - 1] != '\n')
            printf("\n");
        char *token;
        const char s[2] = ",";
        token = strtok(line, s);
        int i_col = 0;
        while (token != NULL)
        {
            inputMatrix_h[row_count * n_col + i_col] = strtol(token, NULL, 10);
            i_col++;
            token = strtok(NULL, s);
        }
        row_count++;
    }

    // Filling filter
    // 1 0 0 0 1
    // 0 1 0 1 0
    // 0 0 1 0 0
    // 0 1 0 1 0
    // 1 0 0 0 1
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++)
            filterMatrix_h[i * 5 + j] = 0;

    filterMatrix_h[0 * 5 + 0] = 1;
    filterMatrix_h[1 * 5 + 1] = 1;
    filterMatrix_h[2 * 5 + 2] = 1;
    filterMatrix_h[3 * 5 + 3] = 1;
    filterMatrix_h[4 * 5 + 4] = 1;

    filterMatrix_h[4 * 5 + 0] = 1;
    filterMatrix_h[3 * 5 + 1] = 1;
    filterMatrix_h[1 * 5 + 3] = 1;
    filterMatrix_h[0 * 5 + 4] = 1;

    fclose(inputFile1);

    // --------------------------------------------------------------------------- //
    // ------ Algorithm Start ---------------------------------------------------- //

    // allocate memory on device for input, output, and filter arrays
    int *outputMatrix_d;
    cudaMalloc((void **)&outputMatrix_d, (n_row * n_col) * sizeof(int));
    int *inputMatrix_d;
    cudaMalloc((void **)&inputMatrix_d, (n_row * n_col) * sizeof(int));
    int *filterMatrix_d;
    cudaMalloc((void **)&filterMatrix_d, 5 * 5 * sizeof(int));

    dim3 dimBlock(16, 16);                                          // 2D block with 16 threads in x and y directions (256 threads per block)
    dim3 dimGrid(ceil(n_col / (float)16), ceil(n_row / (float)16)); // number of blocks in x and y direction (in this case, 8 blocks on both x and y)

    struct timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start);
    // 1. Transfer the input image (the A matrix) to the device memory
    cudaMemcpy(inputMatrix_d, inputMatrix_h, (n_row * n_col) * sizeof(int), cudaMemcpyHostToDevice); // copy input image from host to device
    // 2. Transfer the convolution filter (the K matrix) to the device memory
    cudaMemcpy(filterMatrix_d, filterMatrix_h, 5 * 5 * sizeof(int), cudaMemcpyHostToDevice);
    clock_gettime(CLOCK_REALTIME, &end);
    double time_host_to_device = (end.tv_sec - start.tv_sec) +
                                 (end.tv_nsec - start.tv_nsec) / BILLION;

    // 3. Launch the convolution kernel to compute the filter map (the B matrix) by applying the convolution to every pixel in the input image.
    clock_gettime(CLOCK_REALTIME, &start);
    kernel<<<dimGrid, dimBlock>>>(outputMatrix_d, inputMatrix_d, filterMatrix_d, n_col, n_row);
    cudaDeviceSynchronize(); // synchronization barrier since host thread returns after kernel launch
    clock_gettime(CLOCK_REALTIME, &end);
    double time_kernel = (end.tv_sec - start.tv_sec) +
                         (end.tv_nsec - start.tv_nsec) / BILLION;

    clock_gettime(CLOCK_REALTIME, &start);
    // 4. Transfer the filter map (the B matrix) from the device memory to the system memory.
    cudaMemcpy(outputMatrix_h, outputMatrix_d, (n_row * n_col) * sizeof(int), cudaMemcpyDeviceToHost);
    clock_gettime(CLOCK_REALTIME, &end);
    double time_device_to_host = (end.tv_sec - start.tv_sec) +
                                 (end.tv_nsec - start.tv_nsec) / BILLION;

    // --------------------------------------------------------------------------- //
    // ------ Algorithm End ------------------------------------------------------ //

    // Save output matrix as csv file
    for (int i = 0; i < n_row; i++)
    {
        for (int j = 0; j < n_col; j++)
        {
            fprintf(outputFile, "%d", outputMatrix_h[i * n_col + j]);
            if (j != n_col - 1)
                fprintf(outputFile, ",");
            else if (i < n_row - 1)
                fprintf(outputFile, "\n");
        }
    }

    // Print time
    fprintf(timeFile, "%.20f\n", time_host_to_device);
    fprintf(timeFile, "%.20f\n", time_kernel);
    fprintf(timeFile, "%.20f\n", time_device_to_host);

    // Cleanup
    fclose(outputFile);
    fclose(timeFile);

    free(inputMatrix_h);
    free(outputMatrix_h);
    free(filterMatrix_h);

    cudaFree(inputMatrix_d);
    cudaFree(outputMatrix_d);
    cudaFree(filterMatrix_d);
    return 0;
}