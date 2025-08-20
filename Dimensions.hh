/*
    Numcy/Dimensions.hh
    Q@khaa.pk
 */

//#include "../ala_exception/ala_exception.hh"
//#include "../String/src/allocator.hh"
//#include "../String/src/string_character_traits.hh"

//#include "DimensionsOfArray.hh"
#include "header.hh"

#ifndef KHAA_PK_DIMENSIONS_HEADER_HH
#define KHAA_PK_DIMENSIONS_HEADER_HH

/*
    To summarize, in a C-style array represented by a linked list of DIMENSIONS structures:
    - Each structure represents a specific dimension of the multi-dimensional array.
    - The next and prev pointers allow traversal between the dimensions.
    - The last DIMENSIONS structure holds the dimensions of the innermost dimension of the array, and the columns member stores the number of elements or columns in that dimension.
    - The columns member in the last link is non-zero to indicate the size of the innermost dimension.
    - The prev pointer of the first link and the next pointer of the last link are both set to NULL to indicate the start and end of the linked list, respectively.
 */
typedef struct Dimensions
{    
    //private:
        cc_tokenizer::string_character_traits<char>::size_type columns;
        cc_tokenizer::string_character_traits<char>::size_type rows;
        struct Dimensions* next;
        struct Dimensions* prev;

        //unsigned int reference_count;
        cc_tokenizer::string_character_traits<char>::size_type reference_count;


    public:

    // Default constructor
    Dimensions (void) : columns(0), rows(0), next(NULL), prev(NULL), reference_count(0) 
    {

    }

    /**
     * @brief Constructs a linked list of Dimensions objects from an array of sizes.
     *
     * This constructor initializes the current Dimensions object and, if the
     * provided array contains more than one size, dynamically allocates and links
     * additional Dimensions objects to represent a multi-dimensional structure.
     *
     * Behavior:
     * - Initializes rows, columns, reference count, and linkage pointers (next/prev).
     * - The first element of the array is assigned to `rows` of the current object.
     * - For each subsequent element (except the last), a new Dimensions node is
     *   allocated, linked as `next`, and assigned its `rows` value.
     * - The final element of the array is assigned to `columns` of the last node.
     *
     * Memory Management:
     * - Uses `cc_tokenizer::allocator<char>` for dynamic allocation of Dimensions
     *   objects, cast to the appropriate type via `reinterpret_cast`.
     * - Establishes a doubly-linked list structure (`next` and `prev`).
     *
     * @param dimensionsOfArray A container (DIMENSIONSOFARRAY) holding integer-like
     *        values where:
     *        - The first element defines the `rows` of the root object.
     *        - Intermediate elements define the `rows` of linked nodes.
     *        - The last element defines the `columns` of the final node.
     */
    Dimensions (DIMENSIONSOFARRAY& dimensionsOfArray) : columns(0), rows(0), next(NULL), prev(NULL), reference_count(0)  
    {        
        if (dimensionsOfArray.size() > 0)
        {
            cc_tokenizer::string_character_traits<char>::size_type i = 0;            
            Dimensions* current = NULL;

            while (i < (dimensionsOfArray.size() - 1)) 
            {
                if (i == 0)
                {
                    this->rows = dimensionsOfArray[0];
                }
                else
                {
                    if (this->next == NULL)
                    {
                        this->next = reinterpret_cast<Dimensions*>(cc_tokenizer::allocator<char>().allocate(sizeof(DIMENSIONS)));

                        current = this->next;

                        current->next = NULL;
                        current->prev = this;
                        current->rows = dimensionsOfArray[1];
                        current->columns = 0;                        
                    }
                    else 
                    {
                        current->next = reinterpret_cast<Dimensions*>(cc_tokenizer::allocator<char>().allocate(sizeof(DIMENSIONS)));
                        current->next->prev = current;
                        current = current->next;

                        current->next = NULL;
                        current->rows = dimensionsOfArray[i];
                        current->columns = 0;                        
                    }
                }

                i = i + 1;
            }

            current->columns = dimensionsOfArray[dimensionsOfArray.size() - 1];                        
        }       
    }

    // Copy constructor, the caller should have called the incrementReferemceCount() already
    Dimensions(Dimensions& ref) : columns(ref.columns), rows(ref.rows), next(ref.next), prev(ref.prev), reference_count(ref.reference_count)
    {

    }
        
    Dimensions(cc_tokenizer::string_character_traits<char>::size_type c, cc_tokenizer::string_character_traits<char>::size_type r, Dimensions* n, Dimensions* p) : columns(c), rows(r), next(n), prev(p), reference_count(0)
    {
    }

    // The caller should have called the incrementReferemceCount() already
    Dimensions(Dimensions* ref) : columns(ref->columns), rows(ref->rows), next(ref->next), prev(ref->prev), reference_count(ref->reference_count)
    {
    }

    /**
     * @brief Destructor for the Dimensions class.
     *
     * Cleans up dynamically allocated memory associated with the linked list
     * of Dimensions objects that may have been created by the constructor.
     *
     * Behavior:
     * - If `next` is non-null, the destructor traverses to the last linked
     *   Dimensions node.
     * - It then deallocates nodes in reverse order (from last to first),
     *   using `cc_tokenizer::allocator<char>().deallocate()`.
     * - Traversal stops once it reaches the root (the current object).
     *
     * Notes:
     * - Ensures that all dynamically allocated linked nodes are released.
     * - Uses `reinterpret_cast<char*>` when calling the allocator's `deallocate`
     *   to match the allocation style used in the constructor.
     *
     * Potential Risks / Considerations:
     * - Manual memory management: If allocation/deallocation logic changes,
     *   mismatches can cause memory leaks or undefined behavior.
     * - The root object (`this`) is not deallocated here—only its linked children.
     * - Care must be taken to avoid double-deallocation if multiple destructors
     *   or cleanup routines are called.
     *
     * Alternative Approach (commented out in code):
     * - Iterative forward traversal with deallocation at each step (simpler but
     *   less cache-friendly than the current reverse-deletion approach).
     */
    ~Dimensions()
    {          
        Dimensions* current = next;
                
        if (next != NULL)
        {
            // Go to the last link of the linked list
            while (1)
            {
                if (current->next == NULL)
                {
                    break;
                }

                current = current->next;
            }

            // Delete the links in reverse order    
            while (1)
            {
                Dimensions* local = current->prev;

                cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(current), sizeof(DIMENSIONS));

                if (local == NULL || local == this)
                {
                    break;
                }

                current = local;
            }
        }
                
        /*if (next != NULL)
        {
            Dimensions* current = next;

            while (1)
            {
                Dimensions* local = current;
                current = current->next;

                cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(local));

                if (current == NULL)
                {
                    break;
                }    
            }                
        }*/

        //std::cout<<"In the destructor...."<<std::endl;        
    }

        /*
            As the object is assigned to variables or passed around, the reference count is incremented (meaning there's one more way to access it).
         */
        void incrementReferenceCount(void) 
        {
            reference_count++;
        }
        /*
            When a reference goes out of scope or is explicitly released, the reference count is decremented.
         */
        void decrementReferenceCount(void)
        {
            if (reference_count != 0)
            {
                reference_count--;
            }

            /*
                Double Deletion: Make sure that you only call decrementReferenceCount() method
                when you atleast have one more reference/pointer to this object. Otherwise double deletion
                may occur,  You explicitly call the destructor which frees the resources and then the same 
                destructor gets called automatically when the object which had no references/pointers to begin wuth goes 
                out of scope.
             */
            /*
                Double Deletion Warning...
                Be cautious when calling decrementReferenceCount() within this function.
                Ensure there's at least one more reference (pointer) to the object before decrementing the
                count. Otherwise, double deletion might occur.

                This happens because...
                - You explicitly call the destructor (delete this;), which frees the object's resources.
                - However, the object's destructor might also be called automatically when the object
                  (with no remaining references) goes out of scope. This can lead to the same memory location being deleted twice.
             */
            if (reference_count == 0)
            {
                delete this;
            }

            /*
                When the reference count reaches zero in decrementReferenceCount, the object itself goes out of scope.
                This triggers the automatic invocation of the destructor, which then takes care of deleting 
                the memory and performing any necessary cleanup tasks.
             */           
        }

    /*
        Creates copy of "instance reference"/"self reference" of type DIMENSIONS(It goes through the whole linkedlist and creates a copy).
        Returns pointer to newly created copy of the context.
     */
    Dimensions* copy(void)
    {
        Dimensions *ret = NULL, *current = NULL;
        cc_tokenizer::string_character_traits<char>::size_type n = getN();
        cc_tokenizer::allocator<char> alloc_obj;
        
        for (size_t i = 0; i < getDimensionsOfArray().size() - 1; i++)
        {
            if (ret == NULL)
            {
                ret = reinterpret_cast<Dimensions*>(alloc_obj.allocate(sizeof(Dimensions)));
                ret->columns = 0;
                ret->rows = getDimensionsOfArray()[i];
                ret->next = NULL;
                ret->prev = NULL;
                /* Wrong doing */
                //*ret = Dimensions {0, getDimensionsOfArray()[i], NULL, NULL};

                current = ret;
                
                current->reference_count = 0;
            }
            else
            {
                current->next = reinterpret_cast<Dimensions*>(alloc_obj.allocate(sizeof(Dimensions)));
                current->next->columns = 0;
                current->next->rows = getDimensionsOfArray()[i];
                current->next->next = NULL;
                current->next->prev = current;
                /* Wrong doing */
                //*(current->next) = Dimensions {0, getDimensionsOfArray()[i], NULL, current}; 

                current = current->next;

                current->reference_count = 0;
            }
        }

        current->columns = getDimensionsOfArray()[getDimensionsOfArray().size() - 1];
        
        return ret;
    }

    /*                
        This method, named 'getN', is designed to traverse a linked list of 'Dimensions' structures, each representing a dimension of a multidimensional array.
        It calculates and returns the total number of indices an array can hold, which essentially indicates the total number of elements within the array.        
     */
    cc_tokenizer::string_character_traits<char>::size_type getN(void)
    {
        cc_tokenizer::string_character_traits<char>::size_type n = 1;
        struct Dimensions* current = this;

        while (1)
        {
            if (current == NULL)
            {
                break;
            }

            n = current->rows * n;
        
            if (current->next == NULL)
            {
                n = current->columns * n;
            }

            current = current->next;
        }

        return n;
    }

    /*
        Please note, before calling this method make sure that linked list is properly formed.
        It simply returns the size of the inner most array(a.k.a the number of columns)
        All arrays are two dimensional(atleast), for example... array[10] is an array of 1 line and 10 columns array.
     */
    cc_tokenizer::string_character_traits<char>::size_type getNumberOfColumns(void)
    {
        cc_tokenizer::string_character_traits<char>::size_type n = 0;
        struct Dimensions* current = this;

        while (1)
        {
            if (current == NULL)
            {
                break;
            }

            n = current->columns;

            current = current->next;
        }

        return n;
    }

    /*
        "In the case of a single dimentional array, it returns 1" PLEASE NOTE: this was old behaviour. I modified a for loop in method DIMENSIONSOFARRAY::getNumberOfInnerArrays() 
        In the case of multidimentional array it wll eturn the number of inner arrays in number of single dimension arrays e.g [2][3][5] is 6 single dimensions array with each row having 5 columns 
     */
    DIMENSIONSOFARRAY getNumberOfRows(void)
    {
        return getDimensionsOfArray();
    }

    /*         
        Just an example of creating a linked list of DIMENSIONS
       ---------------------------------------------------------
        DIMENSIONS dim3 = {10, 3, NULL, NULL};
        DIMENSIONS dim2 = {0, 10, &dim3, NULL};
        dim3.prev = &dim2;
        DIMENSIONS dim1 = {0, 78, &dim2, NULL};
        dim2.prev = &dim1;
        DIMENSIONS dim = {0, 9, &dim1, NULL};
        dim1.prev = &dim;
        Returns compoite DIMENIONSOFARRAY, a struct data type. It returns the number of dimensions a array has and 
        the size of each dimension of the array, including the size of innermost array(a.k.a the number of columns) 
     */
    DIMENSIONSOFARRAY getDimensionsOfArray(void) throw (ala_exception)
    {
        cc_tokenizer::string_character_traits<char>::size_type n = getNumberOfLinks();

        /*
            The last link of linked list has number of columns of the inner most array, that is why this method would return atleast 2 as the dimensions of the array and it implies having both rows and columns in the last link of either multilink or single link list.
         */
        if (n != 0)
        {
            n = n + 1;
        }
        else
        {
            throw ala_exception("Dimensions::gteDimensionsOfArray() Error: Malformed shape of array, link count is 0");
        }

        cc_tokenizer::string_character_traits<char>::size_type *ptr = reinterpret_cast<cc_tokenizer::string_character_traits<char>::size_type*>(cc_tokenizer::allocator<char>().allocate(n*sizeof(cc_tokenizer::string_character_traits<char>::size_type)));

        cc_tokenizer::string_character_traits<char>::size_type i = 0;
        Dimensions* current = this;

        while (1)
        {
            ptr[i] = current->rows;

            if (current->next == NULL)
            {
                ptr[i + 1] = current->columns;

                break;
            }
            
            current = current->next;

            i = i + 1;
        }

        return {ptr, n};
    }

    /*        
        Returns number of links in a linked list and this number would atleast be 1 because each linked list has a head where the previous pointer is NULL.
     */
    cc_tokenizer::string_character_traits<char>::size_type getNumberOfLinks(void)
    {
        cc_tokenizer::string_character_traits<char>::size_type n = 0;
        struct Dimensions* current = this;

        while (current != NULL)
        {
            n++;
            current = current->next;                 
        }

        return n;
    }

    Dimensions& operator= (Dimensions& other)
    {
        // Check for self-assignment
        if (this == &other)
        {            
            return *this;    
        }

        /*
            Placement new operator is used to create a new object at the same memory location as the current object.
            This is done to avoid memory leaks and to ensure that the current object is properly destructed before creating a new object.

            Lets hope it works as expected. I used it as it looks cool and it is a good practice to use it.
         */
        // Delete the current object
        this->~Dimensions();
        // Create a new object with the same values as the other object
        new (this) Dimensions (other);
        
        /* 
            If things start to go wrong, you can always revert back to the old way of doing things.            
         */
        /*
            this->columns = other.columns;
            this->rows = other.rows;

            this->next = other.next;
            this->prev = other.prev;
         */ 
        
         /*Dimensions* current = next;
                
         if (next != NULL)
         {
             // Go to the last link of the linked list
             while (1)
             {
                 if (current->next == NULL)
                 {
                     break;
                 }
 
                 current = current->next;
             }
 
             // Delete the links in reverse order    
             while (1)
             {
                 Dimensions* local = current->prev;
 
                 cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(current), sizeof(DIMENSIONS));
 
                 if (local == NULL || local == this)
                 {
                     break;
                 }
 
                 current = local;
             }
         }*/
         
            /*
            this->columns = other.columns;
            this->rows = other.rows;

            this->next = other.next;
            this->prev = other.prev;
             */
                  
        return *this;        
    }
    
    /*
        Overloaded [] operator
        @index, when it is 0(NUMCY_DIMENSIONS_SHAPE_ROWS), value of DIMENSIONS::rows is returned
                when it is 1(NUMCY_DIMENSIONS_SHAPE_COLUMNS), value of DIMENSIONS::columns is returned
     */   
    cc_tokenizer::string_character_traits<char>::size_type operator[](cc_tokenizer::string_character_traits<char>::size_type index) const
    {   
        cc_tokenizer::string_character_traits<char>::size_type ret = 0;

        switch (index) 
        {
            case NUMCY_DIMENSIONS_SHAPE_ROWS:
                ret = rows;
            break;
            case NUMCY_DIMENSIONS_SHAPE_COLUMNS:
                ret = columns;
            break;
        }

        return ret;     
    }

    /*        
        An overloaded equality operator (==) implementation for comparing two Dimensions objects for equality.
        @other, instance of Dimensions composite of the two which needs the following comparision

        Return Value:
        If no discrepancies and both Diemsions objects are equal then it returns true indicating that the two Dimensions objects are equal.
     */    
    bool operator==(Dimensions& other) 
    {
        /*
            It starts by comparing the number of dimensions (N)
         */
        if (getN() != other.getN())
        {
            return false;
        }

        /*
            Check number of links, for both it should be same
         */
        if (getNumberOfLinks() != other.getNumberOfLinks())
        {
            return false;
        }

        /*
            Iterative Comparison: 
            It then iterates over each dimension in a linked list fashion.
            For each dimension, it compares the number of rows and columns. If any pair of rows/columns are not equal between the two Dimensions objects, it returns false.
         */    
        Dimensions* receiver = this;
        Dimensions* other_receiver = &other;

        do {

            if (receiver->rows != other_receiver->rows || receiver->columns != other_receiver->columns)
            {
                return false;
            }

            receiver = this->next;
            other_receiver = other_receiver->next;

        } while (receiver != NULL && other_receiver != NULL); // It iterates through the linked list until either one of the linked lists ends.

        return true;
    }

} DIMENSIONS;

typedef Dimensions* DIMENSIONS_PTR;
typedef const Dimensions* CONST_DIMENSIONS_PTR;

#endif