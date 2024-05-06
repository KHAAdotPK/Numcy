/*
    numcy/DimensionsOfArray.hh
    Q@Khaa.pk
 */

#include "header.hh"

#ifndef KHAA_PK_DIMENSIONS_OF_ARRAY_HEADER_HH
#define KHAA_PK_DIMENSIONS_OF_ARRAY_HEADER_HH

//#include "header.hh"

typedef struct DimensionsOfArray
{   
    // The properties are private, "initializer-lists" will work only if it has a repective overloaded constructor
    private:    
        cc_tokenizer::string_character_traits<char>::size_type *ptr; // Pointer to an array. The size of this array is "n"
        cc_tokenizer::string_character_traits<char>::size_type n; // The size of the array pointed to by "ptr"

        unsigned int reference_count;

    public: 
        /* We need all different overloaded constructors, to support "initializer-lists" or else make all properties public */   
        DimensionsOfArray(cc_tokenizer::string_character_traits<char>::size_type* p, cc_tokenizer::string_character_traits<char>::size_type nn) : ptr(p), n(nn), reference_count(1)
        {
        }

        ~DimensionsOfArray(void)
        {

            if (reference_count)
            {
                return;
            }
            
            if (ptr != NULL)
            {                            
                cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(ptr));
                
                ptr = NULL;
                n = 0;                
                // Just for documentation purposes
                reference_count = 0;                
            }            
        }

        void incrementReferenceCount(void) 
        {
            reference_count++;
        }

        void decrementReferenceCount(void)
        {
            reference_count--;

            if (reference_count == 0)
            {
                delete this;
            }
        }

        cc_tokenizer::string_character_traits<char>::size_type operator[](cc_tokenizer::string_character_traits<char>::size_type index) const
        {   
                    
            return ptr[index];     
        }
    
        /*
            Compares two instances together
            @other, reference to the other instance
            @axis, can have any value defined for enum type named AXIS        
                   dfault is AXIS_COLUMN
            AXIS_COLUMN Will returns true if both arrays have same dimensions other than the dimensions of the inner most array. 
                        For example... a[2][3][4][5] and b[2][3][4][2], only the inner most array ([5], [2]) is different
         */
        bool compare(DimensionsOfArray& other, AXIS axis=AXIS_COLUMN)
        {
            bool ret = false;
        
            switch (axis)
            {
                case AXIS_COLUMN:
                    if (n == other.n)
                    {
                        cc_tokenizer::string_character_traits<char>::size_type i = 0;

                        while (i < (n - 1))
                        {
                            if (ptr[i] != other.ptr[i])
                            {
                                break;
                            }

                            i++;
                        }

                        if (i == (n - 1))
                        {                       
                            ret = true;
                        }
                    }
                break;
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
        cc_tokenizer::string_character_traits<char>::size_type getNumberOfInnerArrays(void) throw (ala_exception)
        {
            // Check if the instance is badly formed (ptr is NULL or n is 0)
            if (ptr == NULL || n == 0)
            {
                throw ala_exception("DimensionsOfArray::getNumberOfInnerArrays() Error: This instance is badly formed.");
            }

            // Initialize a variable 'ret' to 1; it will store the result
            cc_tokenizer::string_character_traits<char>::size_type ret = 1;

            // Loop through the dimensions
            // ptr[n - 1] has number of columns per inner array, exclude that
            /*
                The function then enters a loop that iterates through the dimensions.
                The loop starts from i = 0 and goes up to (n - IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray).
                Inside the loop, it multiplies the current value of ret by the value of ptr[i], which represents the size of the current dimension.
             */
            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i <= (n - IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray); i++)
            {
                /*std::cout<<"ptr[i]"<<ptr[i]<<std::endl;*/
                ret = ret * ptr[i]; 
            }

            /*std::cout<<"ret = "<<ret<<std::endl;*/
        
            return ret;
        }
        
        /*                
            Returns the size of the n-dimensional array, representing its actual dimensions.
            In this context, the array[][][] is a three-dimensional array, and this method
            returns this physical number of dimensions, which as above mentioned is three.

            The actual dimensions of array are...

            for i is 0 to (size() - 1) {

                std::cout<< (*this)[i] << " "; 
            }

            @return the he size, i.e., the number of physical dimensions of the array.
         */
        cc_tokenizer::string_character_traits<char>::size_type size(void) 
        {

            return n;
        }

} DIMENSIONSOFARRAY;

typedef DIMENSIONSOFARRAY* DIMENSIONSOFARRAY_PTR;

#endif