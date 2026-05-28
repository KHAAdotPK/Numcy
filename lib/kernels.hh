/*
 * Numcy/lib/kernels.hh
 * Q@hackers.pk
 */

//  https://share.google/aimode/cnwoyypLemleG7Nha
//  https://share.google/aimode/OHMIjYZE1HWdEVwZu

#ifndef NUMCY_CUDA_KERNELS_HH
#define NUMCY_CUDA_KERNELS_HH

#ifdef COMPILE_FOR_DEVICE 

/*
__global__ void randn_kernel(double* data, curandState* states, size_t n)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n)
    {
        curandState state = states[idx];
        data[idx] = curand_normal_double(&state);
        states[idx] = state;
    }
}
 */

/*
    "__global__": This qualifier tells the compiler that this function is a "CUDA kernel".
                  It means the function is called from the Host (CPU) but executed on the Device (GPU). 
    "size_t idx = ...": This is the most crucial part. Since every thread is running the same code,
                        it needs a way to figure out *which element* in the `data` array it is responsible for.

                        CUDA provides built-in variables to help
                        ----------------------------------------
    CUDA provides these automatically to every thread; you never set them, you just read them ...
    blockIdx.x: which block. Which block this thread is in | CUDA runtime | ... (e.g., Block 0, 1, 2, or 3)
    blockDim.x: block size. How many threads per block | CUDA runtime via <<<blocks, threads_per_block>>> | ... (e.g., 256)
    threadIdx.x: which thread in block. Which thread this is inside its block | CUDA runtime | ... (e.g.,Thread 0, 1, 2, ... 255)
    gridDim.x: grid size. How many blocks in total | CUDA runtime | ... How many blocks are there in total? (e.g., 4)
 */


/*
    The Kernel Launch Syntax (`<<< ... >>>`)
    Once the grid dimensions (blocks, threads_per_block) are calculated, the kernel is dispatched to the GPU using the triple-chevron syntax `<<<blocks, threads>>>`:
    "<<< ... >>>": This is the CUDA execution configuration syntax.
                   It essentially tells the GPU runtime, "Hey, run the setup_curand_kernel function on the GPU,
                   and organize the execution using this many blocks, with this many threads per block."
 */
// Naive transpose: one thread per output element
// rows/cols refer to the INPUT matrix dimensions
template <typename T = double, typename E = size_t>
__global__ void transpose_kernel(const T* input, T* output, E rows, E cols)
{
    E idx = blockIdx.x * blockDim.x + threadIdx.x;
    E total = rows * cols;

    if (idx < total)
    {
        // Which row and col in the INPUT does this thread handle?
        E row = idx / cols; // Vertically down the columns
        E col = idx % cols; // Horizontally across the rows to the next column

        // Write to transposed position in OUTPUT
        // input[row][col] → output[col][row]
        output[col * rows + row] = input[row * cols + col];
        //     ^which       ^which         ^which       ^which
        //      vertical     horizontal    horizontal   vertical
    }
}

/*
    The Kernel Launch Syntax (`<<< ... >>>`)
    Once the grid dimensions (blocks, threads_per_block) are calculated, the kernel is dispatched to the GPU using the triple-chevron syntax `<<<blocks, threads>>>`:
    "<<< ... >>>": This is the CUDA execution configuration syntax.
                   It essentially tells the GPU runtime, "Hey, run the setup_curand_kernel function on the GPU,
                   and organize the execution using this many blocks, with this many threads per block."
 */
// The cuRAND state setup kernel
// One thread per element, each thread initializes its own curandState
__global__ void setup_curand_kernel(curandState* states, unsigned long seed, size_t n)
{
    /*
        "size_t idx = ...": This is the most crucial part. Since every thread is running the same code,
                            it needs a way to figure out *which element* in the `data` array it is responsible for.
                            CUDA provides built-in variables to help:
    */
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    //           ^which block  ^block size   ^which thread in block
    //           gives each thread a unique position in the flat array

    // Because of the "ceiling" calculation we did earlier, we almost always launch slightly more threads than we have elements.
    // For example, if you have 1000 elements and 256 threads per block, you launch 1024 (4 * 256) threads. 
    // This means the last 24 threads will do nothing (their `idx` will be >= `numel`), which is perfectly fine and very common in CUDA.   
    if (idx < n)              
    {
        // Each thread gets same seed but different sequence number
        // This guarantees different random numbers per thread
        curand_init(seed, idx, 0, &states[idx]);
        //          ^seed ^sequence ^offset ^pointer to state
        //                ^^^
        //          sequence number: each thread gets a different
        //          sequence so they generate different random numbers
        //          even though seed is the same for all
    }
}

/*
    The Kernel Launch Syntax (`<<< ... >>>`)
    Once the grid dimensions (blocks, threads_per_block) are calculated, the kernel is dispatched to the GPU using the triple-chevron syntax `<<<blocks, threads>>>`:
    "<<< ... >>>": This is the CUDA execution configuration syntax.
                   It essentially tells the GPU runtime, "Hey, run the randn_kernel function on the GPU,
                   and organize the execution using this many blocks, with this many threads per block."
 */
// The actual random number generation kernel
__global__ void randn_kernel(curandState* states, double* data, size_t n)
{
    /*
        "size_t idx = ...": This is the most crucial part. Since every thread is running the same code,
                            it needs a way to figure out *which element* in the `data` array it is responsible for.
                            CUDA provides built-in variables to help:
    */
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    //           ^which block  ^block size   ^which thread in block
    //           gives each thread a unique position in the flat array

    // Because of the "ceiling" calculation we did earlier, we almost always launch slightly more threads than we have elements.
    // For example, if you have 1000 elements and 256 threads per block, you launch 1024 (4 * 256) threads. 
    // This means the last 24 threads will do nothing (their `idx` will be >= `numel`), which is perfectly fine and very common in CUDA. 
    if (idx < n)
    {
        // Use this thread's state to generate one random number
        data[idx] = curand_normal_double(&states[idx]);
    }
}

/*
    The Kernel Launch Syntax (`<<< ... >>>`)
    Once the grid dimensions (blocks, threads_per_block) are calculated, the kernel is dispatched to the GPU using the triple-chevron syntax `<<<blocks, threads>>>`:
    "<<< ... >>>": This is the CUDA execution configuration syntax.
                   It essentially tells the GPU runtime, "Hey, run the scale_kernel function on the GPU,
                   and organize the execution using this many blocks, with this many threads per block."
 */
// The scaling kernel, that correctly handles scaling elements by a factor using existing cuRand launch block sizes.
template <typename T>
__global__ void scale_kernel(T* data, T factor, size_t n)
{
    /*
        "size_t idx = ...": This is the most crucial part. Since every thread is running the same code,
                            it needs a way to figure out *which element* in the `data` array it is responsible for.
                            CUDA provides built-in variables to help:
    */
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    //           ^which block  ^block size   ^which thread in block
    //           gives each thread a unique position in the flat array

    // Because of the "ceiling" calculation we did earlier, we almost always launch slightly more threads than we have elements.
    // For example, if you have 1000 elements and 256 threads per block, you launch 1024 (4 * 256) threads. 
    // This means the last 24 threads will do nothing (their `idx` will be >= `numel`), which is perfectly fine and very common in CUDA. 
    if (idx < n)
    {
        data[idx] *= factor;
    }
}

/*
__global__ void randn_kernel(double* data, curandState* states, size_t n)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n)
    {
        curandState state = states[idx];
        data[idx] = curand_normal_double(&state);
        states[idx] = state;
    }
}
*/    

#endif

#endif