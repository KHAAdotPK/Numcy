/*
 * Numcy/CollectiveProperties.hh
 * 
 * Q@hackers.pk
 */

#ifndef NUMCY_COLLECTIVE_PROPERTIES_HH
#define NUMCY_COLLECTIVE_PROPERTIES_HH

#include "./Dimensions.hh"

template <typename T = double, typename E = size_t>
class CollectiveProperties
{
    /*
        Strict -Wconversion -Wsign-conversion flags, and Dimensions::numel() returns type. 
        Dimensions::numel() returns type is size_t and here it's being assigned to E, which is a template parameter.
        This is a potential issue because size_t is unsigned and E can be signed (int). 
        However, this is not a bug because the number of elements in a matrix cannot be negative.
        Just make sure that E is same type as Dimensions::numel() — or at minimum a type that compares without signed/unsigned mismatch warnings under strict -Wconversion -Wsign-conversion flags.
     */
    Dimensions<E> dimensions; // It's a value member it destructor will be called automatically when the object is destroyed
    T* data; // It's a pointer member it destructor will not be called automatically when the object is destroyed
    size_t reference_count;
    MemoryLocation memory_location;
    
    public:

        /*
            *  CollectiveProperties(T* ptr, Dimensions<E>& d, MemoryLocation mem_loc = MemoryLocation::Device)
            *  ├─► this->dimensions = d
            *  ├─► this->data = ptr
            *  ├─► this->reference_count = 1
            *  └─► this->memory_location = mem_loc
         */
        CollectiveProperties(T* ptr, const Dimensions<E>& d, MemoryLocation mem_loc = MemoryLocation::Device) : dimensions(d), data(ptr), reference_count(1), memory_location(mem_loc)
        {
        }

        /*
            *  CollectiveProperties(const Dimensions<E>& d, MemoryLocation mem_loc = MemoryLocation::Host)
            *  ├─► this->dimensions = d
            *  ├─► this->data = new T[this->dimensions.numel()]
            *  ├─► this->reference_count = 1
            *  └─► this->memory_location = mem_loc
         */
        CollectiveProperties(const Dimensions<E>& d, MemoryLocation mem_loc = MemoryLocation::Host) : dimensions(d), data(nullptr), reference_count(1), memory_location(mem_loc)
        {
            try
            {
                this->data = new T[this->dimensions.numel()];
            }
            catch (const std::bad_alloc& e)
            {
                throw std::runtime_error("CollectiveProperties<T, E>::CollectiveProperties(Dimensions<E>) Error: " + std::string(e.what()));
            }
            catch (const std::exception& e)
            {
                throw std::runtime_error("CollectiveProperties<T, E>::CollectiveProperties(Dimensions<E>) Error: " + std::string(e.what()));
            }
            catch (...)
            {
                throw std::runtime_error("CollectiveProperties<T, E>::CollectiveProperties(Dimensions<E>) Error: Unknown exception");
            }
        }

        /*
            *  CollectiveProperties(const CollectiveProperties<T, E>& other)
            *  ├─► this->dimensions = other.dimensions
            *  ├─► this->data = other.data
            *  ├─► this->reference_count = other.reference_count
            *  └─► this->memory_location = other.memory_location
         */
        CollectiveProperties(const CollectiveProperties<T, E>& other) : dimensions(other.dimensions), data(other.data), reference_count(other.reference_count), memory_location(other.mem_loc)
        {
            this->incrementReferenceCount();
        }

        /*
         * Under the -Werror=effc++ flag (Effective C++ compliance), the compiler requires
         * that any class with pointer data members explicitly declares a copy assignment
         * operator. This is because the compiler-generated default operator= would perform
         * a shallow copy of the raw pointer 'data', causing two objects to point to the
         * same heap memory — leading to a double-free when both destructors run.
         *
         * We do not want assignment semantics for this class, so instead of implementing
         * a full operator= we explicitly delete it. This satisfies the compiler's requirement
         * (the declaration exists) while making the intention clear: CollectiveProperties
         * objects are not assignable. Any attempt to use = on this class will result in a
         * clean compile-time error rather than silent undefined behavior.
         */
        CollectiveProperties<T, E>& operator=(const CollectiveProperties<T, E>& other) = delete;

        ~CollectiveProperties()
        {
            // The destructor is only ever called when refcount is already zero, that is the contract Collective guarantees.

            /*
             *  ~CollectiveProperties()
             *  └─► if (this->data != nullptr)
             *        └─► delete[] this->data
             *              └─► ~T() (for each element)
             *        └─► this->data = nullptr
             */
            if (this->data != nullptr && this->memory_location == MemoryLocation::Host)
            {
                delete[] this->data;
                this->data = nullptr;
            }
            /*
             *  ~CollectiveProperties()
             *  └─► if (this->data != nullptr && this->memory_location == MemoryLocation::Device)
             *        └─► cudaFree(this->data)
             *              └─► this->data = nullptr
             *        └─► this->data = nullptr
             *      (Note: cudaFree is only called if COMPILE_FOR_DEVICE is defined, otherwise it is not even compiled in. This prevents compilation errors in CPU-only builds.)
            */
#ifdef COMPILE_FOR_DEVICE            
            else if (this->data != nullptr && this->memory_location == MemoryLocation::Device)
            {
                // TODO: uncomment when CUDA build is enabled.
                // Cannot call cudaFree in CPU-only build — cuda_runtime.h not included.

                cudaFree(this->data);
                this->data = nullptr;
            }
#endif
            // When this destructor gets called all the destructors of value members will be called automatically.
            // Here that value member is 'dimensions' and the destructor of dimensions will be called automatically.            
        }

        void incrementReferenceCount(void)
        {
            this->reference_count++;
        }

        void decrementReferenceCount(void)
        {
            if (this->reference_count > 0)
            {
                this->reference_count--;
            }
        }

        size_t getReferenceCount(void) const
        {
            return this->reference_count;
        }

        /*
            const Dimensions<E>& getDimensions(void) const
            ├─► if (this->properties == nullptr)
            │     └─► throw std::runtime_error("Collective<T, E>::getShape() Error: CollectiveProperties<T, E> is nullptr")
            └─► return this->properties->getDimensions()

            The method is const, so it cannot return a non-const reference.
            The const ensures that the caller cannot modify the Dimensions object. Returning by reference avoids copying the entire Dimensions object, which can be expensive if it has many nodes.            
         */
        const Dimensions<E>& getDimensions(void) const
        {
            /*
                The method is const, so it cannot return a non-const reference. 
                That is the reason for const qualifier on the return type.

                Why return by reference?
                -------------------------
                Returning by reference avoids copying the entire Dimensions object, which can be expensive if it has many nodes.
                The const ensures that the caller cannot modify the Dimensions object.
            */
            return this->dimensions;
        }

        /*
            *  T* getData(void) const
            *  └─► return this->data
         */
        T* getData(void) const
        {
            return this->data; // Return by pointer to avoid copy
        }

        /**
         *  getMemoryLocation()
         *  └─► return this->memory_location
         */
        MemoryLocation getMemoryLocation(void) const
        {
            return this->memory_location;
        }
};

#endif