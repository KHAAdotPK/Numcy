/*
 * Numcy/Collective.hh
 * Q@hackers.pk
 */

#ifndef NUMCY_COLLECTIVE_HH
#define NUMCY_COLLECTIVE_HH

template <typename T = double, typename E = size_t>
class Collective
{
    /*
     *   Collective A ──┐
     *                  ├──► CollectiveProperties (refcount=2)
     *   Collective B ──┘         ├──► data[]
     *                            └──► Dimensions (value member)
     *                                  ├──► head*
     *                                  └──► tail*
     *                                        └──► DimensionsProperties nodes
     *                                              (refcount incremented for
     *                                               every Dimensions that shares them)
     */
    CollectiveProperties<T, E>* properties;
    
    public: 

        /*
            *  Collective()
            *  └─► this->properties = nullptr            
         */
        Collective(void) : properties(nullptr)
        {
        }
    
        /*
            *  Collective(T* ptr, Dimensions<E>& d)
            *  ├─► try
            *  │     ├─► this->properties = new CollectiveProperties<T, E>(ptr, d)
         */
        Collective(T* ptr, const Dimensions<E>& d, MemoryLocation mem_loc = MemoryLocation::Device) : properties(nullptr)
        {
            try
            {
                properties = new CollectiveProperties<T, E>(ptr, d, mem_loc);

                /*
                    CollectiveProperties<T, E>(T* ptr, Dimensions<E>& d, MemoryLocation mem_loc)
                    ├─► this->data = ptr
                    ├─► this->dimensions = d
                    └─► this->mem_loc = mem_loc
                */
            }
            catch (std::runtime_error& e)
            {
                throw std::runtime_error("Collective<T, E>::Collective(T*, Dimensions<E>) Error: " + std::string(e.what()));
            }
            catch (...)
            {
                throw std::runtime_error("Collective<T, E>::Collective(T*, Dimensions<E>) Error: Unknown exception");
            }
        }

        /*
            *  Collective(const Dimensions<E>& d, MemoryLocation mem_loc = MemoryLocation::Host)
            *  ├─► try
            *  │     ├─► this->properties = new CollectiveProperties<T, E>(d, mem_loc)
         */
        Collective(const Dimensions<E>& d, MemoryLocation mem_loc = MemoryLocation::Host) : properties(nullptr)
        {
            try
            {
                properties = new CollectiveProperties<T, E>(d, mem_loc);
            }
            catch (std::runtime_error& e)
            {
                throw std::runtime_error("Collective<T, E>::Collective(Dimensions<E>) -> " + std::string(e.what()));
            }
            catch (...)
            {
                throw std::runtime_error("Collective<T, E>::Collective(Dimensions<E>) Error: Unknown exception");
            }            
        }

        /*
            *  Collective(const Collective<T, E>& other)
            *  └─► this->properties = other.properties
         */
        Collective(const Collective<T, E>& other) : properties(other.properties)
        {
            /*
             *  Collective<T, E>(const Collective<T, E>& other)
             *  ├─► this->properties = other.properties
             *  └─► if (this->properties != nullptr)
             *        └─► this->properties->incrementReferenceCount()
             */
            if (this->properties != nullptr) 
            {
                this->properties->incrementReferenceCount();
            }
        }

        /*
            *  Collective<T, E>& operator=(const Collective<T, E>& other)
            *  ├─► if (this->properties != nullptr)
            *  │     ├─► this->properties->decrementReferenceCount()
            * │     └─► if (getReferenceCount() == 0)
            *  │           └─► delete this->properties
            * │  ├─► this->properties = other.properties
            *  └─► if (this->properties != nullptr)
            *       └─► this->properties->incrementReferenceCount()
         */
        Collective<T, E>& operator=(const Collective<T, E>& other)
        {
            /*
             *  Collective<T, E>& operator=(const Collective<T, E>& other)
             *  ├─► if (this->properties != nullptr)
             *  │     ├─► this->properties->decrementReferenceCount()
             *  │     └─► if (getReferenceCount() == 0)
             *  │           └─► delete this->properties
             *  │                 └─► ~CollectiveProperties()
             *  │                       ├─► delete[] data        (manual)
             *  │                       └─► ~Dimensions<E>()     (automatic)
             *  ├─► this->properties = other.properties
             *  └─► if (this->properties != nullptr)
             *        └─► this->properties->incrementReferenceCount()
             */
            if (this != &other)
            {
                if (this->properties != nullptr)  // ← guard
                {
                    this->properties->decrementReferenceCount();

                    if (this->properties->getReferenceCount() == 0)
                    {
                        delete this->properties;
                    }
                }

                this->properties = other.properties;
                if (this->properties != nullptr)
                {
                    this->properties->incrementReferenceCount();
                }
            }

            return *this;
        }
        
        ~Collective()
        {            
            /*
             *  ~Collective()
             *  └─► if (this->properties != nullptr)
             *        ├─► this->properties->decrementReferenceCount()
             *        └─► if (getReferenceCount() == 0)
             *              └─► delete this->properties
             *                    └─► ~CollectiveProperties()
             *                          ├─► delete[] data        (manual)
             *                          └─► ~Dimensions<E>()     (automatic)
             */
            if (this->properties != nullptr)
            {
                this->properties->decrementReferenceCount();

                if (this->properties->getReferenceCount() == 0)
                {
                    delete this->properties;                                                 
                }

                this->properties = nullptr;
            }
        }

        // //////////////////// //
        // Overloaded Operators //
        // //////////////////// //
        
        // Non-const version — read and write
        /*
            *  T& operator[](E index)
            *  ├─► if (this->properties == nullptr)
            *  │     └─► throw std::runtime_error("Collective<T, E>::operator[](E) Error: properties is nullptr")
            *  ├─► if (index >= this->getShape().numel())
            *  │     └─► throw std::runtime_error("Collective<T, E>::operator[](E) Error: index out of bounds")
            *  └─► return this->properties->getData()[index]
         */
        T& operator[](E index)
        {
            if (this->properties == nullptr)
            {
                throw std::runtime_error("Collective<T, E>::operator[](E) Error: properties is nullptr");
            }

            if (index >= this->getShape().numel())
            {
                throw std::runtime_error("Collective<T, E>::operator[](E) Error: index out of bounds");
            }

            return this->properties->getData()[index];
        }

        // Const version — read only
        /*
            *  const T& operator[](E index) const
            *  ├─► if (this->properties == nullptr)
            *  │     └─► throw std::runtime_error("Collective<T, E>::operator[](E) const Error: properties is nullptr")
            *  ├─► if (index >= this->getShape().numel())
            *  │     └─► throw std::runtime_error("Collective<T, E>::operator[](E) const Error: index out of bounds")
            *  └─► return this->properties->getData()[index]
         */
        const T& operator[](E index) const
        {
            if (this->properties == nullptr)
            {
                throw std::runtime_error("Collective<T, E>::operator[](E) const Error: properties is nullptr");
            }

            if (index >= this->getShape().numel())
            {
                throw std::runtime_error("Collective<T, E>::operator[](E) const Error: index out of bounds");
            }

            return this->properties->getData()[index];
        }

        // //////////////////// //
        // Other Public Methods //
        // //////////////////// //

        const Dimensions<E>& getShape(void) const
        {
            /*
                The method is const, so it cannot return a non-const value.
                That is the reason for const qualifier on the return type.

                Why return by value?
                --------------------
                Returning by value ensures that the caller gets their own Dimensions object.
                The const ensures that the caller cannot modify the Dimensions object.
             */

            if (this->properties == nullptr)
            {
                throw std::runtime_error("Collective<T, E>::getShape() Error: CollectiveProperties<T, E> is nullptr");
            }

            return this->properties->getDimensions();
        }

        /*
            Exposing Data Pointer
            This allows the utility functions to retrieve the raw pointer (either Host or Device pointer) necessary for launching CUDA kernels over the raw array.

            T* getData(void) const
            ├─► if (this->properties == nullptr)
            │     └─► throw std::runtime_error("Collective<T, E>::getData() Error: CollectiveProperties<T, E> is nullptr")
            └─► return this->properties->getData()
         */
        T* getData(void) const
        {
            if (this->properties == nullptr)
            {
                throw std::runtime_error("Collective<T, E>::getData() Error: CollectiveProperties<T, E> is nullptr");
            }

            return this->properties->getData();
        }

        MemoryLocation getMemoryLocation(void) const
        {
            if (this->properties == nullptr)
            {
                throw std::runtime_error("Collective<T, E>::getMemoryLocation() Error: CollectiveProperties<T, E> is nullptr");
            }

            return this->properties->getMemoryLocation();
        }

        /*
            Collective<T, E> toDevice(void)
            ├─► if (this->properties == nullptr)
            │     └─► throw std::runtime_error("Collective<T, E>::toDevice() Error: CollectiveProperties<T, E> is nullptr")
            ├─► if (this->getData() == nullptr)
            │     └─► throw std::runtime_error("Collective<T, E>::toDevice() Error: CollectiveProperties<T, E>::getData() returned nullptr")
            ├─► if (this->properties->getMemoryLocation() == MemoryLocation::Device)
            │     └─► return *this  // Already on device
            ├─► Allocate device memory
            ├─► Copy from host to device
            └─► Return new device Collective
        */
        Collective<T, E> toDevice(void)
        {
            if (this->properties == nullptr)
            {
                throw std::runtime_error("Collective<T, E>::toDevice() Error: CollectiveProperties<T, E> is nullptr");
            }

            if (this->getData() == nullptr)
            {
                throw std::runtime_error("Collective<T, E>::toDevice() Error: CollectiveProperties<T, E>::getData() returned nullptr");
            }

            if (this->properties->getMemoryLocation() == MemoryLocation::Device)
            {
                return *this;  // Already on device
            }

            /*
                No, there's no need for a CUDA kernel here.                 
             */
            Collective<T, E> device_collective;

#ifdef COMPILE_FOR_DEVICE
            // 1. Calculate the size of the data in bytes
            size_t numel = this->getShape().numel();
            size_t byteSize = numel * sizeof(T);  // size_t is an unsigned integer type that is used to represent the size of objects in memory.
        
            // 2. Allocate memory on the GPU
            T* ptr = nullptr;
            cudaError_t allocErr = cudaMalloc(&ptr, byteSize);
            if (allocErr != cudaSuccess) 
            {
                throw std::runtime_error("Numcy::toDevice() cudaMalloc() Error: " + std::string(cudaGetErrorString(allocErr)));
            }

            // 3. Copy from Host (this->getData()) to Device (d_ptr)
            /*
                cudaMemcpy with cudaMemcpyHostToDevice is specifically designed for this exact operation, 
                bulk data transfer from host to device memory.
                - Handled by the CUDA runtime, which uses DMA (Direct Memory Access) internally, bypassing the GPU's compute cores entirely
                - Faster than a kernel for this purpose, since a kernel would need to launch threads, schedule warps,
                  and go through the GPU pipeline just to read memory.
                - Simpler and safer: no thread indexing, no synchronization concerns  
            */            
            cudaError_t copyErr = cudaMemcpy(ptr, this->getData(), byteSize, cudaMemcpyHostToDevice);
            if (copyErr != cudaSuccess) 
            {
                cudaFree(ptr); // Clean up if copy fails
                throw std::runtime_error("Numcy::toDevice() cudaMemcpy() Error: " + std::string(cudaGetErrorString(copyErr)));
            }

            // 4. Update the collective object 'c' with the new device pointer
            // Note: You might need to ensure 'c' or 'this->properties' handles the 
            // lifecycle of this new d_ptr so you don't leak GPU memory!
            device_collective = Collective<T, E>(ptr, this->getShape(), MemoryLocation::Device);
#endif                       
            return device_collective;

            /*
                Where kernels would make sense:
                -------------------------------
                If you needed to perform an operation on the data during the transfer (e.g., scaling, type conversion, or a custom transformation),
                then a kernel would be necessary. For example:

                // Kernel-based scaling (if you wanted to scale by 2.0 during transfer)
                __global__ void scale_kernel(float* data, float scale, size_t n) {
                    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
                    if (idx < n) {
                        data[idx] *= scale;
                    }
                }

                // Usage
                scale_kernel<<<blocks, threads>>>(d_ptr, 2.0f, numel);
                cudaDeviceSynchronize(); // Wait for kernel to finish

                But for a raw memory copy, cudaMemcpy is the correct and most efficient tool.

                ---------------------------------------------------------------------------------

                To support type promotion on upload, for example uploading float16 from host but storing float32 on device, a kernel would be warranted:

                // Host has float16, device needs float32
                __global__ void convertHalfToFloat(const __half* src, float* dst, size_t numel)
                {
                    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
                    if (idx < numel)
                        dst[idx] = __half2float(src[idx]);
                }
            */
        }

        /*
            Collective<T, E> toHost(void)
            ├─► if (this->properties == nullptr)
            │     └─► throw std::runtime_error("Collective<T, E>::toHost() Error: CollectiveProperties<T, E> is nullptr")
            ├─► if (this->properties->getData() == nullptr)
            │     └─► throw std::runtime_error("Collective<T, E>::toHost() Error: CollectiveProperties<T, E>::getData() returned nullptr")
            ├─► if (this->properties->getMemoryLocation() == MemoryLocation::Host)
            │     └─► return *this  // Already on host
            ├─► Allocate host memory
            ├─► Copy from device to host
            └─► Return new host Collective
        */
        Collective<T, E> toHost(void)
        {            
            if (this->properties == nullptr)
            {
                throw std::runtime_error("Collective<T, E>::toHost() Error: CollectiveProperties<T, E> is nullptr");
            }

            if (this->properties->getData() == nullptr)
            {
                throw std::runtime_error("Collective<T, E>::toHost() Error: CollectiveProperties<T, E>::getData() returned nullptr");
            }

            if (this->properties->getMemoryLocation() == MemoryLocation::Host)
            {
                return *this; // Copy constructor increments reference count of properties
            }

            /*
                No, there's no need for a CUDA kernel here.                 
             */

            Collective<T, E> host_collective; // Default constructor initializes properties to nullptr
         
#ifdef COMPILE_FOR_DEVICE
            T* host_data = nullptr;
            E numel = this->getShape().numel(); 

            try
            {
                host_data = new T[numel];                
            }
            catch (std::bad_alloc& e)
            {
                throw std::runtime_error("Collective<T, E>::toHost() Error: " + std::string(e.what()));
            }
            catch (std::exception& e)
            {
                throw std::runtime_error("Collective<T, E>::toHost() Error: " + std::string(e.what()));
            }
            catch (...)
            {
                throw std::runtime_error("Collective<T, E>::toHost() Error: Unknown exception");
            }

            // Copy from device to host
            /*
                cudaMemcpy with cudaMemcpyDeviceToHost is specifically designed for this exact operation, 
                bulk data transfer from device to host memory.
                - Handled by the CUDA runtime, which uses DMA (Direct Memory Access) internally, bypassing the GPU's compute cores entirely
                - Faster than a kernel for this purpose, since a kernel would need to launch threads, schedule warps,
                  and go through the GPU pipeline just to read memory.
                - Simpler and safer: no thread indexing, no synchronization concerns  
            */
            cudaError_t err = cudaMemcpy(host_data, this->properties->getData(), numel * sizeof(T), cudaMemcpyDeviceToHost);
            if (err != cudaSuccess)
            {
                delete[] host_data;
                throw std::runtime_error("Collective<T, E>::toHost() -> cudaMemcpy() Error: " + std::string(cudaGetErrorString(err)));
            }

            host_collective = Collective<T, E>(host_data, this->getShape(), MemoryLocation::Host);
#endif
            return host_collective;  
            
            /*
                When you would use a kernel:
                If you needed to perform some computation on the data *during* the transfer
                (e.g., convert from int16 to float32 on the fly, apply a mask, or perform a simple transformation),
                then a kernel would be appropriate. In that case, you would:
                1. Allocate device memory for the output
                2. Launch a kernel that reads from the input device array and writes to the output device array
                3. Copy the output device array back to the host
                4. Return the new host collective

                Example:
                Collective<T, E> transform_device_to_host(const Collective<T, E>& device_collective)
                {
                    // 1. Allocate device memory for the output
                    E numel = device_collective.getShape().numel();
                    T* d_output = nullptr;
                    cudaMalloc(&d_output, numel * sizeof(T));
                    cudaError_t err = cudaGetLastError();
                    if (err != cudaSuccess)
                    {
                        throw std::runtime_error("Collective<T, E>::toHost() -> cudaMalloc() Error: " + std::string(cudaGetErrorString(err)));
                    }

                    // 2. Launch a kernel that reads from the input device array and writes to the output device array
                    transform_kernel<<<blocks, threads>>>(device_collective.getData(), d_output, numel);
                    err = cudaGetLastError();
                    if (err != cudaSuccess)
                    {
                        cudaFree(d_output);
                        throw std::runtime_error("Collective<T, E>::toHost() -> transform_kernel() Error: " + std::string(cudaGetErrorString(err)));
                    }

                    // 2.1. Synchronize the device
                    err = cudaDeviceSynchronize();
                    if (err != cudaSuccess)
                    {
                        cudaFree(d_output);
                        throw std::runtime_error("Collective<T, E>::toHost() -> cudaDeviceSynchronize() Error: " + std::string(cudaGetErrorString(err)));
                    }

                    // 3. Copy the output device array back to the host
                    T* h_output = new T[numel];
                    cudaMemcpy(h_output, d_output, numel * sizeof(T), cudaMemcpyDeviceToHost);
                    err = cudaGetLastError();
                    if (err != cudaSuccess)
                    {
                        cudaFree(d_output);
                        delete[] h_output;
                        throw std::runtime_error("Collective<T, E>::toHost() -> cudaMemcpy() Error: " + std::string(cudaGetErrorString(err)));
                    }

                    // 4. Return the new host collective
                    return Collective<T, E>(h_output, device_collective.getShape(), MemoryLocation::Host);
                }
             */
        }

        /*
            Please go through the following document for more information...
            collective_data_model.docx
         */
        Collective<T, E> transpose(numcy::Axis axis1 = numcy::Axis::Last, numcy::Axis axis2 = numcy::Axis::SecondLast)
        {
            std::vector<E> vec1 = this->getShape().toVector();

            for (size_t i = 0; i < vec1.size(); i++)
            {
                std::cout<< vec1[i] << " ";
            }
            std::cout<< std::endl;

            std::cout<< "vec1.size() = " << vec1.size() << std::endl;

            Dimensions<E> dims_transposed = this->getShape().transpose(axis1, axis2);

            std::vector<E> vec2 = dims_transposed.toVector();

            for (size_t i = 0; i < vec2.size(); i++)
            {
                std::cout<< vec2[i] << " ";
            }
            std::cout<< std::endl;

            std::cout<< "vec2.size() = " << vec2.size() << std::endl;

            /*Dimensions<E> dims = this->getShape();
            Dimensions<E> dims_transposed = dims.transpose();*/

            /* Phyical transpose only when axis1 is Last and axis2 is SecondLast */
            if (axis1 == numcy::Axis::Last && axis2 == numcy::Axis::SecondLast)
            {
                //Numcy::transpose(*this);           
            }
            
            /*Collective<T, E> transposed_collective =*/ return Collective<T, E>(this->getData(), dims_transposed, this->properties->getMemoryLocation());            
        }      
};

#endif