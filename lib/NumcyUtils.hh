/*
 * NumcyUtils.hh
 *
 * Q@hackers.pk
 */

#ifndef NUMCY_NUMCYUTILS_HH
#define NUMCY_NUMCYUTILS_HH

/*
    Why inside the include guards?
    If you place "#include <random>" outside the include guards, 
    the "<random>" header itself will be opened and processed by the C++ preprocessor *every single time* "NumcyUtils.hh" is included in a file,
    even if the contents of "NumcyUtils.hh" itself are skipped by its include guard. 
 */
#include <random> 

namespace NumcyUtils
{    
#ifdef COMPILE_FOR_DEVICE
    /*
        The key design decision: 
        keep randn_device as pure standard normal (mean=0, std=1), then let the caller scale. 
     */
    template <typename T = double, typename E = size_t>
    Collective<T, E> randn_device(const Dimensions<E>& d, uint64_t seed = 0)
    {
        E numel = d.numel();
        T* data = nullptr;

        // Step 1 — allocate device/host memory for the data
        cudaError_t err = cudaMalloc(&data, numel * sizeof(T));

        if (err != cudaSuccess)
        {
            throw std::runtime_error("NumcyUtils::randn_device(Dimensions<E>&, uint64_t) Error: cudaMalloc for data failed: " + std::string(cudaGetErrorString(err)));
        }

        // Step 2 — allocate device memory for cuRAND states
        // One curandState per thread, one thread per element
        curandState* states = nullptr;
        err = cudaMalloc(&states, sizeof(curandState) * numel);

        if (err != cudaSuccess)
        {
            cudaFree(data);   // clean up first allocation before throwing
            throw std::runtime_error("NumcyUtils::randn_device(Dimensions<E>&, uint64_t) Error: cudaMalloc for states failed: " + std::string(cudaGetErrorString(err)));
        }

        // Step 3 — configure grid and block dimensions
        // Before calling the kernel, calculate how many threads and blocks are needed to process every element in the tensor.
        E threads_per_block = 256; // This tells CUDA to group 256 threads into a single "Block". 256 is a standard, efficient size for CUDA execution.
        E blocks = (numel + threads_per_block - 1) / threads_per_block; // This calculates how many "Blocks" are needed to cover all elements. In each block every element is processed by a thread (one thread per element or every element gets its own thread). 
        /*
            The `+ threads_per_block - 1` part is a common trick to handle the ceiling division (rounding up).
            For example, if you have 1000 elements and 256 threads per block, you need 4 blocks (1000 + 256 - 1) = 1255 
            1255 / 256 = 4.88... which is truncated becuase we are using integer division, not floating so, E should be type without partial values 
            So it equal to ceil(1000 / 256) = ceil(3.90625) = 4 = (1000 + 256 - 1) / 256 (integer division)
            
            Because of the "ceiling" calculation we did earlier, we almost always launch slightly more threads than we have elements.
            For example, if you have 1000 elements, you launch 1024 (4 * 256) threads. 
            This means the last 24 threads will do nothing (their `idx` will be >= `numel`), which is perfectly fine and very common in CUDA.
        */

        // Step 4 — launch kernels, cast to unsigned int for CUDA. Initialize each thread's cuRAND state
        // Matches randn_host: standard normal mean=0, std=1
        // Caller scales for their specific init scheme (Xavier, He, Word2Vec etc.)
        /*
            CUDA kernel launch syntax expects unsigned int or int for grid and block dimensions, not size_t. 
            Need to static_cast to unsigned int, which is explicit and clean, satisfies -Wconversion and makes the intent clear.
            The "E = size_t" arithmetic is correct for large tensors, and the cast to unsigned int only happens at the kernel launch boundary where CUDA requires it.
        */
        setup_curand_kernel<<<static_cast<unsigned int>(blocks), static_cast<unsigned int>(threads_per_block)>>>(states, seed, numel);
        err = cudaGetLastError();
        if (err != cudaSuccess)
        {
            cudaFree(data);
            cudaFree(states);
            throw std::runtime_error("NumcyUtils::randn_device(Dimensions<E>&, uint64_t) setup_curand_kernel() Error: " + std::string(cudaGetErrorString(err)));
        }

        // Step 5 — launch the random number generation kernel
        /*
            The Kernel Launch Syntax (`<<< ... >>>`)
            Once the grid dimensions (blocks, threads_per_block) are calculated, the kernel is dispatched to the GPU using the triple-chevron syntax `<<<blocks, threads>>>`:
            "<<< ... >>>": This is the CUDA execution configuration syntax.
                           It essentially tells the GPU runtime, "Hey, run the randn_kernel function on the GPU,
                           and organize the execution using this many blocks, with this many threads per block."

            CUDA kernel launch syntax expects unsigned int or int for grid and block dimensions, not size_t. 
            Need to static_cast to unsigned int, which is explicit and clean, satisfies -Wconversion and makes the intent clear.
            The "E = size_t" arithmetic is correct for large tensors, and the cast to unsigned int only happens at the kernel launch boundary where CUDA requires it.
        */
        randn_kernel<<<static_cast<unsigned int>(blocks), static_cast<unsigned int>(threads_per_block)>>>(states, data, numel);
        err = cudaGetLastError();
        if (err != cudaSuccess)
        {
            cudaFree(data);
            cudaFree(states);
            throw std::runtime_error("NumcyUtils::randn_device(Dimensions<E>&, uint64_t) randn_kernel() Error: " + std::string(cudaGetErrorString(err)));
        }

        // Step 6 — free the states, they are no longer needed
        cudaFree(states);

        return Collective<T, E>(data, d, MemoryLocation::Device);
    }
#endif 

    /*
        The key design decision: 
        keep randn_host as pure standard normal (mean=0, std=1), then let the caller scale. 
     */
    template <typename T = double, typename E = size_t>
    Collective<T, E> randn_host(const Dimensions<E>& d, uint64_t seed = 0)
    {
        E numel = d.numel();

        if (!numel)
        {
            throw std::runtime_error("NumcyUtils::randn_host: shape must not be zero");
        }

        T* data = nullptr;
        try
        {
            data = new T[numel];
        }
        catch (const std::bad_alloc& e)
        {
            throw std::runtime_error(std::string("NumcyUtils::randn_host: alloc failed: ") + e.what());
        }   

        // Matches device: standard normal mean=0, std=1
        // Caller scales for their specific init scheme (Xavier, He, Word2Vec etc.)
        std::mt19937_64 gen(seed ? seed : std::random_device{}());
        std::normal_distribution<T> dis(static_cast<T>(0), static_cast<T>(1));

        for (E i = 0; i < numel; i++)
        {
            data[i] = dis(gen);
        }

        return Collective<T, E>(data, d, MemoryLocation::Host);
    }
    
    // ─────────────────────────────────────────────────────────────
    // Base: standard normal, mean=0 std=1 (already done above)
    // Everything below scales the output of randn_host/randn_device
    // ─────────────────────────────────────────────────────────────
    // ─────────────────────────────────────────────────────────────
    // Scales every element of a Collective in-place on host
    template <typename T = double, typename E = size_t>
    void scale_host(Collective<T, E>& c, T factor)
    {
        E numel = c.getShape().numel();

        for (E i = 0; i < numel; i++)
        {
            c[i] *= factor;
        }
    }

/*
    The CUDA kernel launch syntax and the `scale_kernel` function work together to distribute a task across thousands of parallel GPU threads. 
    The kernel is the reusable parallel algorithm, and the launch syntax is how you invoke it on a specific grid and block configuration.

 */    
#ifdef COMPILE_FOR_DEVICE
    // Scales every element of a Collective in-place on device
    template <typename T = double, typename E = size_t>
    void scale_device(Collective<T, E>& c, T factor)
    {
        E numel = c.getShape().numel();
        T* data = c.getData();

        // Before calling the kernel, calculate how many threads and blocks are needed to process every element in the tensor.
        E threads_per_block = 256; // This tells CUDA to group 256 threads into a single "Block". 256 is a standard, efficient size for CUDA execution.
        E blocks = (numel + threads_per_block - 1) / threads_per_block; // This calculates how many "Blocks" are needed to cover all elements. In each block every element is processed by a thread (one thread per element or every element gets its own thread). 
        /*
            The `+ threads_per_block - 1` part is a common trick to handle the ceiling division (rounding up).
            For example, if you have 1000 elements and 256 threads per block, you need 4 blocks (1000 + 256 - 1) = 1255 
            1255 / 256 = 4.88... which is truncated becuase we are using integer division, not floating so, E should be type without partial values 
            So it equal to ceil(1000 / 256) = ceil(3.90625) = 4 = (1000 + 256 - 1) / 256 (integer division) 

            Because of the "ceiling" calculation we did earlier, we almost always launch slightly more threads than we have elements.
            For example, if you have 1000 elements, you launch 1024 (4 * 256) threads. 
            This means the last 24 threads will do nothing (their `idx` will be >= `numel`), which is perfectly fine and very common in CUDA.
        */

        /*
            The Kernel Launch Syntax (`<<< ... >>>`)
            Once the grid dimensions (blocks, threads_per_block) are calculated, the kernel is dispatched to the GPU using the triple-chevron syntax `<<<blocks, threads>>>`:
            "<<< ... >>>": This is the CUDA execution configuration syntax.
                           It essentially tells the GPU runtime, "Hey, run the scale_kernel function on the GPU,
                           and organize the execution using this many blocks, with this many threads per block."

            CUDA kernel launch syntax expects unsigned int or int for grid and block dimensions, not size_t. 
            Need to static_cast to unsigned int, which is explicit and clean, satisfies -Wconversion and makes the intent clear.
            The "E = size_t" arithmetic is correct for large tensors, and the cast to unsigned int only happens at the kernel launch boundary where CUDA requires it.
        */
        scale_kernel<T><<<static_cast<unsigned int>(blocks), static_cast<unsigned int>(threads_per_block)>>>(data, factor, numel);
        cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess)
        {
            throw std::runtime_error("NumcyUtils::scale_device(Collective<T, E>&, T) Error: " + std::string(cudaGetErrorString(err)));
        }
    }
#endif

};

#endif // NUMCY_NUMCYUTILS_HH