/*
    numcy/Collective.hh
    Q@khaa.pk
 */

#include "header.hh"

#ifndef KHAA_PK_COLLECTIVE_HEADER_HH
#define KHAA_PK_COLLECTIVE_HEADER_HH

/*  
    The following composite is named "Collective", the instance of this composite
    is a collection of array of type "E" and its shape. 
 */
template<typename E = double>
struct Collective 
{
    private:
        E* ptr;
        /*
            Reference counting tracks the number of active references (pointers or other ways to access) to an object.
            When an object is first created, there are no references to it, so the count should be 0.
           
            The responsibility of managing the reference count lies with the class itself...
            the incrementRefernceCount() and decrementReferenceCount() methods and the implementation of 
            destructor method         
         */        
        //unsigned int reference_count;
        cc_tokenizer::string_character_traits<char>::size_type reference_count;

    
    public:        
        DIMENSIONS shape;

        // Default constructor
        Collective (void) : ptr(NULL), shape(Dimensions{0, 0, NULL, NULL}), reference_count(0)
        {              
        }

        Collective (E* v, DIMENSIONS like) : ptr(v), shape(like), reference_count(0)
        {
        }

        /**
         * Constructor for the Collective class.
         *
         * This constructor initializes the object's member variables:
         *  - `ptr`: Pointer to data (of type E*)
         *  - `shape`: Shape information (of type DIMENSIONS)
         *  - `reference_count`: Initial reference count (set to the provided `count`)
         *
         * **Important Note:** It's crucial to avoid explicitly calling `delete this;` within the constructor 
         * or the `decrementReferenceCount` function. This is because the object's memory will be 
         * automatically managed through the destructor when the reference count reaches zero. Explicitly 
         * deleting the object within these methods leads to "double deletion" and program crashes.
         *
         * @param v Pointer to data (of type E*)
         * @param like Shape information (of type DIMENSIONS)
         * @param count Initial reference count
         */
        Collective (E* v, DIMENSIONS like, cc_tokenizer::string_character_traits<char>::size_type count) : ptr(v), shape(like), reference_count(count)
        {
        }

        Collective (E *v, Dimensions *like) : ptr(v), shape(*like), reference_count(0)
        {            
        }

        Collective (Collective<E>& other) throw (ala_exception)
        {   
            reference_count = 0;         

            try 
            {
                ptr = cc_tokenizer::allocator<E>().allocate(other.getShape().getN());
            }
            catch (std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::Collective() Error: ") + cc_tokenizer::String<char>(e.what()));
            }

            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < other.getShape().getN(); i++)
            {
                ptr[i] = other[i];
            }

            shape = *(other.getShape().copy());
        } 

        DIMENSIONS& getShape(void)
        {
            return shape;
        }

        cc_tokenizer::string_character_traits<char>::size_type showReferenceCounter(void)
        {
            return reference_count;
        }
        
        /*
            As the object is assigned to variables or passed around, the reference count is incremented (meaning there's one more way to access it).
         */
        void incrementReferenceCount(void) 
        {
            reference_count++;

            //shape.incrementReferenceCount();
        }
        /*
            When a reference goes out of scope or is explicitly released, the reference count is decremented.
         */
        void decrementReferenceCount(void)
        {
            shape.decrementReferenceCount();

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
            else if (reference_count == 0)
            {
                /*
                    Look for better version of this comment block below...
                    ---------------------------------------------------------
                    Gemini please make the following comment better... 
                    Some how I think this is not the way to call the constructor explicitly, 
                    I am compiling this program with cl(Visual Studio Tools) and when the following
                    statement gets executed whole program just crashes.

                    Naively I provided a constructor for this class which takes the refeence_count as an one of its arguments.
                    The idea is that a the time if instanciation you set the reference count to 1, and then during the ife time of the instanciated object 
                    you call this method(the decrement reference count) in the hope that object resources get collected in safe manner,
                    but even then program crashes when I call this decrementReferenceCount() method.
                 */
                /*
                    Better version of above comments...
                    --------------------------------------
                    There seems to be an issue with explicitly calling the destructor within the 
                    `decrementReferenceCount` function. While compiling with cl (Visual Studio Tools),
                    the program crashes when the commented-out line `delete this;` is executed.

                    My initial approach was to provide a constructor that takes the `reference_count`
                    as an argument. This was intended to set the initial reference count to 1
                    during object creation. Subsequently, I planned to call `decrementReferenceCount`
                    throughout the object's lifetime to manage memory in a controlled manner.
                    However, the program crashes even with this approach.
                 */

                //delete this;
            }

            /*
                When the reference count reaches zero in decrementReferenceCount, the object itself goes out of scope.
                This triggers the automatic invocation of the destructor, which then takes care of deleting 
                the memory and performing any necessary cleanup tasks.
             */           
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
            if (reference_count)
            {
                return;
            }
            
            /*
                When the reference count reaches 0, it signifies there are no more active references, and the object's memory can be safely deallocated.
             */

            /*
                To stop the effects of double deletion... becuse we are explicitly calling the destructor method of this object
             */
            if (ptr != NULL)
            {                            
                cc_tokenizer::allocator<E>().deallocate(ptr);

                /* 
                    TODO, causing problems, find where?
                 */     
                /* 
                    Reset the state of the object to a valid but empty state 
                  -------------------------------------------------------------  
                 */
                /* 
                    Compile time error "No operator found which takes right hand operand of type initializer list"                             
                    *this = {NULL, {0, 0, NULL, NULL}, 0};
                 */
                /*
                    But the above mentioned compile time error is not thrown for this statenment
                    *this = {NULL, {0, 0, NULL, NULL}};
                 */
                /*
                    So instead of using the above mentioned ways I went for the following...
                 */
                ptr = NULL;

                /* 
                    Should be called explicitly. The destructor of the vector by the name of Dimensions will be called explicitly
                    //shape.~Dimensions();
                    //shape = DIMENSIONS {0, 0, NULL, NULL};
                 */

                // Just for documentation purposes
                reference_count = 0;                
            }            
        }

        /**
         * @brief Copy Assignment Operator
         * 
         * This copy assignment operator allows for the assignment of one Collective instance to another.
         * It ensures proper handling of reference counting, preventing resource leaks.
         *
         * @param other The Collective instance to be assigned from.
         * @return A reference to the current instance after assignment.
         */
        Collective& operator= (Collective &other) throw (ala_exception)
        {            
            // Check for self-assignment
            if (this == &other)
            {
                return *this;
            }

            if (ptr != NULL)
            {
                cc_tokenizer::allocator<E>().deallocate(ptr);

                ptr = NULL;
            }
            
            // Decrement the reference count of the current instance            
            /*
                FOR DOCUMENTATION PURPOSES...
                This assignment operator used to have back-to-back calls to decrementReferenceCount() and incrementReferenceCount().
                Why were they there? They are still here, but now they are commented because I finally discovered the reason for my hour-long (maybe less) misery and pain.
                It wasn't a mistake; no, it was a blunder. I know myself, and that's why I'm confident I'll make the same, or perhaps even a more spectacularly stupid, error again.

                P.S.: I've left these comments here as a digital monument to my intellectual acrobatics. A reminder for my future self: "Don't pull a 'decrement-increment' stunt again!"
             */
            /* decrementReferenceCount(); */

            // Update the pointer and shape from the other instance
            /*ptr = other.ptr;
            shape = other.shape;*/

            reference_count = 0;

            try 
            {
                ptr = cc_tokenizer::allocator<E>().allocate(other.getShape().getN());
            }
            catch (std::bad_alloc &e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::assignment operator() Error: ") + cc_tokenizer::String<char>(e.what()));                
            }

            for (size_t i = 0; i < other.getShape().getN(); i++)
            {
                ptr[i] = other[i];
            }
            shape = other.shape;

            // Increment the reference count of the new instance
            /*
                FOR DOCUMENTATION PURPOSES...
             */
            /* incrementReferenceCount(); */

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
            if (index >= shape.getN())
            {
                throw ala_exception("Collective::operator[] Error: Index out of range.");
            }
            
            return ptr[index];
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
        Collective<E> operator* (F n) throw (ala_exception)
        {                        
            F* ptr = NULL;

            try
            {
                ptr = cc_tokenizer::allocator<F>().allocate(getShape().getN());
            }
            catch (std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + cc_tokenizer::String<char>(e.what()));    
            }

            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < shape.getN(); i++)
            {
                ptr[i] = (*this)[i] * n;
            }

            return Collective<E>{ptr, getShape().copy()};
        }

        // Overloading the binary * operator for multiplying two Collective instances
        Collective<E> operator* (Collective<E>& other) throw (ala_exception)
        {         
            try
            {                
                return Numcy::dot(*this, other);                
            }
            catch(ala_exception e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Collective::operator*() Error: ") + e.what());
            }                    
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
    Collective<E> operator+ (Collective<E>& other) 
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

                    *current = {0, 0, NULL, NULL};    

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

            ret = {reinterpret_cast<E*>(alloc_obj.allocate(sizeof(E)*n*(a[this->shape.getNumberOfLinks()] + b[other.shape.getNumberOfLinks()]))), *dim_head};
        
            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < n; i++)
            {
                for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < a[this->shape.getNumberOfLinks()]; j++)
                {
                    //ret.ptr[i*(a.ptr[this->shape.getNumberOfLinks()] + b.ptr[other.shape.getNumberOfLinks()]) + j] = this->ptr[j];

                    ret.ptr[i*(a[this->shape.getNumberOfLinks()] + b[other.shape.getNumberOfLinks()]) + j] = this->ptr[j];
                }

                for (cc_tokenizer::string_character_traits<char>::size_type j = a[this->shape.getNumberOfLinks()]; j < a[this->shape.getNumberOfLinks()] + b[other.shape.getNumberOfLinks()]; j++)
                {
                    //ret.ptr[i*(a.ptr[this->shape.getNumberOfLinks()] + b.ptr[other.shape.getNumberOfLinks()]) + j] = other.ptr[j - a.ptr[this->shape.getNumberOfLinks()]];

                    ret.ptr[i*(a[this->shape.getNumberOfLinks()] + b[other.shape.getNumberOfLinks()]) + j] = other.ptr[j - a[this->shape.getNumberOfLinks()]];
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
};
#endif