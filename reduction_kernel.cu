// array on device memory is slow to access so use shared memory
// perform reduction in shared memory, then copy back to device

__global__ void reduction_kernel(unsigned int *device_min_hashes, unsigned int *device_min_nonces,
                                 unsigned int *device_hash_array, unsigned int *device_nonce_array, unsigned int size)
{

    int index = 2 * blockIdx.x * blockDim.x + threadIdx.x; // global rank of this thread (each block of threads handles 2 blocks of elements)

    // declare reduction arrays in shared memory for local min hashes and nonces (shared with all other threads in block)
    __shared__ unsigned int hash_reduction[BLOCK_SIZE];
    __shared__ unsigned int nonce_reduction[BLOCK_SIZE];

    // block of threads loads an array of blocksize from device memory to shared memory
    // if thread has valid element, populate block's reduction array
    if (index < size)
    {
        hash_reduction[threadIdx.x] = device_hash_array[index];
        nonce_reduction[threadIdx.x] = device_nonce_array[index];
    }
    else
    {
        // fill out of bounds indices
        hash_reduction[threadIdx.x] = UINT_MAX; // initialization value for min of unsigned int
        nonce_reduction[threadIdx.x] = 0;
    }

    // if possible, find minimum between this thread and its corresponding element in the next block (completes first level of tree reduction)
    if ((index + BLOCK_SIZE) < size)
    {
        // get the minimum hash between the pair, while keeping track of the corresponding nonce value
        if (hash_reduction[threadIdx.x] > device_hash_array[index + BLOCK_SIZE])
        {
            hash_reduction[threadIdx.x] = device_hash_array[index + BLOCK_SIZE];   // element block size away is smaller
            nonce_reduction[threadIdx.x] = device_nonce_array[index + BLOCK_SIZE]; // keep track of the nonce value
        }
    }

    // block of threads reduce the reduction array to a scalar
    // use sequential addressing to help with control divergence
    for (int stride = BLOCK_SIZE / 2; stride >= 1; stride /= 2)
    {
        __syncthreads(); // ensure that threads combine updated values after every iteration
        if (threadIdx.x < stride)
        {
            // get the minimum hash between this thread and corresponding element stride size away, while keeping track of the corresponding nonce value
            if (hash_reduction[threadIdx.x] > hash_reduction[threadIdx.x + stride])
            {
                hash_reduction[threadIdx.x] = hash_reduction[threadIdx.x + stride];   // element block size away is smaller
                nonce_reduction[threadIdx.x] = nonce_reduction[threadIdx.x + stride]; // keep track of the nonce value
            }
        }
    }

    // thread 0 in this block copies local minimum hash & nonce of reduction arrays into device memory
    if (threadIdx.x == 0)
    {
        device_min_hashes[blockIdx.x] = hash_reduction[0];
        device_min_nonces[blockIdx.x] = nonce_reduction[0];
    }
}