/*
    numcy/DimensionsOfArray.hh
    Q@Khaa.pk
 */

/*
    // When you need independent modification
    DimensionsOfArray base_shape;   // [batch, seq, hidden]
    DimensionsOfArray modified_shape(base_shape);  // Deep copy
    // Modify modified_shape without affecting base_shape

    // When you need efficient sharing
    DimensionsOfArray shared_shape;
    shared_shape = base_shape;      // Reference counting
 */ 

#include "header.hh"

#ifndef KHAA_PK_DIMENSIONS_OF_ARRAY_HEADER_HH
#define KHAA_PK_DIMENSIONS_OF_ARRAY_HEADER_HH

struct DimensionsOfArray;

typedef struct DimensionsOfArrayProperties 
{
    //public:
    private:
        cc_tokenizer::string_character_traits<char>::size_type *ptr; // Pointer to an array. The size of this array is "n"
        cc_tokenizer::string_character_traits<char>::size_type n; // The size of the array pointed to by "ptr"

        /*
         * Reference counting tracks the number of active references (pointers or other ways to access) to an object.
         * When an object is first created, there are no references to it, so the count should be 0.
         *  
         * The responsibility of managing the reference count lies with the class itself...
         * the incrementRefernceCount() and decrementReferenceCount() methods and the implementation of 
         * destructor method         
         */       
        unsigned int reference_count; // Reference counting for shared ownership

        friend struct DimensionsOfArray; // To allow DimensionsOfArray to access private members

} DIMENSIONSOFARRAYPROPERTIES;
typedef DIMENSIONSOFARRAYPROPERTIES* DIMENSIONSOFARRAYPROPERTIES_PTR; 

typedef struct DimensionsOfArray
{   
    private:   
        DIMENSIONSOFARRAYPROPERTIES_PTR properties;

    public:
        /**
         * @brief Default constructor for DimensionsOfArray.
         * 
         * Initializes the properties structure by dynamically allocating memory for 
         * DIMENSIONSOFARRAYPROPERTIES and sets initial values:
         * - ptr: NULL (no array data initially)
         * - n: 0 (zero dimensions initially)
         * - reference_count: 0 (no references initially)
         * 
         * @throws ala_exception with descriptive error message if memory allocation fails
         *         (std::bad_alloc) or if length constraints are violated (std::length_error).
         *         The exception message includes the original exception details for debugging.
         * 
         * @note Uses cc_tokenizer::allocator for memory allocation to maintain consistency
         *       with the tokenizer's memory management strategy.
         * @note The allocated properties structure must be properly deallocated in the
         *       destructor to prevent memory leaks.
         */ 
        DimensionsOfArray() throw (ala_exception)
        {             
            try
            {
                this->properties = reinterpret_cast<DIMENSIONSOFARRAYPROPERTIES_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(DIMENSIONSOFARRAYPROPERTIES))); 
                
                this->properties->ptr = NULL;
                this->properties->n = 0;            
                this->properties->reference_count = 0;
            }
            catch (std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::DIMENSIONSOFARRAY() Error: ") + cc_tokenizer::String<char>(e.what())); 
            }
            catch (std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::DIMENSIONSOFARRAY() Error: ") + cc_tokenizer::String<char>(e.what())); 
            }          
        }

        /*
         * Deep Copy Constructor - Creates an independent copy of tensor shape.
         *
         * This constructor performs a DEEP COPY, creating a completely independent 
         * DimensionsOfArray instance with its own memory allocation. Unlike the 
         * assignment operator which shares references, this creates separate storage.
         *
         * Design Intent:
         * - Copy constructor: Creates independent copies (deep copy)
         * - Assignment operator: Shares references (shallow copy with ref counting)
         *
         * Use Cases:
         * - When you need to modify dimensions without affecting the original
         * - When creating temporary tensor shapes for computations
         * - When you want guaranteed memory isolation between objects
         *
         * Example:
         * DimensionsOfArray original_shape;    // [2][512][768]
         * DimensionsOfArray copy_shape(original_shape); // Independent [2][512][768]
         * // Modifying copy_shape won't affect original_shape
         *    
         * DimensionsOfArray shared_shape;
         * shared_shape = original_shape;       // Shares memory via reference counting
         *
         * @param other Source tensor shape to copy from
         * @throws ala_exception if source is uninitialized, invalid, or allocation fails
         *
         * Memory Allocation:
         * - Allocates new properties structure
         * - Allocates new dimension array 
         * - Copies all dimension values
         * - Sets reference_count to 1 (independent object)
         */
        DimensionsOfArray (DimensionsOfArray& other) throw (ala_exception)
        {
            if (other.properties == NULL || other.properties->ptr == NULL)
            {                            
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::DIMENSIONSOFARRAY(DIMENSIONSOFARRAY&) Error: ") +
                                    cc_tokenizer::String<char>("Source tensor shape is uninitialized or invalid"));
            }

            try
            {
                this->properties = reinterpret_cast<DIMENSIONSOFARRAYPROPERTIES_PTR>( cc_tokenizer::allocator<char>().allocate(sizeof(DIMENSIONSOFARRAYPROPERTIES))); 
                
                this->properties->ptr = cc_tokenizer::allocator<cc_tokenizer::string_character_traits<char>::size_type>().allocate(other.size());
                this->properties->n = other.size();            
                this->properties->reference_count = 0;

                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < other.size(); i++)
                {
                    this->properties->ptr[i] = other.properties->ptr[i];
                }
            }
            catch (std::bad_alloc& e)            
            {
                // Clean up partially allocated memory before re-throwing
                if (this->properties != NULL)
                {
                    if (this->properties->ptr != NULL)
                    {
                        cc_tokenizer::allocator<cc_tokenizer::string_character_traits<char>::size_type>().deallocate(this->properties->ptr, other.size());
                    }
                    cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(this->properties), sizeof(DIMENSIONSOFARRAYPROPERTIES));
                }

                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::DIMENSIONSOFARRAY(DIMENSIONSOFARRAY&) Error: ") + cc_tokenizer::String<char>(e.what())); 
            }
            catch (std::length_error& e)
            {
                // Clean up partially allocated memory before re-throwing
                if (this->properties != NULL)
                {
                    if (this->properties->ptr != NULL)
                    {
                        cc_tokenizer::allocator<cc_tokenizer::string_character_traits<char>::size_type>().deallocate(this->properties->ptr, other.size());
                    }
                    cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(this->properties), sizeof(DIMENSIONSOFARRAYPROPERTIES));
                }

                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::DIMENSIONSOFARRAY(DIMENSIONSOFARRAY&) Error: ") + cc_tokenizer::String<char>(e.what())); 
            }          
        }
        
        /**
         * @brief Parameterized constructor for DimensionsOfArray.
         * 
         * Initializes the properties structure with provided array dimensions and reference count.
         * Dynamically allocates memory for DIMENSIONSOFARRAYPROPERTIES and initializes it with:
         * - ptr: Pointer to the array dimensions data (p)
         * - n: Number of dimensions (nn)
         * - reference_count: Reference count for the array (rc), defaults to 0 if not specified
         * 
         * @param p Pointer to an array of size_type values representing the dimensions of the array.
         *          The caller retains ownership of this memory.
         * @param nn The number of dimensions in the array (size of the array pointed to by p).
         * @param rc Optional reference count for the array dimensions (defaults to 0).
         * 
         * @throws ala_exception with descriptive error message if memory allocation fails
         *         (std::bad_alloc) or if length constraints are violated (std::length_error).
         *         The exception message includes the original exception details and constructor
         *         signature for debugging.
         * 
         * @note Uses cc_tokenizer::allocator for memory allocation to maintain consistency
         *       with the tokenizer's memory management strategy.
         * @note This constructor does not copy the dimension data - it only stores the pointer.
         *       The caller is responsible for ensuring the memory pointed to by p remains valid
         *       for the lifetime of this DimensionsOfArray object.
         * @note The allocated properties structure must be properly deallocated in the
         *       destructor to prevent memory leaks.
         */
        DimensionsOfArray(cc_tokenizer::string_character_traits<char>::size_type* p, cc_tokenizer::string_character_traits<char>::size_type nn, cc_tokenizer::string_character_traits<char>::size_type rc = NUMCY_DEFAULT_REFERENCE_COUNT) throw (ala_exception)
        {
            try
            {
                this->properties = reinterpret_cast<DIMENSIONSOFARRAYPROPERTIES_PTR>( cc_tokenizer::allocator<char>().allocate(sizeof(DIMENSIONSOFARRAYPROPERTIES))); 
                
                this->properties->ptr = p;
                this->properties->n = nn;            
                this->properties->reference_count = rc;
            }
            catch (std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::DIMENSIONSOFARRAY(cc_tokenizer::string_character_traits<char>::size_type*, cc_tokenizer::string_character_traits<char>::size_type, cc_tokenizer::string_character_traits<char>::size_type) Error: ") + cc_tokenizer::String<char>(e.what())); 
            }
            catch (std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::DIMENSIONSOFARRAY(cc_tokenizer::string_character_traits<char>::size_type*, cc_tokenizer::string_character_traits<char>::size_type, cc_tokenizer::string_character_traits<char>::size_type) Error: ") + cc_tokenizer::String<char>(e.what())); 
            }                        
        }

        ~DimensionsOfArray(void)
        {  
            if (this->properties != NULL)
            {
                if (this->properties->reference_count > 0)
                {
                    this->properties->reference_count--;
                }

                // If we're the last one
                if (this->properties->reference_count == 0)
                {
                    // We're the last reference holder - clean up everything
            
                    // First, deallocate the dimension array
                    if (this->properties->ptr != NULL)
                    {
                        cc_tokenizer::allocator<cc_tokenizer::string_character_traits<char>::size_type>().deallocate(this->properties->ptr, this->properties->n);
                        this->properties->ptr = NULL;
                    }
            
                    // Then deallocate the properties structure itself
                    cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(this->properties), sizeof(DimensionsOfArrayProperties));
                }
        
                // Always set our pointer to null (safety)
                this->properties = NULL;
            }
        }

        void incrementReferenceCount(void) 
        {
            if (this->properties != NULL)
            {
                this->properties->reference_count++;
            }
        }

        void decrementReferenceCount(void) 
        {
            if (this->properties != NULL && this->properties->reference_count > 0)
            {
                // Decrement reference count and then collect, if we're the last one
                if (--this->properties->reference_count == 0)
                {
                    if (this->properties->ptr != NULL)
                    {
                        cc_tokenizer::allocator<cc_tokenizer::string_character_traits<char>::size_type>().deallocate(this->properties->ptr, this->properties->n);
                        this->properties->ptr = NULL;
                    }
            
                    cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(this->properties), sizeof(DIMENSIONSOFARRAYPROPERTIES));
                }
        
                this->properties = NULL;
            }
        }

        /**
         * @brief Subscript operator for accessing array dimension values.
         * 
         * Provides read-only access to individual dimension values at the specified index.
         * Performs bounds checking to ensure the index is valid within the current dimensions.
         * 
         * @param index Zero-based index of the dimension to access. Must be in range [0, size() - 1].
         * 
         * @return cc_tokenizer::string_character_traits<char>::size_type The value of the dimension at the specified index.
         * 
         * @throws ala_exception if:
         *         - properties structure is NULL (object not properly initialized)
         *         - dimension pointer (ptr) is NULL (no dimensions allocated)
         *         - index is out of bounds (index >= size())
         *         The exception message includes the invalid index and current size for debugging.
         * 
         * @note This operator is const and provides read-only access to dimension values.
         * @note For modifying dimension values, consider providing a separate non-const method
         *       or operator if needed, with appropriate validation.
         * @note The bounds checking ensures safe access but adds a small performance overhead.
         *       For performance-critical code, consider adding an unchecked access method.
         */
        cc_tokenizer::string_character_traits<char>::size_type& operator[](cc_tokenizer::string_character_traits<char>::size_type index) const throw (ala_exception)
        {  
            if (this->properties != NULL && this->properties->ptr != NULL && index < this->size()) 
            {
                return this->properties->ptr[index];
            }
                        
            throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::operator[] Error: ") + 
                                cc_tokenizer::String<char>("Index ") + 
                                cc_tokenizer::String<char>(index) + 
                                cc_tokenizer::String<char>(" out of bounds or invalid tensor shape (size: ") +
                                cc_tokenizer::String<char>(this->size()) + 
                                cc_tokenizer::String<char>(")"));
        }

        /**
         * @brief Copy assignment operator for DimensionsOfArray.
         * 
         * Assigns the properties from another DimensionsOfArray object to this object.
         * Manages reference counting by decrementing the current reference count (if any)
         * and incrementing the reference count of the source object.
         * 
         * @param other The source DimensionsOfArray object to assign from.
         * 
         * @return DimensionsOfArray& Reference to this object after assignment.
         * 
         * @throws ala_exception if either this object's properties or the other object's
         *         properties are NULL (indicating uninitialized objects). The exception
         *         message describes the assignment error.
         * 
         * @note Handles self-assignment correctly by returning early without any operations.
         * @note The reference count management ensures proper shared ownership semantics
         *       for the underlying dimension properties.
         * @note This performs a shallow copy - both objects will share the same underlying
         *       properties structure. Use with caution if deep copy semantics are required.
         * @note The caller must ensure that both objects are properly initialized before
         *       assignment. The destructor should properly handle reference counting
         *       to prevent memory leaks.
         */
        DimensionsOfArray operator= (DimensionsOfArray& other) throw (ala_exception)
        {
            // Self-assignment check
            if (this == &other)
            {
                return *this;
            }

            if (this->properties != NULL && other.properties != NULL)
            {
                this->decrementReferenceCount();
                
                this->properties = other.properties;

                this->incrementReferenceCount();

                return *this;
            }
            
            throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::operator= Error: ") +
                                cc_tokenizer::String<char>("Invalid assignment - source or destination tensor shape is null or uninitialized"));
        }
    
        /**
         * @brief Compares two DIMENSIONSOFARRAY objects based on the specified axis.
         * 
         * Performs a comparison of dimension arrays between this object and another
         * DIMENSIONSOFARRAY object. Currently supports column-wise comparison (AXIS_COLUMN)
         * which checks if all dimensions except the last one are equal.
         * 
         * @param other The DIMENSIONSOFARRAY object to compare against.
         * @param axis The axis along which to perform the comparison. Currently only
         *             AXIS_COLUMN is supported (default).
         * 
         * @return bool true if the dimensions match according to the specified axis
         *              comparison rules, false otherwise.
         * 
         * @throws ala_exception if:
         *         - An unknown axis type is specified
         *         - Any underlying dimension access fails (e.g., out of bounds)
         *         The exception message includes the original error details and method context.
         * 
         * @note For AXIS_COLUMN comparison:
         *       - Both arrays must have the same number of dimensions
         *       - Both arrays must have at least 1 dimension
         *       - All dimensions except the last one must be equal
         *       - The last dimension is ignored in the comparison
         * 
         * @note This is useful for checking if two arrays can be operated upon in column-wise
         *       operations where the last dimension may vary but other dimensions must match.
         * 
         * @note Additional axis types can be implemented by adding new case statements
         *       following the same pattern as AXIS_COLUMN.
         * 
         * @example
         * // Compare if two arrays have matching dimensions for column operations
         * if (dims1.compare(dims2, AXIS_COLUMN)) {
         *     // Safe to perform column-wise operations
         * }
         */
        bool compare(const DimensionsOfArray& other, AXIS axis=AXIS_COLUMN) const throw (ala_exception)
        {
            bool ret = false;
            cc_tokenizer::string_character_traits<char>::size_type i = 0;
            
            try
            {            
                switch (axis)
                {
                    case AXIS_COLUMN:
                        // Must have same number of dimensions and at least 1 dimension
                        if (this->size() == other.size() && this->size() > 0)
                        {   
                             // Compare all dimensions except the last one                    
                            while (i < (this->size() - 1))
                            {
                                if ((*this)[i] != other[i])
                                {
                                    break;
                                }

                                i++;
                            }

                            // If we compared all dimensions except the last, then they match
                            if (i == (this->size() - 1))
                            {                       
                                ret = true;
                            }
                        }
                    break;
                    // Add other AXIS cases here as needed
                    // case AXIS_ROW:
                    //    // Implementation for row-wise comparison
                    // break;
                    default:
                        throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::compare() Error: ") +
                                            cc_tokenizer::String<char>("Unknown axis type"));    
                }
            }
            catch (ala_exception& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::compare() -> ") + 
                                    cc_tokenizer::String<char>(e.what()));
            }
       
            return ret;
        }

        /*
            Number of total number of rows in a array. A multidimentional array has row/s and column/s.
            For example array[1][1] is two dimension array or 1 roW and 1 column, it has 1 inner array and that is the first and the only row.
            In the context of Numcy, the DIMENSIONS composite is used to carry around the dimensions of an array. The implementation of DIMENSIONS 
            is as such that even a single dimension array as array[10] is regarded as array[1][10]. 
            In Numcy a array has atleast two dimensions and treats array[10] as array[1][10]. 
            So array in Numcy has atleast 1 inner array and array[2][3][10] has 6 inner arrays and each inner array has further 10 columns.
        */
        /*
            The following  function is intended to calculate and return the total number of inner arrays within a multidimensional array.

            In NumCy each array has atlease two dimensions.
            -------------------------------------------------
            This comment describes the concept of counting the total number of rows in a multidimensional array, emphasizing how NumCy treats arrays with at least two dimensions and standardizes their representation. 
            In a multidimensional array, we have both rows and columns. For instance, when referring to array[1][1], we are dealing with a two-dimensional array with 1 row and 1 column, essentially containing a single inner array that represents the first and only row.
            In the context of NumCy, the "DIMENSIONS" composite is a construct used to convey the dimensions of an array. NumCy's implementation ensures that even a seemingly single-dimensional array like array[10] is regarded as a two-dimensional array with 1 row and 10 columns, conforming to the NumPy convention.
            Therefore, in NumCy, every array has at least one inner array, and an array like array[2][3][10] contains a total of 6 inner arrays, each of which, in turn, has 10 columns.
            This comment provides a clear explanation of the topic, highlighting the NumCy-specific treatment of array dimensions.        
         */
        /*
         * Calculates the total number of "inner arrays" (rows) in a multidimensional tensor.
         *
         * In your NumCy convention, all arrays have at least 2 dimensions:
         * - array[10] is treated as array[1][10] (1 row, 10 columns)
         * - array[2][3] has 2 rows, each with 3 columns
         * - array[2][3][4] has 2*3 = 6 rows, each with 4 columns
         * - array[2][3][4][5] has 2*3*4 = 24 rows, each with 5 columns
         *
         * The "inner arrays" are the arrays at the deepest level that contain actual data elements.
         * This method multiplies all dimensions EXCEPT the last one (which represents columns).
         *
         * Examples:
         * - Tensor shape [2][3][4]:
         * - Inner arrays = 2 * 3 = 6
         * - Each inner array has 4 elements
         *
         * - Tensor shape [batch=2][seq=128][hidden=768]:
         * - Inner arrays = 2 * 128 = 256  
         * - Each inner array has 768 elements
         *
         * @return Total number of inner arrays (rows) in the tensor
         * @throws ala_exception if tensor is uninitialized or malformed
         */
        cc_tokenizer::string_character_traits<char>::size_type getNumberOfInnerArrays(void) const throw (ala_exception)
        {
            if (this->properties == NULL || this->properties->ptr == NULL)
            {
                throw ala_exception("DIMENSIONSOFARRAY::getNumberOfInnerArrays() Error: This instance is badly formed.");
            }
         
            /*
                Initialize to 1 (multiplicative identity) for correct dimension multiplication
                - Ensures first multiplication ret * dimension[0] works correctly
                - Handles edge case of 2D arrays properly
                - Result should never be 0 unless the value at index 0 of the array is 0
             */
            cc_tokenizer::string_character_traits<char>::size_type ret = 1;

            try 
            {
                // Loop through the dimensions
                // ptr[n - 1] has number of columns per inner array, exclude that
                /*
                    The function then enters a loop that iterates through the dimensions.
                    The loop starts from i = 0 and goes up to (n - IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray).
                    Inside the loop, it multiplies the current value of ret by the value of ptr[i], which represents the size of the current dimension.
                 */
                if (this->size() >= IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray)
                {
                    // Multiply all dimensions except the last one
                    // Last dimension represents columns per inner array
                    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i <= (this->size() - IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray); i++)
                    {                
                        ret = ret * (*this)[i]; 
                    }
                }   
            }
            catch (ala_exception& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::getNumberOfInnerArrays() -> ") + 
                                    cc_tokenizer::String<char>(e.what()));
            }
                    
            return ret;
        }

        /*
            getNumberOfInnerArraysActual()
    
            Purpose:
            Returns the number of inner arrays (i.e., the size of the penultimate dimension)
            when the array has 2 or more dimensions. This represents the count of sub-arrays
            contained within each element of the outermost dimension.
        
            Behavior by number of dimensions:
                - For a 3D (or higher) array, e.g., [2][3][19]: returns 3  
                  (each of the 2 outermost elements contains 3 inner arrays of size 19)
                - For a 2D array, e.g., [2][3]: returns 2  
                  (each of the 2 rows contains 3 elements – the "inner arrays" are of size 2 when viewed from the outer perspective)
                - Return 0 for any invalid array shape (size < 2  
                    
            Implementation notes:
                - Dimensions are assumed to be stored in the container from outermost (index 0)
                  to innermost (index size()-1).
                - When size() >= 2, the penultimate dimension is accessed at index size() - 2.
                - The constant IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray
                  is defined as 2 to achieve this indexing and to guard against access when fewer
                  than 2 dimensions are present.
                - The default return value of 0 serves as an error for size < 2.
    
            Throws:
                ala_exception if the instance is not properly initialized
                (properties == NULL or properties->ptr == NULL or properties->n < 2).
                Also propagates any ala_exception thrown by operator[] with a prefixed message
                for better diagnostics.
         */
        cc_tokenizer::string_character_traits<char>::size_type getNumberOfInnerArraysActual(void) const throw (ala_exception)
        {
            if (this->properties == NULL || this->properties->ptr == NULL || this->size() < IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray)
            {
                throw ala_exception("DIMENSIONSOFARRAY::getNumberOfInnerArraysActual() Error: This instance is badly formed.");
            }
          
            cc_tokenizer::string_character_traits<char>::size_type ret = 0;

            try 
            {                                
                ret = (*this)[this->size() - IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray];                 
            }
            catch (ala_exception& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::getNumberOfInnerArraysActual() -> ") + cc_tokenizer::String<char>(e.what()));
            }
                    
            return ret;
        }

        /**
         * @brief Returns the size of the innermost array dimension.
         *
         * This function retrieves the dimension value for the innermost (highest-rank)
         * array dimension. In a multi-dimensional array declaration such as
         * `int arr[d1][d2][d3][d4]`, the container holds the dimensions in order
         * {d1, d2, d3, d4}, and the innermost size is the last element (d4).
         *
         * The function performs additional validation beyond basic null/empty checks:
         * it ensures the container has at least the minimum number of dimensions
         * required by the specific use case
         * (IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray).
         *
         * @return The size of the innermost dimension as a size_type.
         *
         * @throws ala_exception If the instance is invalid:
         *         - properties or properties->ptr is NULL,
         *         - the container is empty, or
         *         - size() is less than IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray.
         *           Also throws a wrapped ala_exception if operator[] access fails
         *           (e.g., out-of-bounds, though the prior size check should prevent this).
         */
        cc_tokenizer::string_character_traits<char>::size_type getSizeOfInnerMostArray(void) const throw (ala_exception)
        {
            if (this->properties == NULL || this->properties->ptr == NULL || this->size() < IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray)
            {
                throw ala_exception("DIMENSIONSOFARRAY::getSizeOfInnerMostArray() Error: This instance is badly formed.");
            }
          
            cc_tokenizer::string_character_traits<char>::size_type ret = 0;

            try 
            {                                
                ret = (*this)[this->size() - 1];                 
            }
            catch (ala_exception& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONSOFARRAY::getSizeOfInnerMostArray() -> ") + cc_tokenizer::String<char>(e.what()));
            }
                    
            return ret;
        }
                
        /*
         * Returns the rank (number of dimensions) of the tensor shape.
         * 
         * For example:
         * - A 1D array [10] has rank 1
         * - A 2D matrix [3][4] has rank 2  
         * - A 3D tensor [2][3][4] has rank 3
         * - A 4D tensor [batch][height][width][channels] has rank 4
         *
         * The actual dimension sizes can be accessed using the [] operator:
         * for (size_t i = 0; i < shape.size(); ++i) {
         *   std::cout << "Dimension " << i << ": " << shape[i] << std::endl;
         * }
         *
         * Example output for a [2][3][4] tensor:
         * Dimension 0: 2
         * Dimension 1: 3  
         * Dimension 2: 4
         *
         * @return The number of dimensions (rank) of the tensor, or 0 if uninitialized
         * 
         * Note: This method differs from getDimensionsOfArray() which returns the actual
         * dimension values. This method only returns how many dimensions exist.
         */
        cc_tokenizer::string_character_traits<char>::size_type size(void) const 
        {
            if (this->properties != NULL)
            {
                return this->properties->n;
            }

            return 0;
        }

        cc_tokenizer::string_character_traits<char>::size_type get_n(void) const
        {
            return size();
        }

} DIMENSIONSOFARRAY;

typedef DIMENSIONSOFARRAY* DIMENSIONSOFARRAY_PTR;

#endif