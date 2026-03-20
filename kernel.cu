// one thread works on one output pixel
__global__ void kernel(int *outputMatrix_d, int *inputMatrix_d, int *filterMatrix_d, int width, int height)
{

    // Performing convolution
    // each thread gets its global column and row
    int Col = blockIdx.x * blockDim.x + threadIdx.x;
    int Row = blockIdx.y * blockDim.y + threadIdx.y;
    int sum_val = 0;

    // check if thread working on valid Col and Row
    if (Col < width && Row < height)
    {
        // nested for loops to get 24 "neighbors"
        for (int blurRow = -BLUR_SIZE; blurRow < BLUR_SIZE + 1; ++blurRow)
        {
            for (int blurCol = -BLUR_SIZE; blurCol < BLUR_SIZE + 1; ++blurCol)
            {
                int curRow = Row + blurRow;
                int curCol = Col + blurCol;

                int i_row = blurRow + BLUR_SIZE;
                int i_col = blurCol + BLUR_SIZE;

                if (curRow > -1 && curRow < height && curCol > -1 && curCol < width)
                {
                    sum_val += inputMatrix_d[curRow * width + curCol] * filterMatrix_d[i_row * 5 + i_col];
                }
            }
        }

        outputMatrix_d[Row * width + Col] = sum_val; // thread updates its corresponding element in the output array
    }
}
