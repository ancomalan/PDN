#include <cuda_runtime_api.h>
#include <curand_kernel.h>
#include <driver_types.h>
#include <curand.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <cstdio>
#include <cuda.h>

// to activate debug statements
#define DEBUG 1

// program constants
#define BLOCK_SIZE 1024
#define SEED 123

// solution constants
#define MAX 123123123
#define TARGET 20

#include "support.h"
#include "hash_kernel.cu"
#include "nonce_kernel.cu"
#include "reduction_kernel.cu"

// functions used
unsigned int generate_hash(unsigned int nonce, unsigned int index, unsigned int *transactions, unsigned int n_transactions);
void read_file(char *file, unsigned int *transactions, unsigned int n_transactions);
void err_check(cudaError_t ret, char *msg, int exit_code);

/* Main ------------------ //
 *   This is the main program.
 */
int main(int argc, char *argv[])
{

    // Catch console errors
    if (argc != 6)
    {
        printf("USE LIKE THIS: gpu_mining transactions.csv n_transactions trials out.csv time.csv\n");
        return EXIT_FAILURE;
    }

    // Output files
    FILE *output_file = fopen(argv[4], "w");
    FILE *time_file = fopen(argv[5], "w");

    // Read in the transactions
    unsigned int n_transactions = strtoul(argv[2], NULL, 10);
    unsigned int *transactions = (unsigned int *)calloc(n_transactions, sizeof(unsigned int));
    read_file(argv[1], transactions, n_transactions);

    // get the number of trials
    unsigned int trials = strtoul(argv[3], NULL, 10);

    // -------- Start Mining ------------------------------------------------------- //
    // ----------------------------------------------------------------------------- //

    // Set timer and cuda error return
    Timer timer;
    startTime(&timer);
    cudaError_t cuda_ret;

    // To use with kernels
    int num_blocks = ceil((float)trials / (float)BLOCK_SIZE);
    dim3 dimGrid(num_blocks, 1, 1);
    dim3 dimBlock(BLOCK_SIZE, 1, 1);

    // ------ Step 1: generate the nonce values ------ //

    // Allocate the nonce device memory
    unsigned int *device_nonce_array;
    cuda_ret = cudaMalloc((void **)&device_nonce_array, trials * sizeof(unsigned int));
    err_check(cuda_ret, (char *)"Unable to allocate nonces to device memory!", 1);

    // Launch the nonce kernel
    nonce_kernel<<<dimGrid, dimBlock>>>(
        device_nonce_array, // put nonces into here
        trials,             // size of array
        MAX,                // to mod with
        SEED                // random seed
    );
    cuda_ret = cudaDeviceSynchronize();
    err_check(cuda_ret, (char *)"Unable to launch nonce kernel!", 2);

    // Copy the transaction array from system memory to the device memory
    unsigned int *device_transaction_array;
    cuda_ret = cudaMalloc((void **)&device_transaction_array, n_transactions * sizeof(unsigned int)); // allocate memory on device to store the transaction array
    err_check(cuda_ret, (char *)"Unable to allocate transactions to device memory!", 1);
    cuda_ret = cudaMemcpy(device_transaction_array, transactions, n_transactions * sizeof(unsigned int), cudaMemcpyHostToDevice);
    err_check(cuda_ret, (char *)"Unable to copy transactions to device!", 1);

    // Allocate the hash value array on the device memory
    unsigned int *device_hash_array;
    cuda_ret = cudaMalloc((void **)&device_hash_array, trials * sizeof(unsigned int));
    err_check(cuda_ret, (char *)"Unable to allocate hash array on device memory!", 1);

    // ------ Step 2: Generate the hash values ------ //

    // TODO Problem 1: perform this hash generation in the GPU
    // Hint: You need both nonces and transactions to compute a hash.

    // launch hash kernel to compute hash value array
    hash_kernel<<<dimGrid, dimBlock>>>(
        device_hash_array,  // GPU stores computed hashes here
        device_nonce_array, // use nonce array stored on device memory
        trials,             // hash array size
        device_transaction_array,
        n_transactions,
        MAX);

    cuda_ret = cudaDeviceSynchronize(); // wait until all threads are done computing hashes before copying back to system memory
    err_check(cuda_ret, (char *)"Unable to launch hash kernel!", 2);

    // Free memory
    free(transactions);

    // ------ Step 3: Find the nonce with the minimum hash value ------ //

    // TODO Problem 2: find the minimum in the GPU by reduction
    // Keep both the nonce value array and the hash value array on the device memory

    unsigned int reduction_num_blocks = ceil(trials / (float)(2.0 * BLOCK_SIZE)); // number of blocks needed in reduction kernel (each block of threads handles 2 blocks of elements)

    // allocate output arrays on device memory to store local values
    unsigned int *device_local_min_hash;
    cuda_ret = cudaMalloc((void **)&device_local_min_hash, reduction_num_blocks * sizeof(unsigned int));
    err_check(cuda_ret, (char *)"Unable to allocate local min hash array on device memory!", 1);

    unsigned int *device_local_min_nonce;
    cuda_ret = cudaMalloc((void **)&device_local_min_nonce, reduction_num_blocks * sizeof(unsigned int));
    err_check(cuda_ret, (char *)"Unable to allocate local min nonce array on device memory!", 1);

    // launch reduction kernel to find min hash and min nonce values
    reduction_kernel<<<reduction_num_blocks, dimBlock>>>(
        device_local_min_hash,  // will get populated with min hashes for each block
        device_local_min_nonce, // will get populated with min nonces for each block
        device_hash_array,      // input hash array currently on device
        device_nonce_array,     // input nonce array currently on device
        trials                  // size of hash array/nonce array
    );

    // each block in reduction kernel computes a local min and nonce which are stored in these following arrays
    unsigned int *system_local_min_hash = (unsigned int *)calloc(reduction_num_blocks, sizeof(unsigned int));
    unsigned int *system_local_min_nonce = (unsigned int *)calloc(reduction_num_blocks, sizeof(unsigned int));

    // Copy output arrays on device memory to local arrays on system
    cuda_ret = cudaMemcpy(system_local_min_hash, device_local_min_hash, reduction_num_blocks * sizeof(unsigned int), cudaMemcpyDeviceToHost);
    err_check(cuda_ret, (char *)"Unable to copy local min hash array from device!", 1);
    cuda_ret = cudaMemcpy(system_local_min_nonce, device_local_min_nonce, reduction_num_blocks * sizeof(unsigned int), cudaMemcpyDeviceToHost);
    err_check(cuda_ret, (char *)"Unable to copy local min nonce array from device!", 1);

    // Find the global min hash values and min nonce values serially using the CPU.
    unsigned int min_hash = MAX;
    unsigned int min_nonce = MAX;
    for (int i = 0; i < reduction_num_blocks; i++)
    {
        if (system_local_min_hash[i] < min_hash)
        {
            min_hash = system_local_min_hash[i];
            min_nonce = system_local_min_nonce[i];
        }
    }

    // Free memory
    cudaFree(device_nonce_array);
    cudaFree(device_transaction_array);
    cudaFree(device_hash_array);
    cudaFree(device_local_min_hash);
    cudaFree(device_local_min_nonce);
    stopTime(&timer);
    // ----------------------------------------------------------------------------- //
    // -------- Finish Mining ------------------------------------------------------ //

    // Get if suceeded
    char *res = (char *)malloc(8 * sizeof(char));
    if (min_hash < TARGET)
        res = (char *)"Success!";
    else
        res = (char *)"Failure.";

    // Show results in console
    if (DEBUG)
        printf("%s\n   Min hash:  %u\n   Min nonce: %u\n   %f seconds\n",
               res,
               min_hash,
               min_nonce,
               elapsedTime(timer));

    // Print results
    fprintf(output_file, "%s\n%u\n%u\n", res, min_hash, min_nonce);
    fprintf(time_file, "%f\n", elapsedTime(timer));

    // Cleanup
    fclose(time_file);
    fclose(output_file);

    return 0;
} // End Main -------------------------------------------- //

/* Read File -------------------- //
 *   Reads in a file of transactions.
 */
void read_file(char *file, unsigned int *transactions, unsigned int n_transactions)
{

    // open file
    FILE *trans_file = fopen(file, "r");
    if (trans_file == NULL)
        fprintf(stderr, "ERROR: could not read the transaction file.\n"),
            exit(-1);

    // read items
    char line[100] = {0};
    for (int i = 0; i < n_transactions && fgets(line, 100, trans_file); ++i)
    {
        char *p;
        transactions[i] = strtof(line, &p);
    }

    fclose(trans_file);

} // End Read File ------------- //

/* Error Check ----------------- //
 *   Exits if there is a CUDA error.
 */
void err_check(cudaError_t ret, char *msg, int exit_code)
{
    if (ret != cudaSuccess)
        fprintf(stderr, "%s \"%s\".\n", msg, cudaGetErrorString(ret)),
            exit(exit_code);
} // End Error Check ----------- //
