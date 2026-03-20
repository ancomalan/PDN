/* Generate Hash kernel ----------------------------------------- //
 *   Generates a hash value from a nonce and transaction list.
 */
__device__ // (that can only be called on the device)
    unsigned int generate_hash(unsigned int nonce, unsigned int index, unsigned int *transactions, unsigned int n_transactions, unsigned int mod)
{

    unsigned int hash = (nonce + transactions[0] * (index + 1)) % mod;
    for (int j = 1; j < n_transactions; j++)
    {
        hash = (hash + transactions[j] * (index + 1)) % mod;
    }
    return hash;

} // End Generate Hash ---------- //

/* Hash Kernel --------------------------------------
 *       Generates an array of hash values from nonces.
 */
__global__ // can be called from host
    void hash_kernel(unsigned int *hash_array, unsigned int *nonce_array, unsigned int array_size, unsigned int *transactions, unsigned int n_transactions, unsigned int mod)
{

    // Calculate thread index
    unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;

    // TODO: Generate hash values
    // each thread computes a hash for one trial using its thread index (no for loop)
    // boundary check
    if (index < array_size)
    {
        hash_array[index] = generate_hash(nonce_array[index], index, transactions, n_transactions, mod);
    }

} // End Hash Kernel //
