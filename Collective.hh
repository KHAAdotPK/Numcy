/*
    numcy/Collective.hh
    Q@khaa.pk
 */

#include "header.hh"

#ifndef KHAA_PK_COLLECTIVE_HEADER_HH
#define KHAA_PK_COLLECTIVE_HEADER_HH

template <typename E>
struct Collective; 

template <typename E = double>
struct CollectiveProperties 
{
    CollectiveProperties() throw (ala_exception)
    {   
        try
        {                     
            this->ptr = NULL;
            this->shape = new DIMENSIONS{0, 0, NULL, NULL};
            this->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT;
        }
        catch (ala_exception& e)
        {
            // Propagate existing ala_exception with additional context
            throw ala_exception(cc_tokenizer::String<char>("CollectiveProperties::CollectiveProperties() -> ") + e.what());
        }
    }

    CollectiveProperties (E* v, DIMENSIONS& like)     
    {
        try
        {        
            this->ptr = v;
            like.incrementReferenceCount();        
            this->shape = new DIMENSIONS{like.getNumberOfColumns(), like.getNumberOfRows(), like.getNext(), like.getPrev()}; 
            this->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT;
        }
        catch (ala_exception& e)
        {
            // Propagate existing ala_exception with additional context
            throw ala_exception(cc_tokenizer::String<char>("CollectiveProperties::CollectiveProperties(E*, DIMENSIONS&) -> ") + e.what());
        }
    }

    private:
        E* ptr;
        DIMENSIONS_PTR shape;
        cc_tokenizer::string_character_traits<char>::size_type reference_count; 
        
    friend struct Collective<E>;
};

/*  
    The following composite is named "Collective", the instance of this composite
    is a collection of array of type "E" and its shape. 
 */
template <typename E = double>
struct Collective 
{
    private:
        E* ptr;
                           
        DIMENSIONS shape;

        /*
         * Reference counting tracks the number of active references (pointers or other ways to access) to an object.
         * When an object is first created, there are no references to it, so the count should be 0.
         *  
         * The responsibility of managing the reference count lies with the class itself...
         * the incrementRefernceCount() and decrementReferenceCount() methods and the implementation of 
         * destructor method         
         */       
        cc_tokenizer::string_character_traits<char>::size_type reference_count;

        CollectiveProperties<E>* properties;
       
    public:
        // Default constructor
        Collective (void)
        {             
            try
            {
                this->properties = new CollectiveProperties<E>();
            }
            /*catch (std::bad_alloc& e)
            {
                // CRITICAL: Memory allocation failure - system should terminate immediately
                // NO cleanup performed - this is a fatal error requiring process exit
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective<E>(void) Error: ") + cc_tokenizer::String<char>(e.what())); 
            }
            catch (std::length_error& e)
            {
                // CRITICAL: Length constraint violation - system should terminate immediately
                // NO cleanup performed - this is a fatal error requiring process exit
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective<E>(void) Error: ") + cc_tokenizer::String<char>(e.what())); 
            }*/
            catch (ala_exception& e)
            {
                // Propagate existing ala_exception with additional context
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective<E>(void) -> ") + e.what());
            }             
        }
            
        /*                            
            Special Case: When 'v' is NULL but 'like' has a valid shape.
            This allows creating a Collective object with a defined shape but no allocated data.
    
                Example 1: Direct Initialization
                           Collective<double> W1 = Collective<double>{NULL, DIMENSIONS{n, m, NULL, NULL}};
    
                Example 2: Assignment After Declaration
                           Collective<E> W1;
                           W1 = Collective<double>{NULL, DIMENSIONS{n, m, NULL, NULL}};
    
                Use Case: This is useful for creating placeholder objects or delaying memory allocation.
                 
            This constructor properly:
            1. v != NULL + valid dimensions → allocate and copy data
            2. v == NULL + valid dimensions → shape-only initialization
            3. Both invalid → default empty state
            4. v != NULL + invalid dimensions → throw exception

         */
        Collective (E* v, DIMENSIONS& like) throw (ala_exception)
        {   
            try 
            {
                if (like.getN() > 0)
                {
                    this->properties = new CollectiveProperties<E> (v, like);
                }
                else 
                {
                    this->properties = new CollectiveProperties<E> ();
                }
            }
            catch (ala_exception& e)
            {
                // Propagate existing ala_exception with additional context
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSIONS&) -> ") + e.what()); 
            }
            
            /*try
            {                   
                if (v != NULL)
                {                       
                    if (like.getN())
                    {                                                                        
                        this->properties = new CollectiveProperties<E> (v, like);
                    }
                    else 
                    {   
                        // Propagate existing ala_exception with additional context
                        //throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSIONS&) -> ") + e.what());
                    }                                         
                }*/             
                /*
                    Special Case: When 'v' is NULL but 'like' has a valid shape.
                    This allows creating a Collective object with a defined shape but no allocated data.
    
                    Example 1: Direct Initialization
                               Collective<double> W1 = Collective<double>{NULL, DIMENSIONS{n, m, NULL, NULL}};
    
                    Example 2: Assignment After Declaration
                               Collective<E> W1;
                               W1 = Collective<double>{NULL, DIMENSIONS{n, m, NULL, NULL}};
    
                    Use Case: This is useful for creating placeholder objects or delaying memory allocation.
                 */                    
                /*else if (like.getN()) // v is NULL
                {                    
                    //this->properties->ptr = NULL;
                    //this->properties->shape = like;
                    //this->properties->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT;
                }
                else
                {
                    //this->properties->ptr = NULL;
                    //this->properties->shape = DIMENSIONS {0, 0, NULL, NULL};
                    //this->properties->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT;
                }*/                
            /*}
            catch (std::bad_alloc& e)
            {
                // CRITICAL: Memory allocation failure - system should terminate immediately
                // NO cleanup performed - this is a fatal error requiring process exit
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSIONS&) Error: ") + cc_tokenizer::String<char>(e.what())); 
            }
            catch (std::length_error& e)
            {
                // CRITICAL: Length constraint violation - system should terminate immediately
                // NO cleanup performed - this is a fatal error requiring process exit
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSIONS&) Error: ") + cc_tokenizer::String<char>(e.what())); 
            } 
            catch (ala_exception& e)
            {
                // Propagate existing ala_exception with additional context
                // NO cleanup performed assuming this is also a critical error
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSIONS&) -> ") + cc_tokenizer::String<char>(e.what()));
            }*/            
        }

        /**
         * Constructor for the Collective class.
         *
         * This constructor initializes the object's member variables:
         *  - `ptr`: Pointer to data (of type E*)
         *  - `shape`: Shape information (of type DIMENSIONS)
         *  - `reference_count`: Initial reference count (set to the provided `rc`)
         *
         * **Important Note:** It's crucial to avoid explicitly calling `delete this;` within the constructor 
         * or the `decrementReferenceCount` function. This is because the object's memory will be 
         * automatically managed through the destructor when the reference count reaches zero. Explicitly 
         * deleting the object within these methods leads to "double deletion" and program crashes.
         *
         * @param v Pointer to data (of type E*)
         * @param like Shape information (of type DIMENSIONS)
         * @param rc Initial reference count
         */
        Collective (E* v, DIMENSIONS& like, cc_tokenizer::string_character_traits<char>::size_type rc) throw (ala_exception) 
        {               
            try
            { 
                //this->properties = reinterpret_cast<CollectiveProperties<E>*>(cc_tokenizer::allocator<char>().allocate(sizeof(CollectiveProperties<E>)));

                if (v != NULL)
                {                        
                    if (like.getN())
                    {
                        /*this->properties->ptr = v;
                        this->properties->shape = like;
                        this->properties->reference_count = rc;*/    
                    }
                    else 
                    { 
                        // NO cleanup performed assuming this is also a critical error                                               
                        throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSIONS&, cc_tokenizer::string_character_traits<char>::size_type) Error: Cannot initialize with data when dimensions specify zero elements"));
                    }                                         
                }             
                /*
                    Special Case: When 'v' is NULL but 'like' has a valid shape.
                    This allows creating a Collective object with a defined shape but no allocated data.
    
                    Example 1: Direct Initialization
                               Collective<double> W1 = Collective<double>{NULL, DIMENSIONS{n, m, NULL, NULL}};
    
                    Example 2: Assignment After Declaration
                               Collective<E> W1;
                               W1 = Collective<double>{NULL, DIMENSIONS{n, m, NULL, NULL}};
    
                    Use Case: This is useful for creating placeholder objects or delaying memory allocation.
                 */                    
                else if (like.getN()) // v is NULL
                {
                    /*this->properties->ptr = NULL;
                    this->properties->shape = like;
                    this->properties->reference_count = rc;*/
                }
                else
                {
                    /*this->properties->ptr = NULL;
                    this->properties->shape = DIMENSIONS {0, 0, NULL, NULL};
                    this->properties->reference_count = rc;*/
                }                
            }
            catch (std::bad_alloc& e)
            {
                // CRITICAL: Memory allocation failure - system should terminate immediately
                // NO cleanup performed - this is a fatal error requiring process exit
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSIONS&, cc_tokenizer::string_character_traits<char>::size_type) Error: ") + cc_tokenizer::String<char>(e.what())); 
            }
            catch (std::length_error& e)
            {
                // CRITICAL: Length constraint violation - system should terminate immediately
                // NO cleanup performed - this is a fatal error requiring process exit
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSIONS&, cc_tokenizer::string_character_traits<char>::size_type) Error: ") + cc_tokenizer::String<char>(e.what())); 
            } 
            catch (ala_exception& e)
            {
                // Propagate existing ala_exception with additional context
                // NO cleanup performed assuming this is also a critical error
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSIONS&, cc_tokenizer::string_character_traits<char>::size_type) -> ") + cc_tokenizer::String<char>(e.what()));
            }                  
        }
        
        /**
        /* @brief Constructs a `Collective` object, allocating memory for its elements based on the
        *         provided `Dimensions` object. The constructor also ensures that the memory is properly
        *         initialized from a given array of elements and handles any exceptions that may arise 
        *         during allocation.
        *
        * @tparam E The type of elements to be stored in the `Collective` object.
        * @param v Pointer to the source array from which the elements will be copied. If `v` is `NULL`, 
        *          no allocation is performed, and `ptr` is set to `NULL`.
        * @param like A pointer to a `Dimensions` object, which provides the necessary shape and size 
        *             information for the allocation. Must not be `NULL`.
        * 
        * @throws ala_exception If an allocation error occurs (e.g., due to exceeding memory limits),
        *         this constructor catches the standard exceptions (`std::length_error`
        *         and `std::bad_alloc`) and wraps them in an `ala_exception` with a detailed 
        *         error message.
        *
        * @details 
        * - The constructor first checks that the `v` pointer is not `NULL`. 
        * - If `v` is valid, it attempts to allocate memory for `ptr` based on the size specified in 
        *   the `like` object. Elements are copied from `v` to the newly allocated array.
        * - If allocation fails due to a length or memory error, an `ala_exception` is thrown.
        * - If `v` is `NULL`, the `ptr` pointer is set to `NULL` to indicate no allocation.
        * - The `shape` member is initialized by copying the dimensions from `like`.
        * - The reference count is set to 0.
        */
        Collective (E *v, DIMENSIONS_PTR like) throw (ala_exception)
        {              
            try
            { 
                this->properties = reinterpret_cast<CollectiveProperties<E>*>(cc_tokenizer::allocator<char>().allocate(sizeof(CollectiveProperties<E>)));

                if (v != NULL)
                {                        
                    if (like->getN())
                    {
                        /*this->properties->ptr = v;
                        this->properties->shape = *like;
                        this->properties->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT;*/
                    }
                    else 
                    {   
                        // NO cleanup performed assuming this is also a critical error                                               
                        throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSIONS_PTR) Error: Cannot initialize with data when dimensions specify zero elements"));
                    }                                         
                }             
                /*
                    Special Case: When 'v' is NULL but 'like' has a valid shape.
                    This allows creating a Collective object with a defined shape but no allocated data.
    
                    Example 1: Direct Initialization
                               Collective<double> W1 = Collective<double>{NULL, DIMENSIONS{n, m, NULL, NULL}};
    
                    Example 2: Assignment After Declaration
                               Collective<E> W1;
                               W1 = Collective<double>{NULL, DIMENSIONS{n, m, NULL, NULL}};
    
                    Use Case: This is useful for creating placeholder objects or delaying memory allocation.
                 */                    
                else if (like->getN()) // v is NULL
                {
                    /*this->properties->ptr = NULL;
                    this->properties->shape = *like;
                    this->properties->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT;*/
                }
                else
                {
                    /*this->properties->ptr = NULL;
                    this->properties->shape = DIMENSIONS {0, 0, NULL, NULL};
                    this->properties->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT;*/
                }
            }
            catch (std::bad_alloc& e)
            {
                // CRITICAL: Memory allocation failure - system should terminate immediately
                // NO cleanup performed - this is a fatal error requiring process exit
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSIONS_PTR) Error: ") + cc_tokenizer::String<char>(e.what())); 
            }
            catch (std::length_error& e)
            {
                // CRITICAL: Length constraint violation - system should terminate immediately
                // NO cleanup performed - this is a fatal error requiring process exit
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSIONS_PTR) Error: ") + cc_tokenizer::String<char>(e.what())); 
            } 
            catch (ala_exception& e)
            {
                // Propagate existing ala_exception with additional context
                // NO cleanup performed assuming this is also a critical error
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(E*, DIMENSTIONS_PTR) -> ") + cc_tokenizer::String<char>(e.what()));
            }
        }

        Collective (const Collective<E>& other) throw (ala_exception)
        {   
            if (other.properties != NULL) 
            {
                try 
                {
                    //ptr = cc_tokenizer::allocator<E>().allocate(other.getShape().getN());

                    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < other.getShape().getN(); i++)
                    {
                        (*this)[i] = other[i];
                    }

                    /*shape = other.getShape().copy();

                    this->properties->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT;*/
                }
                catch (std::length_error& e)
                {
                    // CRITICAL: Length constraint violation - system should terminate immediately
                    // NO cleanup performed - this is a fatal error requiring process exit
                    throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(&) Error: ") + cc_tokenizer::String<char>(e.what())); 
                }
                catch (std::bad_alloc& e)
                {   
                    // CRITICAL: Memory allocation failure - system should terminate immediately
                    // NO cleanup performed - this is a fatal error requiring process exit             
                    throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(&) Error: ") + cc_tokenizer::String<char>(e.what()));
                }
                catch (ala_exception& e)
                {
                    // Propagate existing ala_exception with additional context
                    // NO cleanup performed assuming this is also a critical error
                    throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(&) -> ") + cc_tokenizer::String<char>(e.what())); 
                }   
            }
            else
            {
                // NO cleanup performed assuming this is also a critical error
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective(Collective<E>&) Error: Cannot initialize with data when either the source is NULL or dimensions specify zero elements"));               
            }                                        
        } 

        DIMENSIONS getShape(void) const throw (ala_exception)
        {
            if (this->properties != NULL && this->properties->shape != NULL)
            {
                return *(this->properties->shape);
            }
            
            return DIMENSIONS{0, 0, NULL, NULL};
        }

        cc_tokenizer::string_character_traits<char>::size_type getReferenceCount(void) const
        {            
            if (this->properties != NULL)
            {
                return this->properties->reference_count;
            }

            return 0;
        }
        
        /*
            As the object is assigned to variables or passed around, the reference count is incremented (meaning there's one more way to access it).
         */
        void incrementReferenceCount(void) 
        {            
            if (this->properties != NULL)
            {
                if (this->properties->shape != NULL)
                {
                    this->properties->shape->incrementReferenceCount();
                }
                this->properties->reference_count++;
            }
        }
        
        void decrementReferenceCount(void)
        { 
            if (this->properties != NULL)
            {
                this->properties->shape->decrementReferenceCount();

                if (this->properties->reference_count > 0)
                {
                    this->properties->reference_count--;
                }
            }

            if (this->properties != NULL && this->properties->reference_count == 0)
            {
                if (this->properties->ptr != NULL)
                {
                    cc_tokenizer::allocator<E>().deallocate(this->properties->ptr, this->properties->shape->getN());

                    this->properties->ptr = NULL;                    
                }
                
                cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(this->properties), sizeof(CollectiveProperties<E>));
            }            
        }
        
        /**
         * @brief Destructor for the Collective class.
         *
         * This destructor is responsible for cleaning up the resources held by an instance of the Collective class.
         * It first checks if the reference_count is non-zero, and if so, it returns without performing any cleanup,
         * indicating that there are still references to the object. If reference_count is zero, it proceeds to
         * deallocate the memory pointed to by 'ptr' using the cc_tokenizer::allocator<char> alloc_obj. After memory
         * deallocation, it resets the state of the object to a valid but empty state, setting 'ptr' to NULL and
         * initializing the internal data structure to default values.
         */  
        ~Collective()
        {   
            this->decrementReferenceCount();
        }

        /*Collective<E>& operator= (E* p)
        {
            this->ptr = p;

            return *this;
        }*/

        /**
         * @brief Copy Assignment Operator
         * 
         * This copy assignment operator allows for the assignment of one Collective instance to another.
         * It ensures proper handling of reference counting, preventing resource leaks.
         *
         * @param other The Collective instance to be assigned from.
         * @return A reference to the current instance after assignment.
         */
        Collective<E>& operator= (const Collective<E>& other) throw (ala_exception)
        {               
            // Check for self-assignment                       
            if (this == &other)
            {
                return *this;
            }

            if (other.properties != NULL)
            {

                this->decrementReferenceCount();

                this->properties = other.properties;
                
                this->incrementReferenceCount();

            }
            
            return *this;
        }
        
        Collective& operator-= (Collective& other) throw (ala_exception)
        {
            if (!(getShape() == other.getShape()))
            {
                throw ala_exception("Collective::operator-=() Error: Matrix subtraction is only defined when the matrices have the same dimensions.");
            }

            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < getShape().getN(); i++)
            {
                (*this)[i] = (*this)[i] - other[i];
            }

            return *this;    
        }

        /**
            Overload the square bracket operator for assignment.
            This operator allows you to access and assign values to elements of the internal array (ptr) using the square bracket notation, just like you would with a regular array. 
        
          * @brief Overload of the square bracket operator for element access and assignment.
          *
          * This operator allows access to and assignment of values to elements of the internal array.
          *
          * @param index The index of the element to access or assign.
          * @return A reference to the element at the specified index.
          * @throws ala_exception if the index is out of range.
         */         
        E& operator[] (cc_tokenizer::string_character_traits<char>::size_type index) throw (ala_exception)
        {            
            if (this->properties == NULL)
            {
                throw ala_exception("Collective::operator[] Error: Attempting to access properties which are not set yet");
            }

            if (this->properties->ptr == NULL)
            {
                throw ala_exception("Collective::operator[] Error: Attempting to access an element in an empty array");
            }

            if (index >= this->getShape().getN())
            {
                throw ala_exception("Collective::operator[] Error: Index out of range.");
            }
            
            return this->properties->ptr[index];
        }

        /*
            Collective<E> operator= (Collective<E>& other)
            {
                ptr = reinterpret_cast<E*>(cc_tokenizer::allocator<char>().allocate(other.shape.getN()*sizeof(E)));    
                return *this;
            }
         */

        /*
            Overloaded [] operator
            @index, displacement into an array. Will retun value at that displacement.
                    Use shape.getN() to get the total number of displacement into array(total number of element in arrays)
         */
        /*E operator[] (cc_tokenizer::string_character_traits<char>::size_type index) throw (ala_exception)
        {
            if (index >= shape.getN())
            {
                throw ala_exception("Collective::operator[] Error: Index out of range.");
            }

            return ptr[index];
        }*/

        template <typename F = double>
        Collective<F> operator* (F n) throw (ala_exception)
        {                        
            F* ptr = NULL;
            Collective<F> other;

            try 
            {
                ptr = cc_tokenizer::allocator<F>().allocate(1);

                *ptr = n;

                other = Collective<F>{ptr, DIMENSIONS{1, 1, NULL, NULL}};
                
                return Numcy::dot(*this, other);
            }
            catch (const std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + cc_tokenizer::String<char>(e.what()));    
            }
            catch (const std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + cc_tokenizer::String<char>(e.what()));
            }
            catch (ala_exception& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + cc_tokenizer::String<char>(e.what()));
            }  

            /*F* ptr = NULL;
            
            try
            {
                ptr = cc_tokenizer::allocator<F>().allocate(getShape().getN());

                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < shape.getN(); i++)
                {
                    ptr[i] = (*this)[i] * n;
                }
            }
            catch (const std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + cc_tokenizer::String<char>(e.what()));    
            }
            catch (const std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + cc_tokenizer::String<char>(e.what()));
            }
            catch (ala_exception& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + cc_tokenizer::String<char>(e.what()));
            }   

            /*for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < shape.getN(); i++)
            {
                ptr[i] = (*this)[i] * n;
            }*/

            return Collective<F>{NULL, DIMENSIONS{0, 0, NULL, NULL}};
        }
        
        /**
         * @brief Overloads the binary * operator to multiply two Collective instances.
         *
         * This function handles element-wise multiplication and broadcasting between matrices of 
         * different shapes, ensuring compatibility before performing the operation. It supports:
         * 
         * 1. **Direct Multiplication:** If both matrices have the same shape, element-wise multiplication occurs.
         * 2. **Row Broadcasting:** If one operand is a row vector (1D array), it is expanded to match 
         *    the number of rows of the other matrix before multiplication.
         * 3. **Column Broadcasting:** If one operand is a column vector, it is expanded to match the 
         *    number of columns of the other matrix before multiplication.
         * 4. **Exception Handling:** If memory allocation fails or dimensions are incompatible, an 
         *    `ala_exception` is thrown with an appropriate error message.
         *
         * The function first attempts to reshape operands if necessary and then calls `Numcy::dot()` 
         * to perform the matrix multiplication.
         *
         * @tparam E The element type of the matrices.
         * @param other The right-hand side operand for multiplication.
         * @return A new `Collective<E>` instance representing the result.
         * @throws ala_exception If memory allocation fails or if the matrices have incompatible shapes.
         */
        Collective<E> operator* (Collective<E>& other) throw (ala_exception)
        {               
            /*
                BROADCAST ACROSS CORRECT AXIS
                In matrix broadcasting, element-wise operations happen between arrays of different shapes.
                The transformation (broadcasting) is not limited to multiplication—it can involve addition, 
                division, or other operations.
                "multiplicand" and "multiplier" might not be the best terms in a general broadcasting context.
                The transformation (broadcasting) is not limited to multiplication. It can involve addition, division, or other operations.
                So instead of "multiplicand" and "multiplier," it's better to use:
                - "Left operand" and "Right operand"
                - "Primary matrix" and "Broadcasted matrix"
                - "Input tensor" and "Scaling tensor" (in deep learning)
             */
            Collective<E> left_operand, right_operand;

            /*
                Automatically broadcast across rows axis:
                Reshape so that a row vector has same number of rows as the other oprand matrix 
             */
            if (getShape().getNumberOfColumns() == other.getShape().getNumberOfColumns())
            {
                if (getShape().getDimensionsOfArray().getNumberOfInnerArrays() != other.getShape().getDimensionsOfArray().getNumberOfInnerArrays())
                {
                    if (getShape().getDimensionsOfArray().getNumberOfInnerArrays() == 1)
                    {
                        // Reshape current operand (Left-hand operand) to match other operand's rows
                        // Reshape receiver operand (Left hand oprand or multiplicand)
                        try 
                        {
                            E* ptr = cc_tokenizer::allocator<E>().allocate(other.getShape().getDimensionsOfArray().getNumberOfInnerArrays()*getShape().getNumberOfColumns());
                            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < other.getShape().getDimensionsOfArray().getNumberOfInnerArrays(); i++)
                            {
                                for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < getShape().getNumberOfColumns(); j++)
                                {
                                    ptr[i*getShape().getNumberOfColumns() + j] = (*this)[j];    
                                }
                            }

                            //*this = Collective<E>{ptr, other.getShape().copy()};
                            left_operand = Collective<E>{ptr, other.getShape().copy()};

                            return Numcy::dot(left_operand, other);
                        }
                        catch (std::bad_alloc& e)
                        {
                            throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                        }
                        catch (std::length_error& e)
                        {
                            throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                        }
                        catch(ala_exception e)
                        {
                            throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                        }
                    }
                    else if (other.getShape().getDimensionsOfArray().getNumberOfInnerArrays() == 1)
                    {   
                        // Reshape other operand (Right-hand operand) to match current operand's rows
                        // Reshape multiplier opernad (right hand oprand)
                        try 
                        {
                            E* ptr = cc_tokenizer::allocator<E>().allocate(getShape().getDimensionsOfArray().getNumberOfInnerArrays()*other.getShape().getNumberOfColumns());
                            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < getShape().getDimensionsOfArray().getNumberOfInnerArrays(); i++)
                            {
                                for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < other.getShape().getNumberOfColumns(); j++)
                                {
                                    ptr[i*getShape().getNumberOfColumns() + j] = other[j];    
                                }
                            }

                            //other = Collective<E>{ptr, other.getShape().copy()};
                            right_operand = Collective<E>{ptr, getShape().copy()}; /* SIGMA CHANGE NEEDED HERE */

                            return Numcy::dot(*this, right_operand);
                        }
                        catch (std::bad_alloc& e)
                        {
                            throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                        }
                        catch (std::length_error& e)
                        {
                            throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                        }
                        catch(ala_exception e)
                        {
                            throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                        }
                    }
                }
            }
            /*
                Automatically broadcast across column axis:
                Reshape so that a column vector has same number of columns as the other oprand matrix    
             */
            else if (getShape().getDimensionsOfArray().getNumberOfInnerArrays() == other.getShape().getDimensionsOfArray().getNumberOfInnerArrays())
            {
                if (getShape().getNumberOfColumns() != other.getShape().getNumberOfColumns())
                {
                    if (getShape().getNumberOfColumns() == 1)
                    {
                        // Reshape current operand (Left-hand operand) to match other operand's columns
                        // Reshape receiver operand (Left hand operand or multiplicand)
                        try 
                        {
                            E* ptr = cc_tokenizer::allocator<E>().allocate(getShape().getDimensionsOfArray().getNumberOfInnerArrays()*other.getShape().getNumberOfColumns());
                            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < getShape().getDimensionsOfArray().getNumberOfInnerArrays(); i++)
                            {
                                for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < other.getShape().getNumberOfColumns(); j++)
                                {
                                    ptr[i*other.getShape().getNumberOfColumns() + j] = (*this)[0];    
                                }
                            }

                            //*this = Collective<E>{ptr, other.getShape().copy()};
                            left_operand = Collective<E>{ptr, other.getShape().copy()}; /* SIGMA CHANGE NEEDED HERE */

                            return Numcy::dot(left_operand, other);
                        }
                        catch (std::bad_alloc& e)
                        {
                            throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                        }
                        catch (std::length_error& e)
                        {
                            throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                        }
                        catch(ala_exception e)
                        {
                            throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                        }
                    }
                    else if (other.getShape().getNumberOfColumns() == 1)
                    {
                        // Reshape other operand (Right-hand operand) to match current operand's columns
                        // Reshape multiplier opernad (right hand operand)
                        try 
                        {
                            E* ptr = cc_tokenizer::allocator<E>().allocate(other.getShape().getDimensionsOfArray().getNumberOfInnerArrays()*getShape().getNumberOfColumns());
                            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < other.getShape().getDimensionsOfArray().getNumberOfInnerArrays(); i++)
                            {
                                for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < getShape().getNumberOfColumns(); j++)
                                {
                                    ptr[i*getShape().getNumberOfColumns() + j] = other[0];    
                                }
                            }

                            //other = Collective<E>{ptr, other.getShape().copy()};
                            right_operand = Collective<E>{ptr, getShape().copy()}; /* SIGMA CHANGE NEEDED HERE */

                            return Numcy::dot(*this, right_operand);
                        }
                        catch (std::bad_alloc& e)
                        {
                            throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                        }
                        catch (std::length_error& e)
                        {
                            throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                        }
                        catch(ala_exception e)
                        {
                            throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                        }
                    }
                }
            }

            try
            {                
                return Numcy::dot(*this, other);                
            }
            catch(ala_exception e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
            }                    
        }

        /**
         * PRESERVED FAILED IMPLEMENTATIONS - ALLOCATOR INSTANTIATION ISSUES
         *
         * Both implementations below exhibit the same core problem:
         * 
         * FAILURE MODE:
         * - Compiler fails to properly instantiate cc_tokenizer::allocator<F>
         * - Occurs despite:
         *   1. Verified template specializations exist for all supported types
         *   2. static_assert confirms allocator<double> is valid
         *   3. typeid shows correct type deduction (F == double)
         *
         * ROOT CAUSE ANALYSIS:
         * 1. Template instantiation depth issue in:
         *    - Nested template classes (Collective<Collective<E>>)
         *    - Complex inheritance hierarchy
         * 2. Visibility problem with allocator specializations:
         *    - May require explicit instantiation in translation unit
         *    - Potential ODR violations across compilation units
         *
         * IMPLEMENTATION #1 (Allocator-based):
         * - Demonstrates the "correct" OOP approach that should work
         * - Shows proper memory management pattern
         * - Failed due to allocator instantiation problems
         *
         * IMPLEMENTATION #2 (Element-wise):
         * - Attempted workaround avoiding allocator
         * - Failed due to similar template issues in:
         *   - Operator return type resolution
         *   - Nested template type deduction         
         **/       
        /*template<typename F = double>
        Collective<E> operator / (F x) throw (ala_exception)
        {  
            // First ensure the type is supported by your allocator
            static_assert(
                std::is_same<F, char>::value ||
                std::is_same<F, float>::value ||
                std::is_same<F, double>::value ||
                std::is_same<F, unsigned char>::value ||
                std::is_same<F, int>::value ||
                std::is_same<F, wchar_t>::value ||
                std::is_same<F, size_t>::value,
                "Type not supported by cc_tokenizer::allocator"
            );

            F* ptr = NULL;
            Collective<F> divisor;

            cc_tokenizer::allocator<F> alloc_obj;

            try
            {
                ptr = alloc_obj.allocate(1);
                ptr[0] = x;

                divisor = Collective<F>{ptr, DIMENSIONS{1, 1, NULL, NULL}};
            }
            catch (const std::bad_alloc& e)
            {                
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator/() Error: ") + e.what());
            }
            catch (const std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator/() Error: ") + e.what());
            }
            catch(ala_exception e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator/() -> ") + e.what());
            }

            return ((*this) / divisor);
        }*/
        /*template <typename F = double>
        Collective<E> operator/ (F x) throw (ala_exception)
        {
            Collective<E> ret;

           for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < this->getShape().getN(); i++)
           {
                (*this)[i] = (F)((*this)[i]) / x;
           }

           ret = (*this);

            return ret;
        }*/

    template <typename F = double>
    Collective<E> operator / (Collective<F>& divisor) throw (ala_exception)
    {
        E* ptr = NULL;
        
        if (!(*this).getShape().getN())
        {
            throw ala_exception("Collective::operator / () Error: Malformed shape of the array received as dividend.");
        }

        try
        {
            ptr = cc_tokenizer::allocator<E>().allocate((*this).getShape().getN());
        }
        catch (std::bad_alloc& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::operator / () Error: ") + e.what());    
        }
        catch (std::length_error& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::operator / () Error: ") + e.what());
        }
        
        if (divisor.getShape().getN() == 1)
        {
            if (divisor[0] == 0)
            {
                cc_tokenizer::allocator<E>().deallocate(ptr, (*this).getShape().getN());
                ptr = NULL;

                throw ala_exception("Collective::operator / () Error: divide by zero is not allowed.");
            }

            try
            {
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < (*this).getShape().getN(); i++)
                {
                    ptr[i] = (*this)[i] / ((E)divisor[0]);            
                }
            }
            catch (ala_exception& e)
            {
                cc_tokenizer::allocator<E>().deallocate(ptr, (*this).getShape().getN());
                ptr = NULL;

                throw ala_exception(cc_tokenizer::String<char>("Collective::operator / () -> ") + e.what());
            }            
        }
        else if ((*this).getShape() == divisor.getShape())
        {
            try 
            {
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < divisor.getShape().getN(); i++)
                {
                    ptr[i] = (*this)[i] / (E)(divisor[i]);
                }
            }
            catch (ala_exception& e)
            {
                cc_tokenizer::allocator<E>().deallocate(ptr, (*this).getShape().getN());
                ptr = NULL;

                throw ala_exception(cc_tokenizer::String<char>("Collective::operator / () -> ") + e.what()); 
            }   
        }
        else 
        {
            throw ala_exception("Collective::operator / () Error: Cannot divide matrices with incompatible shapes. Ensure both matrices have the same dimensions before performing the operation.");
        }

        return Collective<E>{ptr, *((*this).getShape().copy())}; /* SIGMA CHANGE NEEDED HERE */
    }

    /**
     * @brief Overloads the subtraction operator for the Collective class.
     *
     * This method performs element-wise subtraction of a given subtrahend 
     * from each element of the current Collective instance.
     *
     * @tparam E The type of elements stored in the Collective instance.
     * 
     * @throws ala_exception Thrown if memory allocation fails (std::bad_alloc)
     *         or if the operation causes a length error (std::length_error).
     *
     * @return Collective<E> A new Collective instance containing the results of 
     *         the subtraction, with the same shape as the original.
     *
     * @details 
     * - Allocates memory dynamically for storing the results of the subtraction.
     * - Handles exceptions arising during memory allocation or other potential errors.
     * - Iterates over the elements of the Collective, subtracting the specified 
     *   subtrahend from each element and storing the results in the allocated memory
     *
     * Example Usage:
     * @code
     * Collective<int> a = ...; // Initialize with some values
     * int subtrahend = 5;
     * Collective<int> result = a - subtrahend;
     * @endcode
     */
    template <typename F = double>
    Collective<E> operator- (F subtrahend) throw (ala_exception)
    {
        // this-> is your receiver object
        if (!(*this).getShape().getN())
        {
            throw ala_exception("Collective::operator - () Error: Malformed shape of the array received as minuend.");
        }

        E* ptr = NULL;

        try
        {
            ptr = cc_tokenizer::allocator<E>().allocate((*this).getShape().getN());
        }
        catch (std::bad_alloc& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::operator - () Error: ") + e.what());    
        }
        catch (std::length_error& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::operator - () Error: ") + e.what());
        }

        for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < (*this).getShape().getN(); i++)
        {
            ptr[i] = (*this)[i] - (E)subtrahend;
        }

        return Collective<E>{ptr, /**((*this).getShape().copy())*/ getShape().copy()}; /* SIGMA CHANGE NEEDED HERE */
    }

    /**
     * @brief Overloaded subtraction operator for the `Collective` class.
     *
     * This function performs element-wise subtraction between the current `Collective` object (minuend)
     * and another `Collective` object (subtrahend). It supports two cases:
     * 1. Scalar subtraction: If the subtrahend is a scalar (shape = 1), it subtracts the scalar value
     *    from every element of the minuend.
     * 2. Matrix subtraction: If the subtrahend has the same shape as the minuend, it performs
     *    element-wise subtraction between the two matrices.
     *
     * @tparam F The data type of the subtrahend (default: double).
     * @param subtrahend The `Collective` object to subtract from the current object.
     *
     * @return A new `Collective` object containing the result of the subtraction.
     *
     * @throws ala_exception Throws an exception under the following conditions:
     *   - If the minuend (current object) has a malformed shape (empty or invalid).
     *   - If memory allocation fails during the operation.
     *   - If the subtrahend is not a scalar and its shape does not match the minuend's shape.
     *
     * @note The function ensures proper memory management by deallocating temporary memory
     *       in case of exceptions or errors.
     *
     * Example usage:
     * @code
     * Collective<double> A = {1.0, 2.0, 3.0};
     * Collective<double> B = {0.5, 1.5, 2.5};
     * Collective<double> result = A - B; // Element-wise subtraction
     * @endcode
     *
     * @code
     * Collective<double> A = {1.0, 2.0, 3.0};
     * Collective<double> scalar = {0.5};
     * Collective<double> result = A - scalar; // Scalar subtraction
     * @endcode
    */
    template <typename F = double>
    Collective<E> operator- (Collective<F>& subtrahend) throw (ala_exception)
    {   
        E* ptr = NULL;

        if (!(*this).getShape().getN())
        {
            throw ala_exception("Collective::operator - () Error: Malformed shape of the array received as minuend.");
        }

        try
        {
            ptr = cc_tokenizer::allocator<E>().allocate(getShape().getN());
        }
        catch (std::bad_alloc& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::operator-() Error: ") + e.what());    
        }
        catch (std::length_error& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::operator-() Error: ") + e.what());
        }

        if (subtrahend.getShape().getN() == 1)
        {
            try
            {                                        
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < getShape().getN(); i++)
                {
                    ptr[i] = (*this)[i] - (E)(subtrahend[0]);
                }
            }
            catch (ala_exception& e)
            {
                cc_tokenizer::allocator<E>().deallocate(ptr, /*(*this).getShape().getN()*/ getShape().getN());
                ptr = NULL;

                throw ala_exception(cc_tokenizer::String<char>("Collective::operator-() -> ") + e.what());
            }
        }
        else if ((*this).getShape() == subtrahend.getShape())
        {
            try
            {
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < subtrahend.getShape().getN(); i++)
                {
                    ptr[i] = (*this)[i] - (E)(subtrahend[i]);
                }
            }
            catch (ala_exception& e)
            {
                cc_tokenizer::allocator<E>().deallocate(ptr, getShape().getN());
                ptr = NULL;

                throw ala_exception(cc_tokenizer::String<char>("Collective::operator-() -> ") + e.what());
            }
        }
        else
        {
            cc_tokenizer::allocator<E>().deallocate(ptr, /*(*this).getShape().getN()*/ getShape().getN());
            ptr = NULL;

            throw ala_exception("Collective::operator-() Error: Cannot subtract matrices with incompatible shapes. Ensure both matrices have the same dimensions before performing the operation.");
        }

        return Collective<E>{ptr, getShape()};
    }

    bool operator== (Collective<E>& other)
    {
        bool ret = false;

        if ((ptr == other.ptr) && (getShape() == other.getShape()))
        {
            ret = true;
        }

        return ret;
    }

    // Overloading the binary * operator for multiplying a Collective by an array of type E; 
    
    /*
    template <typename F = float>
    void operator* (const F* vector) throw (ala_exception)
    {
        if (vector == NULL)
        {
            throw ala_exception("Collective::operator*(): pointer to multiplier array is null.");
        }
        
        std::cout<<"getN = "<<this->shape.getN()<<std::endl;         
    }
     */
        
    template <typename F = double>
    Collective<F> operator+ (F a) throw (ala_exception)
    {   
        /*static_assert(!std::is_same<F, Collective<typename F::value_type>>::value, 
            "E must not be a Collective type.");*/

        F* ptr = NULL;
        Collective<E> ret;

        try
        {
            // Allocate memory for the scalar value
            ptr = cc_tokenizer::allocator<F>().allocate(1);
            ptr[0] = a;
            /**
             *  Nested Type Issue
             *  --------------------
             *  -----------------------
             *  The fact that explicitly creating a temp object works while using a temporary object directly
             *  fails strongly supports Following arguments.... 
             */    
            /** 
             *  In the statement ret = ((*this) + Collective<E>{ptr, DIMENSIONS{1, 1, NULL, NULL}});, 
             *  the expression Collective<E>{ptr, DIMENSIONS{1, 1, NULL, NULL}} creates an instance of type 
             *  Collective<E>.
             *  The template argument deduction allows the compiler to automatically determine the template 
             *  arguments based on the types of the arguments passed to a templated function.
             *  if E is double then, in the statement ret = ((*this) + Collective<E>{ptr, DIMENSIONS{1, 1, NULL, NULL}}); 
             *  the expression "(*this) +" is deducing the type of argumnet at right as Collective<Collective<double>> 
             *  instead of a Collective<double>. This nested type is then passed to the assignment operator,
             *  which expects a Collective<double>, leading to the type mismatch error.
             */                        
            // Create a temporary Collective<E> object to hold the scalar value            
            Collective<E> temp = Collective<E>{ptr, DIMENSIONS{1, 1, NULL, NULL}};
            // Perform element-wise addition            
            ret = (*this) + temp;
            /** 
             *  THIS DOES NOT WORK(the failing version), WHY?
             *  ------------------------------------------------
             *  ret = ((*this) + Collective<E>{ptr, DIMENSIONS{1, 1, NULL, NULL}});
             *  This version fails strongly suggests that the issue is related to how the compiler handles 
             *  temporary objects and type deduction in the context of the operator+ function.
             *  TEMPORARY OBJECT CREATION
             *  ----------------------------
             *  In the working version, the "temp" is explicitly created as a Collective<E>. 
             *  The compiler knows the exact type of temp and uses it in the operator+ call.
             *  In the failing version Collective<E>{ptr, DIMENSIONS{1, 1, NULL, NULL}} is a 
             *  temporary object. The compiler might treat it differently during template 
             *  argument deduction, leading to unexpected behavior.
             */
             /**
              *  The failing version suggests that the compiler might be treating the temporary object as a
              *  nested type (Collective<Collective<E>>) instead of a flat type (Collective<E>).
              *  This is likely due to how the "operator+"" function is implemented(it seems that implementation has no problems)
              *  or how the compiler handles temporary objects in template deduction.
              */        
        }
        catch(const std::bad_alloc& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::operator+() Error: ") + cc_tokenizer::String<char>(e.what()));
        }
        catch (const std::length_error& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::operator+() Error: ") + cc_tokenizer::String<char>(e.what()));
        }
        catch (ala_exception& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::operator+() -> ") + cc_tokenizer::String<char>(e.what()));
        }
         
        return ret;        
    }    

    /**
     * @brief Overloaded addition operator for combining two Collective objects.
     * 
     * This method performs an element-wise addition of two Collective instances. 
     * If the shape of the `other` Collective has a single element, the value of that 
     * element is broadcast and added to all elements of the current Collective. 
     * Otherwise, the two Collectives must have identical shapes for the addition 
     * to be performed successfully.
     * 
     * @tparam E The type of elements stored in the Collective.
     * @param other A reference to another Collective<E> object to be added.
     * @return Collective<E> A new Collective object representing the result of the addition.
     * 
     * @throws ala_exception If:
     *         - The shape of `other` is invalid (zero elements).
     *         - The shapes of the two Collectives are incompatible.
     *         - Memory allocation fails during the creation of the result.
     *         - Any length or size errors occur.
     * 
     * @note The left and right operands can be the same instance. 
     *       All necessary checks are performed to ensure safe and meaningful operations.
     */
    Collective<E> operator+ (Collective<E>& other) throw (ala_exception)
    {
        /*static_assert(!std::is_same<E, Collective<typename E::value_type>>::value, 
            "E must not be a Collective type.");*/

        /*
            No need, left and right oprands can be same instances
         */
        /*if (*this == other)
        {
            return *this;
        }*/
        
        if (!(other.getShape().getN() > 0))
        {
            throw ala_exception("Collective::operator+() Error: The 'other' Collective has an invalid shape with zero elements.");
        }

        /*
            BROADCAST ACROSS CORRECT AXIS
            In matrix broadcasting, element-wise operations happen between arrays of different shapes.
            The transformation (broadcasting) is not limited to multiplication—it can involve addition, 
            division, or other operations.            
            It's better to use:
            - "Left operand" and "Right operand"
            - "Primary matrix" and "Broadcasted matrix"
            - "Input tensor" and "Scaling tensor" (in deep learning)
         */
         Collective<E> left_operand, right_operand;

         /*
            Automatically broadcast across rows axis:
            Reshape so that a row vector has same number of rows as the other oprand matrix 
          */
         if (getShape().getNumberOfColumns() == other.getShape().getNumberOfColumns())
         {
            if (getShape().getDimensionsOfArray().getNumberOfInnerArrays() != other.getShape().getDimensionsOfArray().getNumberOfInnerArrays())
            {
                if (getShape().getDimensionsOfArray().getNumberOfInnerArrays() == 1)
                {
                    // Reshape current operand (Left-hand operand) to match other operand's rows
                    // Reshape receiver operand (Left hand oprand or multiplicand)
                    try 
                    {
                        E* ptr = cc_tokenizer::allocator<E>().allocate(other.getShape().getDimensionsOfArray().getNumberOfInnerArrays()*getShape().getNumberOfColumns());
                        for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < other.getShape().getDimensionsOfArray().getNumberOfInnerArrays(); i++)
                        {
                            for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < getShape().getNumberOfColumns(); j++)
                            {
                                //ptr[i*getShape().getNumberOfColumns() + j] = (*this)[j];
                                
                                ptr[i*getShape().getNumberOfColumns() + j] = (*this)[j] + other[i*getShape().getNumberOfColumns() + j]; 
                            }
                        }

                        //*this = Collective<E>{ptr, other.getShape().copy()};
                        left_operand = Collective<E>{ptr, other.getShape().copy()}; /* SIGMA CHANGE NEEDED HERE */

                        //return Numcy::dot(left_operand, other);

                        return left_operand;
                    }
                    catch (std::bad_alloc& e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                    }
                    catch (std::length_error& e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                    }
                    catch(ala_exception e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                    }
                }
                else if (other.getShape().getDimensionsOfArray().getNumberOfInnerArrays() == 1)
                {   
                    // Reshape other operand (Right-hand operand) to match current operand's rows
                    // Reshape multiplier opernad (right hand oprand)
                    try 
                    {
                        E* ptr = cc_tokenizer::allocator<E>().allocate(getShape().getDimensionsOfArray().getNumberOfInnerArrays()*other.getShape().getNumberOfColumns());
                        for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < getShape().getDimensionsOfArray().getNumberOfInnerArrays(); i++)
                        {
                            for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < other.getShape().getNumberOfColumns(); j++)
                            {
                                //ptr[i*getShape().getNumberOfColumns() + j] = other[j]; 

                                ptr[i*getShape().getNumberOfColumns() + j] = (*this)[i*getShape().getNumberOfColumns() + j] + other[j];
                            }
                        }

                        //other = Collective<E>{ptr, other.getShape().copy()};
                        right_operand = Collective<E>{ptr, getShape().copy()}; /* SIGMA CHANGE NEEDED HERE */

                        //return Numcy::dot(*this, right_operand);

                        return right_operand;
                    }
                    catch (std::bad_alloc& e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                    }
                    catch (std::length_error& e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                    }
                    catch(ala_exception e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                    }
                }
            }
         }
         /*
            Automatically broadcast across column axis:
            Reshape so that a column vector has same number of columns as the other oprand matrix    
          */
         else if (getShape().getDimensionsOfArray().getNumberOfInnerArrays() == other.getShape().getDimensionsOfArray().getNumberOfInnerArrays())
         {
             if (getShape().getNumberOfColumns() != other.getShape().getNumberOfColumns())
             {
                 if (getShape().getNumberOfColumns() == 1)
                 {
                     // Reshape current operand (Left-hand operand) to match other operand's columns
                     // Reshape receiver operand (Left hand operand or multiplicand)
                     try 
                     {
                         E* ptr = cc_tokenizer::allocator<E>().allocate(getShape().getDimensionsOfArray().getNumberOfInnerArrays()*other.getShape().getNumberOfColumns());
                         for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < getShape().getDimensionsOfArray().getNumberOfInnerArrays(); i++)
                         {
                             for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < other.getShape().getNumberOfColumns(); j++)
                             {
                                 ptr[i*other.getShape().getNumberOfColumns() + j] = (*this)[0] + other[i*other.getShape().getNumberOfColumns() + j];
                             }
                         }

                         /*this = Collective<E>{ptr, other.getShape().copy()};*/
                         left_operand = Collective<E>{ptr, other.getShape().copy()}; /* SIGMA CHANGE NEEDED HERE */
 
                         //return Numcy::dot(left_operand, other);

                         return left_operand;
                     }
                     catch (std::bad_alloc& e)
                     {
                         throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                     }
                     catch (std::length_error& e)
                     {
                         throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                     }
                     catch(ala_exception e)
                     {
                         throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                     }
                 }
                 else if (other.getShape().getNumberOfColumns() == 1)
                 {
                     // Reshape other operand (Right-hand operand) to match current operand's columns
                     // Reshape multiplier opernad (right hand operand)
                     try 
                     {
                         E* ptr = cc_tokenizer::allocator<E>().allocate(other.getShape().getDimensionsOfArray().getNumberOfInnerArrays()*getShape().getNumberOfColumns());
                         for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < other.getShape().getDimensionsOfArray().getNumberOfInnerArrays(); i++)
                         {
                             for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < getShape().getNumberOfColumns(); j++)
                             {
                                 ptr[i*getShape().getNumberOfColumns() + j] = (*this)[i*getShape().getNumberOfColumns() + j] + other[0];
                             }
                         }

                         //other = Collective<E>{ptr, other.getShape().copy()};
                         right_operand = Collective<E>{ptr, getShape().copy()}; /* SIGMA CHANGE NEEDED HERE */

                         //return Numcy::dot(*this, right_operand);

                         return right_operand;
                     }
                     catch (std::bad_alloc& e)
                     {
                         throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                     }
                     catch (std::length_error& e)
                     {
                         throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                     }
                     catch(ala_exception e)
                     {
                         throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
                     }
                 }
             }
         }

        E* local_ptr = NULL;
        Collective<E> ret;
        
        if (other.getShape().getN() == 1)
        {
            // Scalar addition
            try
            {
                local_ptr = cc_tokenizer::allocator<E>().allocate(getShape().getN());
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < getShape().getN(); i++)
                {
                    local_ptr[i] = (*this)[i] + other[0];
                }
                ret = Collective<E>{local_ptr, getShape().copy()}; /* SIGMA CHANGE NEEDED HERE */
            }
            catch (const std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator+() Error: ") + e.what());
            }
            catch (const std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator+() Error: ") + e.what());
            }
            catch (ala_exception& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator+() -> ") + e.what());
            }
        }
        else if (getShape() == other.getShape())
        {
            // Element-wise addition
            try
            {
                local_ptr = cc_tokenizer::allocator<E>().allocate(other.getShape().getN());
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < other.getShape().getN(); i++)
                {
                    local_ptr[i] = (*this)[i] + other[i];
                }
                ret = Collective<E>{local_ptr, other.getShape().copy()}; /* SIGMA CHANGE NEEDED HERE */
            }
            catch (const std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator+() Error: ") + e.what());
            }
            catch (const std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator+() Error: ") + e.what());                
            }
            catch (ala_exception& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator+() -> ") + e.what());
            }
        }
        else
        {
            throw ala_exception("Collective::operator+() Error: The shapes of the two collectives are incompatible for addition.");
        }

        return ret;
    }
     
    Collective<E> operator_plus (Collective<E>& other) 
    {
        cc_tokenizer::allocator<char> alloc_obj;

        DIMENSIONSOFARRAY a = shape.getDimensionsOfArray();        
        DIMENSIONSOFARRAY b = other.shape.getDimensionsOfArray();
        DIMENSIONS_PTR dim_head = NULL, current = NULL;

        Collective<E> ret = {NULL, {0, 0, NULL, NULL}};

        if (a.compare(b))
        {             
            //std::cout<<"Both are same"<<", a = "<<a.ptr[(this->shape.getNumberOfLinks())]<<", b = "<<b.ptr[(other.shape.getNumberOfLinks())]<<std::endl;
            /* For example... a[2][3][4][5] and b[2][3][4][2] */
            cc_tokenizer::string_character_traits<char>::size_type n = 1;
            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < this->shape.getNumberOfLinks(); i++)
            {
                //n = n * a.ptr[i];

                n = n * a[i];
            }

            //std::cout<<"Rows(n) = "<<n<<std::endl;
            //std::cout<<"Total number of columns = "<<a.ptr[this->shape.getNumberOfLinks()] + b.ptr[other.shape.getNumberOfLinks()]<<std::endl;

            //DIMENSIONS_PTR dim_head = NULL, current = NULL;

            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < shape.getNumberOfLinks(); i++)
            {
                if (dim_head == NULL)
                {
                    dim_head = reinterpret_cast<DIMENSIONS_PTR>(alloc_obj.allocate(sizeof(DIMENSIONS)));
                    current = dim_head;

                    *current = DIMENSIONS{0, 0, NULL, NULL};    

                    //current->next = NULL;
                    //current->prev = NULL;                    
                }
                else
                {
                    current->next = reinterpret_cast<DIMENSIONS_PTR>(alloc_obj.allocate(sizeof(DIMENSIONS)));
                    current->next->next= NULL;
                    current->next->prev = current;
                    current = current->next;
                }

                current->columns = 0;                
                //current->rows = a.ptr[i];
                current->rows = a[i];
            }

            //current->columns = a.ptr[shape.getNumberOfLinks()] + b.ptr[shape.getNumberOfLinks()];

            current->columns = a[shape.getNumberOfLinks()] + b[shape.getNumberOfLinks()];

            //ret.shape = *dim_head;
            //ret.ptr = reinterpret_cast<E*>(alloc_obj.allocate(sizeof(E)*n*(a.ptr[this->shape.getNumberOfLinks()] + b.ptr[other.shape.getNumberOfLinks()])));

            ret = Collective<E>{reinterpret_cast<E*>(alloc_obj.allocate(sizeof(E)*n*(a[/*this->*/ shape.getNumberOfLinks()] + b[other.shape.getNumberOfLinks()]))), *dim_head};
        
            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < n; i++)
            {
                for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < a[/*this->*/ shape.getNumberOfLinks()]; j++)
                {
                    //ret.ptr[i*(a.ptr[this->shape.getNumberOfLinks()] + b.ptr[other.shape.getNumberOfLinks()]) + j] = this->ptr[j];

                    ret.ptr[i*(a[/*this->*/ shape.getNumberOfLinks()] + b[other.shape.getNumberOfLinks()]) + j] = this->ptr[j];
                }

                for (cc_tokenizer::string_character_traits<char>::size_type j = a[this->shape.getNumberOfLinks()]; j < a[this->shape.getNumberOfLinks()] + b[other.shape.getNumberOfLinks()]; j++)
                {
                    //ret.ptr[i*(a.ptr[this->shape.getNumberOfLinks()] + b.ptr[other.shape.getNumberOfLinks()]) + j] = other.ptr[j - a.ptr[this->shape.getNumberOfLinks()]];

                    ret.ptr[i*(a[/*this->*/ shape.getNumberOfLinks()] + b[other.shape.getNumberOfLinks()]) + j] = other.ptr[j - a[/*this->*/ shape.getNumberOfLinks()]];
                }
            }
        }
                                        
        ////cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<cc_tokenizer::string_character_traits<char>::pointer>(a.ptr));
        ////cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<cc_tokenizer::string_character_traits<char>::pointer>(b.ptr));

        //alloc_obj.deallocate(reinterpret_cast<cc_tokenizer::string_character_traits<char>::pointer>(a.ptr));
        a.decrementReferenceCount();
        //alloc_obj.deallocate(reinterpret_cast<cc_tokenizer::string_character_traits<char>::pointer>(b.ptr));
        b.decrementReferenceCount();
        alloc_obj.deallocate(reinterpret_cast<cc_tokenizer::string_character_traits<char>::pointer>(dim_head));

        return ret;
    }

    /**
     * @brief Extracts a slice from the Collective object based on the specified axis and dimensions.
     * 
     * This function provides slicing capabilities for a Collective object along different axes.
     * It can handle two main cases:
     * - `AXIS_NONE`: Extracts a contiguous slice from the Collective object.
     * - `AXIS_COLUMN`: Extracts a slice along the column axis (currently limited to specific cases).
     * 
     * @tparam E The type of the elements in the Collective.
     * 
     * @param i The starting index for the slice.
     * @param dim The dimensions of the slice to be extracted. Must have a non-zero length.
     * @param axis The axis along which to extract the slice. Defaults to `AXIS_NONE`.
     * 
     * @return A new `Collective<E>` object containing the extracted slice.
     * 
     * @throws ala_exception If:
     * - `dim.getN()` is zero.
     * - The slice range exceeds the bounds of the available data for the specified axis.
     * - Memory allocation fails while extracting the slice.
     * - Other length errors occur during operation.
     * 
     * @note Memory is dynamically allocated for the slice during extraction and 
     *       deallocated immediately after the slice is copied into the return object.
     * 
     * @details 
     * - For `AXIS_NONE`: A contiguous range of elements is extracted from index `i`.
     * - For `AXIS_COLUMN`: Elements are extracted from the column axis, with 
     *   consideration of the structure of the Collective's internal storage.
     * 
     * Example Usage:
     * ```cpp
     * DIMENSIONS dim = ...; // Initialize dimensions
     * Collective<int> coll = ...; // Initialize a Collective object
     * try {
     *     Collective<int> slice = coll.slice(0, dim, AXIS_NONE);
     *     // Use the sliced Collective
     * } catch (const ala_exception& e) {
     *     std::cerr << e.what() << std::endl;
     * }
     * ```
     */
    Collective<E> slice(cc_tokenizer::string_character_traits<char>::size_type i, DIMENSIONS& dim, AXIS axis = AXIS_NONE) throw(ala_exception)
    {
        E* slice_ptr;
        Collective<E> ret;

        if (!dim.getN())
        {
            throw ala_exception("Collective::slice() Error: The slice length must be greater than zero.");
        }

        switch (axis)
        {
            case AXIS_NONE:
            {
                if ((i + dim.getN()) > getShape().getN())
                {
                    throw ala_exception("Collective::slice() Error: The slice range exceeds the bounds of the available data for \"AXIS_NONE\".");
                }

                try
                {
                    slice_ptr = cc_tokenizer::allocator<E>().allocate(dim.getN());

                    for (cc_tokenizer::string_character_traits<E>::size_type j = 0; j < dim.getN(); j++)
                    {
                        slice_ptr[j] = ptr[i + j];
                    }


                    ret = Collective<E>{slice_ptr, *(dim.copy())};

                    cc_tokenizer::allocator<E>().deallocate(slice_ptr, dim.getN());

                    slice_ptr = NULL;
                }
                catch(const std::bad_alloc& e)
                {
                    throw ala_exception(cc_tokenizer::String<char>("Collective::slice() Error: ") + cc_tokenizer::String<char>(e.what()));
                }
                catch(const std::length_error& e)
                {
                    throw ala_exception(cc_tokenizer::String<char>("Collective::slice() Error: ") + cc_tokenizer::String<char>(e.what()));
                }
            }
            break;
            case AXIS_COLUMN:
            {                
                /*
                    At the moment, we are only considering the case where the slice is along the column axis 
                 */
                if (dim.getN() > getShape().getDimensionsOfArray().getNumberOfInnerArrays() || i > getShape().getNumberOfColumns())
                {
                    throw ala_exception("Collective::slice() Error: The slice range exceeds the bounds of the available data for \"AXIS_COLUMN\".");
                }
                                
                try
                {
                    slice_ptr = cc_tokenizer::allocator<E>().allocate(dim.getN());                    
                    for (cc_tokenizer::string_character_traits<E>::size_type j = 0; j < getShape().getDimensionsOfArray().getNumberOfInnerArrays(); j++)
                    {
                        slice_ptr[j] = ptr[i + j*getShape().getNumberOfColumns()];
                    }
                                        
                    ret = Collective<E>{slice_ptr, *(dim.copy())};
                                        
                    cc_tokenizer::allocator<E>().deallocate(slice_ptr, dim.getN());

                    slice_ptr = NULL;
                }
                catch(const std::bad_alloc& e)
                {
                    throw ala_exception(cc_tokenizer::String<char>("Collective::slice() Error: ") + cc_tokenizer::String<char>(e.what()));
                }
                catch (const std::length_error& e)
                {
                     throw ala_exception(cc_tokenizer::String<char>("Collective::slice() Error: ") + cc_tokenizer::String<char>(e.what()));  
                }              
            }
            break;
        }

        return ret;
    }

    /*
    Collective<E> slice(cc_tokenizer::string_character_traits<char>::size_type i, DIMENSIONS& dim) throw(ala_exception)
    {
        E* slice_ptr;
        Collective<E> ret;

        if (!dim.getN())
        {
            throw ala_exception("Collective::slice() Error: The slice length must be greater than zero.");
        }

        if ((i + dim.getN()) > shape.getN())
        {
            throw ala_exception("Collective::slice() Error: The slice range exceeds the bounds of the available data.");
        }

        try
        {
            slice_ptr = cc_tokenizer::allocator<E>().allocate(dim.getN());

            for (cc_tokenizer::string_character_traits<E>::size_type j = 0; j < dim.getN(); j++)
            {
                slice_ptr[j] = ptr[i + j];
            }

            ret = Collective<E>{slice_ptr, *dim.copy()};
        }
        catch(const std::bad_alloc& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::slice() Error: ") + cc_tokenizer::String<char>(e.what()));                
        }
        catch(const std::length_error& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::slice() Error: ") + cc_tokenizer::String<char>(e.what())); 
        }

        return ret;
    }
     */

    /**
      * @brief Slices a portion of data starting from a given index and for a specified length.
      * 
      * This method returns a dynamically allocated slice of elements from the data buffer starting at index 'i' 
      * and spanning 'n' elements. The function performs several safety checks, including ensuring that the 
      * slice length 'n' is greater than zero and that the slice does not exceed the bounds of the available data. 
      * If these conditions are violated, an `ala_exception` is thrown. Additionally, the method manages memory 
      * allocation for the slice, throwing an `alloc_exception` in case of memory allocation issues.
      *
      * @param i The starting index from which the slice begins.
      * @param n The number of elements to include in the slice.
      * @return E* A pointer to the allocated array containing the sliced data.
      * 
      * @throws ala_exception If the slice length 'n' is less than or equal to zero or if the slice exceeds 
      * the bounds of the available data.
      * @throws alloc_exception If memory allocation for the slice fails.
      *
      * @todo Extend this function to handle more complex shapes, not just simple linear slices.
    */
    /*E**/ Collective<E> slice(cc_tokenizer::string_character_traits<char>::size_type i, cc_tokenizer::string_character_traits<char>::size_type n) throw(ala_exception)
    {
        E* slice_ptr = NULL;
        Collective<E> ret;

        if (!(n > 0))
        {            
            throw ala_exception("Collective::slice() Error: The slice length 'n' must be greater than zero.");
        }

        if ((i + n) > /*shape.getN()*/ getShape().getN())
        {
            throw ala_exception("Collective::slice() Error: The slice range exceeds the bounds of the available data.");
        }

        try
        {
            slice_ptr = cc_tokenizer::allocator<E>().allocate(n);

            for (cc_tokenizer::string_character_traits<E>::size_type j = 0; j < n; j++)
            {
                slice_ptr[j] = ptr[i + j];
            }

            ret = Collective<E>{slice_ptr, DIMENSIONS{n, 1, NULL, NULL}};

            cc_tokenizer::allocator<E>().deallocate(slice_ptr, n);
        }
        catch(const std::bad_alloc& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::slice() Error: ") + cc_tokenizer::String<char>(e.what()));                
        }
        catch(const std::length_error& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::slice() Error: ") + cc_tokenizer::String<char>(e.what())); 
        }
        catch (ala_exception& e)
        {
            throw ala_exception(cc_tokenizer::String<char>("Collective::slice() -> ") + e.what());
        }
        
        //return slice_ptr;
        return ret;
    }
};
#endif