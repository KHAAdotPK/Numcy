/*
 * Numcy/Numcy.hh
 * Q@hackers.pk
 */

#ifndef NUMCY_NUMCY_HH
#define NUMCY_NUMCY_HH

class Numcy
{
    public:
        template <typename T = double, typename E = size_t>
        static Collective<T, E> randn(const Dimensions<E>& d, uint64_t seed = 0)
        {
#ifdef COMPILE_FOR_DEVICE
            return NumcyUtils::randn_device(d, seed);
#else
            return NumcyUtils::randn_host(d, seed);
#endif            
        }

        // ─────────────────────────────────────────────────────────────
        // BERT / GPT / Transformer Initialization
        // ─────────────────────────────────────────────────────────────
        /**
         * @brief BERT-Standard Weight Initialization (mean=0.0, std=0.02)
         * 
         * Generates a Standard Normal distribution and scales it to standard 
         * deviation 0.02. This initialization is widely used in modern transformer 
         * architectures (e.g., BERT, GPT) for embeddings, attention weights, 
         * and linear layers.
         * 
         * RATIONALE:
         * Keeping initial weights close to zero with a small, fixed variance 
         * helps maintain stable activation variances across deep layers, preventing 
         * vanishing or exploding gradients during early training.
         */
        template <typename T = double, typename E = size_t>
        static Collective<T, E> randn_bert(const Dimensions<E>& d, uint64_t seed = 0)
        {
            Collective<T, E> c;
#ifdef COMPILE_FOR_DEVICE
            c = NumcyUtils::randn_device<T, E>(d, seed); // Standard Normal (mean=0, std=1)
            NumcyUtils::scale_device(c, static_cast<T>(0.02));            // Scale to std=0.02
#else
            c = NumcyUtils::randn_host<T, E>(d, seed);   // Standard Normal (mean=0, std=1)
            NumcyUtils::scale_host(c, static_cast<T>(0.02));              // Scale to std=0.02
#endif
            return c;
        }
        
        template <typename T = double, typename E = size_t>
        static Collective<T, E> transpose(const Collective<T, E>& c, numcy::Axis axis1 = numcy::Axis::Last, numcy::Axis axis2 = numcy::Axis::SecondLast)        
        {
            Dimensions<E> d_transposed;
            T* data_transposed = nullptr;

            try
            {
                // Step 1 — get the transposed dimensions
                d_transposed = c.getShape().transpose(axis1, axis2);

                // Step 2 — Allocate memory for host or device and initialize it with the data from the original collective but transposed
                if (axis1 != numcy::Axis::Last && axis2 != numcy::Axis::SecondLast)
                {
                    if (c.getMemoryLocation() == MemoryLocation::Host)
                    {
                        // Allocate memory for host
                        data_transposed = new T[c.getShape().numel()];

                        /*
                            For axis1 != Last and axis2 != SecondLast — these are higher dimensional swaps like swapping batch and sequence dimensions in a 3D tensor [batch, sequence, features].
                            In those cases only the shape metadata changes, the data layout in memory stays the same. 
                         */
                        
                        // Copy data from the original collective to the transposed memory location
                        memcpy(data_transposed, c.getData(), c.getShape().numel() * sizeof(T));
                    }
                    else // MemoryLocation::Device
                    {
#ifdef COMPILE_FOR_DEVICE
                        // Allocate memory for device
                        cudaError_t err = cudaMalloc(&data_transposed, c.getShape().numel() * sizeof(T));
                        if (err != cudaSuccess)
                        {
                            throw std::runtime_error("Numcy::transpose(const Collective<T, E>&, Axis, Axis) -> " + std::string(cudaGetErrorString(err)));
                        }

                        /*
                            For axis1 != Last and axis2 != SecondLast — these are higher dimensional swaps like swapping batch and sequence dimensions in a 3D tensor [batch, sequence, features].
                            In those cases only the shape metadata changes, the data layout in memory stays the same. 
                         */

                        // Copy data from the original collective to the transposed memory location
                        err = cudaMemcpy(data_transposed, c.getData(), c.getShape().numel() * sizeof(T), cudaMemcpyDeviceToDevice);
                        if (err != cudaSuccess)
                        {
                            cudaFree(data_transposed);
                            throw std::runtime_error("Numcy::transpose(const Collective<T, E>&, Axis, Axis) -> " + std::string(cudaGetErrorString(err)));
                        }
#endif                       
                    }

                    return Collective<T, E>(data_transposed, d_transposed, c.getMemoryLocation());                                        
                }

                // Step 4 — if axis1 == numcy::Axis::Last && axis2 == numcy::Axis::SecondLast
                if (c.getMemoryLocation() == MemoryLocation::Host)
                {
                    // Allocate memory for host
                    data_transposed = new T[c.getShape().numel()];

                    /* A physical transpose; actually shuffle the bytes. */

                    for (E i = 0; i < d_transposed.getNumberOfRows(); i++)
                    {
                        for (E j = 0; j < d_transposed.getNumberOfColumns(); j++)
                        {
                            data_transposed[j * d_transposed.getNumberOfColumns() + i] = c.getData()[i * c.getShape().getNumberOfColumns() + j];
                        }
                    }

                    return Collective<T, E>(data_transposed, d_transposed, /*c.getMemoryLocation()*/ MemoryLocation::Host);
                }

                /* A physical transpose; actually shuffle the bytes. */

                // Step 5 — MemoryLocation::Device
                // Input data is on GPU. Transpose entirely on GPU using a kernel.
                // No host round-trip needed or wanted.
#ifdef COMPILE_FOR_DEVICE
                // Allocate OUTPUT buffer on device (not host)
                //T* data_transposed = nullptr;
                size_t numel = c.getShape().numel();
                E rows = c.getShape().getNumberOfRows();
                E cols = c.getShape().getNumberOfColumns();

                cudaError_t err = cudaMalloc(&data_transposed, numel * sizeof(T));
                if (err != cudaSuccess)
                {
                    throw std::runtime_error("Numcy::transpose(const Collective<T, E>&, Axis, Axis) -> " + std::string(cudaGetErrorString(err)));
                }

                // Calculate grid/block dimensions for naive transpose kernel
                const E threads_per_block = 256;
                const E blocks = (numel + threads_per_block - 1) / threads_per_block;

                // Launch naive transpose kernel
                transpose_kernel<T, E><<<blocks, threads_per_block>>>(c.getData(), data_transposed, rows, cols);

                // Check for kernel launch errors
                err = cudaGetLastError();
                if (err != cudaSuccess)
                {
                    cudaFree(data_transposed);
                    throw std::runtime_error("Numcy::transpose(const Collective<T, E>&, Axis, Axis) -> " + std::string(cudaGetErrorString(err)));
                }

                // Create Collective wrapper for device memory
                return Collective<T, E>(data_transposed, d_transposed, /*c.getMemoryLocation()*/ MemoryLocation::Device);
#endif                                                
            }
            catch (const std::bad_alloc& e)
            {
                throw std::runtime_error("Numcy::transpose(const Collective<T, E>&, Axis, Axis) -> " + std::string(e.what()));
            }
            catch (std::runtime_error& e)
            {
                throw std::runtime_error("Numcy::transpose(const Collective<T, E>&, Axis, Axis) -> " + std::string(e.what()));
            }
            catch (...)
            {
                throw std::runtime_error("Numcy::transpose(const Collective<T, E>&, Axis, Axis) Error: Unknown exception");
            }

            // Should never reach heres
            return Collective<T, E> (nullptr, Dimensions<E>(), MemoryLocation::None);        
        }
};

#endif