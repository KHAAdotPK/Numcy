/*
    numcy/numcy.hh
    Q@Khaa.pk
 */

#include "header.hh"

#ifndef REPLIKA_PK_NUMCY_HEADER_HH
#define REPLIKA_PK_NUMCY_HEADER_HH

/*
   Does not modify the internal state of the class within the most of the methods.
 */
class Numcy
{       
    public:    
        /*
            The product of two matrices can be computed by multiplying elements of the first row of the first matrix with the first column 
            of the second matrix then, add all the product of elements. Continue this process until each row of the first matrix is multiplied 
            with each column of the second matrix.

            [2.3 3.4 1.2 1.0] * [2.3  8.9]  = [(2.3*2.3 + 3.4*1.0 + 1.2*9.0 + 1.0*3.2) (2.3*8.9 + 3.4*1.0 + 1.2*2.3 + 1.0*8.9)] = [22.69  35.53]
                                [1.0  1.0]
                                [9.0  2.3]
                                [3.2  8.9]                                           
         */
        /**
         * Calculate the dot product of two matrices represented by Collective objects.
         *
         * @tparam E        The element type of the matrices.
         * @param a         The first input matrix.
         * @param b         The second input matrix.
         * @return          A new Collective object representing the result of the dot product.
         * @throws ala_exception If the matrices have incompatible shapes for dot product.
         *
         * This function computes the dot product of two matrices by multiplying elements
         * of the first matrix's rows with the corresponding columns of the second matrix
         * and summing the results. The resulting matrix is returned as a new Collective object.
         *
         * Example:
         * Matrix A:
         *   [2.3 3.4 1.2 1.0]
         *   [1.0 1.0 9.0 2.3]
         *   [3.2 8.9 2.3 8.9]
         *
         * Matrix B:
         *   [2.3 8.9]
         *   [1.0 1.0]
         *   [9.0 2.3]
         *   [3.2 8.9]
         *
         * Resulting Dot Product:
         *   [22.69 35.53]
         *   [12.6  27.56]
         *   [24.07 98.81]
         *
         * @note The input matrices must have compatible shapes for dot product, i.e., the
         *       number of columns in the first matrix must match the number of rows in the second matrix
         *       or second matrix is a scalar.
         */
        template<typename E = double>
        static Collective<E> dot(Collective<E> a, Collective<E> b) throw (ala_exception)
        {               
            if (a.getShape().getNumberOfColumns() != b.getShape().getDimensionsOfArray().getNumberOfInnerArrays())
            {
                //throw ala_exception("Numcy::dot(): Incompatible shapes for dot product: Number of columns in the first matrix does not match the number of rows in the second matrix.");
                if (!(b.getShape().getN() == 1))
                {
                    // Needs exception message...
                    throw ala_exception("Numcy::dot() Error: Incompatible shapes for dot product...\neither number of columns of the first matrix must match the number of rows of the second matrix,\nor second matrix has to be a scalar.");
                }
            }
            
            std::function</*E*()*/ Collective<E>()> multiplier_is_scalar_func = [&a, &b]/* That is called capture list */() -> /*E**/ Collective<E>
            {
                Collective<E> ret;                
                try 
                {
                    E* ptr = cc_tokenizer::allocator<E>().allocate(a.getShape().getN());
                    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < a.getShape().getN(); i++)
                    {
                        ptr[i] = a[i] * b[0];
                    }
                    ret = Collective<E>{ptr, a.getShape().copy()};                    
                }    
                catch(std::length_error& e)
                {
                    throw ala_exception(cc_tokenizer::String<char>("Numcy::dot() Error: ") + cc_tokenizer::String<char>(e.what())); 
                }
                catch (std::bad_alloc& e)
                {
                    throw ala_exception(cc_tokenizer::String<char>("Numcy::dot() Error: ") + cc_tokenizer::String<char>(e.what())); 
                }
                catch (ala_exception& e)
                {
                    throw ala_exception(cc_tokenizer::String<char>("Numcy::dot() Error: ") + cc_tokenizer::String<char>(e.what()));
                }    
                
                return ret;
            };                        
            /*
                Immutable capture list...
                ----------------------------
                Originaly this was the lambda's ca[ture list [a, b]. 
                It captures the variables a and b by value.
                This means that func has access to the values of a and b as they were when the lambda was created.
                With the above capture list, I was getting compile time errors...
                - "error C2662: 'DIMENSIONSOFARRAY Dimensions::getNumberOfRows(void)': cannot convert 'this' pointer from 'const DIMENSIONS' to 'Dimensions'"
                - "error C2662: 'cc_tokenizer::string_character_traits<char>::size_type Dimensions::getNumberOfColumns(void)': cannot convert 'this' pointer from 'const DIMENSIONS' to 'Dimensions &'"

                Then I change it to [&a, &b]...
                ----------------------------------
                If you need to modify a and b within the lambda, you should capture them by reference using [&a, &b] in the capture list.
                However, this would require that a and b are mutable in the outer scope.
             */                        
            std::function</*E*()*/ Collective<E>()> func = [&a, &b]/* That is called capture list */() -> /*E**/ Collective<E>
            {                  
                cc_tokenizer::allocator<char> alloc_obj;
                E* ptr = reinterpret_cast<E*>(alloc_obj.allocate(sizeof(E)*a.shape.getNumberOfRows().getNumberOfInnerArrays()*b.shape.getNumberOfColumns()));
                /*
                    TODO, check if it is actually working
                 */
                memset(ptr, 0, sizeof(E)*b.shape.getNumberOfColumns());
                                
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < b.shape.getNumberOfColumns(); i++)
                {                    
                    for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < a.shape.getNumberOfColumns(); j++)
                    {
                        ptr[i] = ptr[i] + a[j]*b[b.shape.getNumberOfColumns()*j + i];
                    }   
                }

                struct Collective<E> ret = Collective<E>{ptr, DIMENSIONS{b.shape.getNumberOfColumns(), a.shape.getNumberOfRows().getNumberOfInnerArrays(), NULL, NULL}};
                
                return ret;
            };
           
           if (b.getShape().getN() == 1)
           {                   
                return multiplier_is_scalar_func();
           }
           else 
           {                
                return func();
           }
           //return NULL;
        }
        
        /*
            Euclidean norm corresponds to the square root of the sum of the absolute squared values of the vector's components

                                    ||u|| = sqrt(u1^2 + u2^2 + u3^2 + .... + (u(n-1))^2 + un^2)

            The notation with two vertical bars ||u|| represents the magnitude or norm of the vector u.
            In the context of vectors, the magnitude of a vector is the length of the vector in space, 
            and it is calculated using the square root of the sum of the squares of its components.
         */
        /*
            This function, enorm(), computes the Euclidean norm of a vector of type Collective<E> by iterating
            over its components, squaring each element, summing these squared values, and returning the square root
            of the sum as the magnitude.

            Parameters:
            - x: A vector of elements of type E, representing the input vector.

            Returns:
            - E: The Euclidean norm of the input vector x.

            Throws:
            - ala_exception: If any error occurs during the calculation.
         */
        template <typename E = double>
        static E enorm(Collective<E> x) throw (ala_exception)
        {
            E sum = 0;

            try 
            {  
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < x.getShape().getN(); i++)
                {                
                    x[i] = x[i]*x[i];

                    sum = sum + x[i];
                }

                return sqrt(sum); 
            }
            catch (ala_exception& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::enorm() -> ") + e.what());
            }           
        }
                
        class Spatial
        {
            public:
                class Distance
                {
                    public:
                        /*
                            The method finds cosine similarity between two vectors.
                                                       u.v
                            cosine_similarity =  -----------------
                                                   ||u|| x ||v||​


                                                        OR 

                                                                u * v          
                            cosine_similarity =  ---------------------------------
                                                 Numcy::enorm(u) * Numcy::enorm(v)​   

                            Cosine Similarity ranges from -1 to 1...
                            ->  1 indicates vectors are identical (perfect similarity). 
                            ->  0 indicates the vectors are orthogonal (no similarity).
                            -> -1 indicates the vectors are completely opposite (dissimilarity)
                         */
                        /*
                            Parameters:
                            - u: A vector of elements of type E, representing the first input vector.
                            - v: A vector of elements of type E, representing the second input vector.

                            Returns:
                            - E: The cosine similarity value between the two vectors.

                            Throws:
                            - ala_exception: If any error occurs during the calculation.
                         */
                        template <typename E = double>
                        static E cosine(Collective<E>& u, Collective<E>& v) throw (ala_exception)
                        {
                            try 
                            {
                                /*
                                    It computes the dot product of the vectors using Numcy::dot(u, v) and 
                                    stores the result in a Collective<E> named product
                                 */
                                Collective<E> product = Numcy::dot(u, v);

                                //std::cout<< product.getShape().getNumberOfColumns() << product.getShape().getDimensionsOfArray().getNumberOfInnerArrays() << std::endl;

                                //Numcy::enorm(u);
                                //Numcy::enorm(v);

                                //return product;

                                return ((product[0])/(Numcy::enorm(u)*Numcy::enorm(v)));
                            }
                            catch (ala_exception& e)
                            {
                                throw ala_exception(cc_tokenizer::String<char>("Numcy::cosine() -> ") + e.what());
                            }
                        }
                };

                class Distance distance;     
        };

        class Spatial spatial;

        class Random
        {
            public:
                /*
                    TODO,
                    Make it variadic function

                    Generates random numbers following a normal distribution with a specified mean and standard deviation, and it returns an object of type Collective<E>. 
                 */
                template <typename E = double>
                static Collective<E> randn(DIMENSIONS& like) throw (ala_exception)
                {                    
                    cc_tokenizer::string_character_traits<char>::size_type n = like.getN();
                    
                    if (n == 0)
                    {
                        throw ala_exception("Numcy::Random::randn() Error: Malformed shape of the array to be returned");
                    }

                    E* ptr = NULL;

                    try 
                    {
                        ptr = cc_tokenizer::allocator<E>().allocate(n);
                    }
                    catch (ala_exception& e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Numcy::Random::randn() Error: ") + cc_tokenizer::String<char>(e.what()));
                    }

                    std::random_device rd{};
                    std::mt19937 gen{rd()};

                    // values near the mean are the most likely
                    // standard deviation affects the dispersion of generated values from the mean
                    std::normal_distribution<> nd{NUMCY_DEFAULT_MEAN, NUMCY_DEFAULT_STANDARD_DEVIATION};

                    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < n; i++)
                    {
                        ptr[i] = nd(gen);                        
                    }

                    return Collective<E>{ptr, like.copy()};
                }

                /*
                    Return random integers from low (inclusive) to high (exclusive).
                 */
                //static int* randint(int, int, DIMENSIONS shape = {1, 1, NULL, NULL}) throw(ala_exception);
                static int* randint(int low, int high, DIMENSIONS shape) throw (ala_exception)
                {
                    cc_tokenizer::string_character_traits<char>::size_type n = shape.getN();

                    if (n == 0)
                    {
                        throw ala_exception("Numcy::Random::randint() Error: Malformed shape of the array to be returned");
                    }

                    cc_tokenizer::allocator<char> alloc_obj;

                    int* ptr = reinterpret_cast<int*>(alloc_obj.allocate(sizeof(int)*n));

                    int range = high - low;

                    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < n; i++)
                    {        
                        ptr[i] = (low + (rand() % range));
                    }
    
                    return ptr;
                }

                template <class E, typename T = cc_tokenizer::string_character_traits<char>::size_type>
                static void shuffle(E& obj, T n)
                {
                    std::random_device rd;
                    std::mt19937 gen(rd());

                    std::uniform_int_distribution<T> dist(0, n);

                    for (T i = 0; i < n; i++)
                    {
                        T a = dist(gen);
                        T b = dist(gen);

                        obj.shuffle(a, b);
                    }
                }
        };
            
        class Random random;

        /*
            @stop, integer or real; end of interval. The interval does not include this value, except in some cases where step is not an integer and floating point round-off affects the length of out.
            @step, integer or real, optional; spacing between values. For any output out, this is the distance between two adjacent values, out[i+1] - out[i]. The default step size is 1. If step is specified as a position argument, start must also be given.
            @like, array_like, optional; at the moment it is here for documentation purposes.
            Returns an array like the instance of DIMENSIONS passed
         */
        template <typename E = float, typename OutType = float>
        OutType* arange(E stop, E step = 1 /*NUMCY_DTYPE dtype = NUMCY_DTYPE_FLOAT*/, DIMENSIONS like = {1, 1, NULL, NULL}) const throw (ala_exception)
        {            
            return arange<E, OutType>(0, stop, step, like);
        }

        /*
            @start, integer or real; start of interval. The default start value is 0 and the interval includes this value.
            @stop, integer or real; end of interval. The interval does not include this value, except in some cases where step is not an integer and floating point round-off affects the length of out.
            @step, integer or real, optional; spacing between values. For any output out, this is the distance between two adjacent values, out[i+1] - out[i]. The default step size is 1. If step is specified as a position argument, start must also be given.
            @like, array_like, optional; at the moment it is here for documentation purposes.
            Returns an array like the instance of DIMENSIONS passed 
         */
        template <typename E = float, typename OutType = float>
        static OutType* arange(E start /*= 0*/, E stop, E step = 1 /*NUMCY_DTYPE dtype = NUMCY_DTYPE_FLOAT*/, DIMENSIONS like = {1, 1, NULL, NULL}) throw (ala_exception)
        {               
            if ((stop - start)/step > like.getN())
            {   
                unsigned int num = (stop - start)/step;
                throw ala_exception(cc_tokenizer::String<char>("Numcy::arange(): Range Size Error - The requested range size of ") + cc_tokenizer::String<char>(num) + cc_tokenizer::String<char>(" exceeds the requested size ") + cc_tokenizer::String<char>(like.getN()) + cc_tokenizer::String<char>(" of the to be allocated memory block's capacity to store the generated values."));
            }

            cc_tokenizer::allocator<char> alloc_obj;
            cc_tokenizer::string_character_traits<char>::size_type i = 0;
            E current = start;
            OutType* ptr = NULL;

            try
            {
                //ptr = reinterpret_cast<OutType*>(alloc_obj.allocate(sizeof(OutType)*like.getN())); 
                ptr = cc_tokenizer::allocator<OutType>().allocate(like.getN());
            }
            catch(const std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::arange() Error: ") + cc_tokenizer::String<char>(e.what()));
            }
            catch(const std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::arange() Error: ") + cc_tokenizer::String<char>(e.what()));
            }
                         
            while (i < like.getN() && current  < stop)
            {                
                ptr[i] = current;

                current = current + step;
                
                i++;
            }    
                                                
            return /*(OutType*)*/ ptr;            
        }
        
        template<typename E>
        static struct Collective<E> concatenate(struct Collective<E> a, struct Collective<E> b, AXIS axis=AXIS_COLUMN) throw (ala_exception)
        {  
            E *ptr = NULL;
            struct Collective<E> ret = {NULL, {0, 0, NULL, NULL}};

            //a.shape.getDimensionsOfArray().compare(b.shape.getDimensionsOfArray());
            /*
            std::cout<<"a = is.Columns = " << std::endl;
            std::cout<< a.getShape().getNumberOfColumns() << std::endl;
            std::cout<< a.getShape().getNumberOfRows().getNumberOfInnerArrays() << std::endl;
            std::cout<<"b = pe.Columns " << std::endl;
            std::cout<< b.getShape().getNumberOfColumns() << std::endl;
            std::cout<< b.getShape().getNumberOfRows().getNumberOfInnerArrays() << std::endl;
             */
                        
            switch(axis)
            {                
                /*
                    -------------------------
                    | Stacking horizontally |
                    -------------------------
                    a[3][256]
                    b[1][3] 
                    After this we've an array [3][257]
                    
                    We need to check that number of rows in first array (a) is same as number of columns in second array (b).
                 */
                case AXIS_COLUMN:

                    // Validation of Shapes
                    if (a.getShape().getDimensionsOfArray().getNumberOfInnerArrays() != b.getShape().getNumberOfColumns())
                    {                        
                        throw ala_exception("Error in Collective::concatenate(AXIS_COLUMNS): Unable to concatenate instances of Collective composite. The number of rows in the first instance does not match the number of columns in the second instance.");
                    }

                    // Memory Allocation
                    try 
                    {
                        ptr = reinterpret_cast<E*>(cc_tokenizer::allocator<float>().allocate( a.getShape().getN() + b.getShape().getN() ));
                    }
                    catch (std::bad_alloc e)
                    {
                        throw ala_exception("Error in Error in Collective::concatenate(AXIS_COLUMNS): Caught std::bad_alloc.");     
                    }

                    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < (a.getShape().getN() + b.getShape().getN()); i++)
                    {
                        ptr[i] = 0;
                    }

                    // Concatenation Loop
                    for (int i = 0; i < a.getShape().getDimensionsOfArray().getNumberOfInnerArrays(); i++)                    
                    {                        
                        for (int j = 0; j < a.getShape().getNumberOfColumns(); j++)
                        {                            
                            ptr[i*a.getShape().getNumberOfColumns() + (i + j)] = a[i*a.getShape().getNumberOfColumns() + j]; 
                        }
                        
                        ptr[i*a.getShape().getNumberOfColumns() + a.getShape().getNumberOfColumns() + i] = b[i];                        
                    }

                    ret = Collective<E>{ptr, {a.getShape().getNumberOfColumns() + 1, a.getShape().getDimensionsOfArray().getNumberOfInnerArrays(), NULL, NULL}};

                break;
            }
            
            return ret;
        }

        template<typename E>
        static struct Collective<E> cos(Collective<E> x) 
        {
            cc_tokenizer::allocator<char> alloc_obj;

            struct Collective<E> ret = {reinterpret_cast<E*>(alloc_obj.allocate(sizeof(E)*x.shape.getN())), {x.shape.getNumberOfColumns(), x.shape.getNumberOfRows().getNumberOfInnerArrays(), NULL, NULL}};

            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < x.shape.getN(); i++)
            {
                ret[i] = std::cos(x[i]);
            }

            return ret;
        }

        template<typename E>
        static Collective<E> divide(Collective<E>& a, Collective<E>& b) throw (ala_exception)
        {   
            if (!a.getShape().getN() || !b.getShape().getN())
            {
                throw ala_exception("Numcy::divide() Error: Malformed shape of the array received as one of the arguments");
            }

            E* ptr = NULL;
            try
            {
                ptr = cc_tokenizer::allocator<E>().allocate(a.getShape().getN());
            }
            catch(std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::divide() Error: ") + cc_tokenizer::String<char>(e.what()));
            }

            
            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < a.getShape().getN(); i++)
            {
                ptr[i] = a[i] / b[0];
            }
            
            return Collective<E>{ptr, a.getShape().copy()};
        }
            
        /*
            Output array, element-wise exponential of x. This is a scalar if @a is a scalar
         */
        template<typename E>
        static E* exp(E* a, cc_tokenizer::string_character_traits<char>::size_type n) throw (ala_exception)
        {                         
            E* ret = NULL; 
            try
            {                                  
                ret = cc_tokenizer::allocator<E>().allocate(n);
            }
            catch (std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::exp() Error: ") + cc_tokenizer::String<char>(e.what()));
            }

            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < n; i++)
            {
                ret[i] = ::exp(a[i]);
            }
            
            return ret;
        }

        template<typename E>
        static Collective<E> exp(Collective<E>& a) throw (ala_exception)
        {            
            if (!a.getShape().getN())
            {
                throw ala_exception("Numcy::exp() Error: Malformed shape of the array received as one of the arguments.");
            }

            E* ptr = NULL;

            try
            {
                ptr = cc_tokenizer::allocator<E>().allocate(a.getShape().getN());
            }
            catch (std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::exp() Error: ") + cc_tokenizer::String<char>(e.what()));
            }
            catch (std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::exp() Error: ") + cc_tokenizer::String<char>(e.what()));
            }

            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < a.getShape().getN(); i++)
            {
                ptr[i] = ::exp(a[i]);
            }

            return Collective<E>{ptr, a.getShape().copy()};
        }

        /* 
            Returns a scalar, the maximum element in an array @a             
            @a, array of type E            
         */
        template<typename E>
        static Collective<E> max(Collective<E>& a) throw (ala_exception)
        {            
            if (!a.getShape().getN())
            {
                throw ala_exception("Numcy::max() Error: Malformed shape of the array received as one of the arguments");
            }

            // Initialize the maximum element to the first element of the array
            E me = a[0];

            // Iterate over the remaining elements of the array
            for (cc_tokenizer::string_character_traits<char>::size_type i = 1; i < a.getShape().getN(); i++)
            {
                // If the current element is greater than the maximum element, update the maximum element
                if (a[i] > me)
                {
                    me = a[i];
                }
            }

            E* ptr = NULL;

            try 
            {
                ptr = cc_tokenizer::allocator<E>().allocate(1);                
            }
            catch (std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::max() Error: ") + cc_tokenizer::String<char>(e.what()));
            }    
            catch (std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::max() Error: ") + cc_tokenizer::String<char>(e.what()));
            }

            *ptr = me;
            
            return Collective<E>{ptr, DIMENSIONS{1, 1, NULL, NULL}};
        }

        template<typename E = double, typename F = cc_tokenizer::string_character_traits<char>::size_type>
        static Collective<E> mean(Collective<E>& a, Collective<F>& like, AXIS axis = AXIS_ROWS) throw (ala_exception)
        {
            if (!a.getShape().getN())
            {
                throw ala_exception("Numcy::mean() Error: Malformed shape of the array received as one of the arguments.");
            }

            E* ptr = NULL;
            Collective<E> ret;

            switch (axis)
            {
                case AXIS_ROWS:
                    try
                    {  
                        ptr = cc_tokenizer::allocator<E>().allocate(a.getShape().getNumberOfColumns());

                        for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < a.getShape().getNumberOfColumns(); i++)
                        {
                            ptr[i] = 0;
                        }

                        if (like.getShape().getN())
                        {                                                                            
                            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < like.getShape().getN(); i++)
                            {
                                for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < a.getShape().getNumberOfColumns(); j++)
                                {
                                    ptr[j] = ptr[j] + a[like[i]*a.getShape().getNumberOfColumns() + j];
                                }
                            }

                            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < a.getShape().getNumberOfColumns(); i++)
                            {
                                ptr[i] = ptr[i] / like.getShape().getN();
                            }

                            ret = Collective<E>{ptr, DIMENSIONS{a.getShape().getNumberOfColumns(), 1, NULL, NULL}};
                        }
                        else
                        {
                            // What to do here for this axis...
                            /*
                                If a has only one inner array, then return that inner array as it is.
                                On the other hand if a has few or many inner arrays then take the column wise mean of all inner arrays.
                             */   
                            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < a.getShape().getDimensionsOfArray().getNumberOfInnerArrays(); i++)
                            {
                                for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < a.getShape().getNumberOfColumns(); j++)
                                {
                                    ptr[j] = ptr[j] + a[i*a.getShape().getNumberOfColumns() + j];
                                }
                            }

                            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < a.getShape().getNumberOfColumns(); i++)
                            {
                                ptr[i] = ptr[i] / like.getShape().getDimensionsOfArray().getNumberOfInnerArrays();
                            }

                            ret = Collective<E>{ptr, DIMENSIONS{a.getShape().getNumberOfColumns(), 1, NULL, NULL}};
                        }
                    }
                    catch (std::bad_alloc& e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Numcy::mean() Error: ") + cc_tokenizer::String<char>(e.what()));
                    }
                    catch (std::length_error& e)
                    {                        
                        throw ala_exception(cc_tokenizer::String<char>("Numcy::mean() Error: ") + cc_tokenizer::String<char>(e.what()));
                    }
                    catch (ala_exception& e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Numcy::mean() Error: ") + cc_tokenizer::String<char>(e.what()));                        
                    }
                                        
                break;
            }
            
            return ret;
        }

        /*
            Return a new array of given shape and type, filled with ones.
            @dim, shape of the array to return
            @order, whether to store multi-dimensional data in row-major (C-style) or column-major (Fortran-style) order in memory
         */
        template<typename E = float>        
        static Collective<E> ones(DIMENSIONS dim, REPLIKA_ROW_MAJOR_OR_COLUMN_MAJOR order = REPLIKA_ROW_MAJOR) throw (ala_exception)
        {
            E* ptr = NULL;        
            cc_tokenizer::string_character_traits<char>::size_type n = dim.getN();
            
            if (n == 0)
            {
                throw ala_exception("Numcy::ones() Error: Dimensional data is empty. Unable to proceed.");
            }

            //ptr = reinterpret_cast<E*>(cc_tokenizer::allocator<E>().allocate(n));
            ptr = cc_tokenizer::allocator<E>().allocate(n);

            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < n; i++)
            {
                ptr[i] = 1;
            }
                    
            return {ptr, {dim.getNumberOfColumns() , dim.getDimensionsOfArray().getNumberOfInnerArrays(), NULL, NULL}};
        }

        /*  
            Compute the outer product of two vectors.       
            @m1, shape (1, M) of 1 row and M columns
            @m2, shape (1, N) of 1 row and N columns
            The shape of returned matrix is (M, N)
                        
            The values of m1.getShape().getN() and m2.getShape().getN() might or might not be the same.

            Given two vectors, a = [a0, a1, ..., aM] and b = [b0, b1, ..., bN], the outer product is...
            [[a0*b0  a0*b1 ... a0*bN ]
             [a1*b0    .
             [ ...          .
             [aM*b0            aM*bN ]]

            Inputs are flattened if not already 1-dimensional

            @m1, first input vector
            @m2, second input vector 

            Returns vector of size (M, N)         
         */
        template<typename E = double>
        static Collective<E> outer(Collective<E>& m1, Collective<E>& m2) throw (ala_exception)
        {               
            /*                
                This commented block should have been removed,
                but I want it to remain for a little longer.
                I need to find out why I put this block of code here in the first place.
             */
            /*if (m1.getShape().getN() != m2.getShape().getN())
            {
                throw ala_exception("Numcy::outer() Error: Input vectors have incompatible shapes. After flattening, both vectors must have the same number of elements.");
            }*/
            
            E* ptr = NULL;

            try 
            {
                ptr = cc_tokenizer::allocator<E>().allocate(m1.getShape().getNumberOfColumns()*m2.getShape().getNumberOfColumns());                
            }
            catch (std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::outer() Error: ") + cc_tokenizer::String<char>(e.what()));
            }

            for (cc_tokenizer::String<char>::size_type i = 0; i < m1.getShape().getNumberOfColumns(); i++)
            {
                for (cc_tokenizer::String<char>::size_type j = 0; j < m2.getShape().getNumberOfColumns(); j++)
                {
                    ptr[i*m2.getShape().getNumberOfColumns() + j] =  m1[i]*m2[j];
                }
            }
 
            return Collective<E> {ptr, DIMENSIONS{m2.getShape().getNumberOfColumns(), m1.getShape().getNumberOfColumns(), NULL, NULL}};
        }
        
        /*
            Reshapes the first matrix to match the shape of the second matrix.
            The reshaping is performed based on the number of columns and the number of inner arrays.

            Parameters:
                m1: First matrix to reshape. This matrix at most can have the same shape as m2 matrix.
                m2: Second matrix whose shape will be matched.

            Throws:
                ala_exception: If the reshape operation fails due to incompatible dimensions.

            Returns:
                A new Collective instance with the reshaped data and shape.
         */
        template<typename E = double>
        static struct Collective<E> reshape(Collective<E>& m1, Collective<E>& m2) throw (ala_exception)
        {
            if (!((m1.getShape().getNumberOfColumns() <= m2.getShape().getNumberOfColumns()) && (m1.getShape().getDimensionsOfArray().getNumberOfInnerArrays() <= m2.getShape().getDimensionsOfArray().getNumberOfInnerArrays())))
            {
                cc_tokenizer::String<char> message1("Numcy::reshape() Error: Reshape operation failed. Incompatible dimensions. For reshape to be successful, Matrix 1 must have: - Less than or equal columns compared to Matrix 2. - Less than or equal inner arrays compared to Matrix 2. ");
                cc_tokenizer::String<char> message2 = cc_tokenizer::String<char>("m1(") + cc_tokenizer::String<char>(m1.getShape().getNumberOfColumns()) + cc_tokenizer::String<char>("c,") + cc_tokenizer::String<char>(m1.getShape().getDimensionsOfArray().getNumberOfInnerArrays()) + cc_tokenizer::String<char>("r) ") + cc_tokenizer::String<char>("m2(") + cc_tokenizer::String<char>(m2.getShape().getNumberOfColumns()) + cc_tokenizer::String<char>("c,") + cc_tokenizer::String<char>(m2.getShape().getDimensionsOfArray().getNumberOfInnerArrays()) + cc_tokenizer::String<char>("r)");

                throw ala_exception(message1 + message2);
            }

            E* ptr = NULL;

            try 
            {
                ptr = cc_tokenizer::allocator<E>().allocate(m2.getShape().getN());
            }
            catch (ala_exception& e)
            {                                
                throw ala_exception(cc_tokenizer::String<char>("Numcy::reshape() Error: ") + cc_tokenizer::String<char>(e.what()));
            }
                        
            memset(ptr, 0, sizeof(E)*m2.getShape().getN());
            /*for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < m2.getShape().getN(); i++)
            {
                ptr[i] = 0.0;
            }*/
            
            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < m1.getShape().getDimensionsOfArray().getNumberOfInnerArrays(); i++)
            {
                for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < m1.getShape().getNumberOfColumns(); j++)
                {
                    ptr[i*m2.getShape().getNumberOfColumns() + j] = m1[i*m1.getShape().getNumberOfColumns() + j];
                }
            }

            return Collective<E>{ptr, m2.getShape().copy()};
        }

        /*
         * This function calculates the sign of elements in a Numcy array 'x'.
         * It returns a new Collective object with the same shape as 'x' containing the signs.
         *
         * Throws exceptions of type 'ala_exception' in the following cases:
         *   - If the input array 'x' has an empty shape (zero elements).
         *   - If memory allocation for the output array fails.
         *
         * The function iterates through each element of 'x':
         *   - If the element is NaN (Not a Number), it's copied directly to the output.
         *   - If the element is a complex number (imaginary part != 0), it's copied directly to the output 
             (since the sign function is not defined for complex numbers in this implementation).
         *  - Otherwise, the sign of the element is determined:
         *  - Negative: -1 is assigned to the output.
         *  - Positive: 1 is assigned to the output.
         *  - Zero: 0 is assigned to the output.
 
         * Note: This implementation treats elements close to zero (within a tolerance level) 
          as zero for efficiency purposes. The tolerance level is not explicitly defined 
          in this code but could be added as a parameter if needed.
        */                  
        template<typename E = double>
        static Collective<E> sign(Collective<E>& x) throw (ala_exception)
        {
            if (!x.getShape().getN())
            {
                throw ala_exception("Numcy::sign() Error: Malformed shape of the array received as one of the arguments.");
            }

            E* ptr = NULL;

            try 
            {
                ptr = cc_tokenizer::allocator<E>().allocate(x.getShape().getN());
            }
            catch (ala_exception& e)
            {                                
                throw ala_exception(cc_tokenizer::String<char>("Numcy::sign() Error: ") + cc_tokenizer::String<char>(e.what()));
            }

            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < x.getShape().getN(); i++)
            {      
                std::complex<E> num(x[i]);
                                  
                if (_isnan(x[i]))
                {
                    ptr[i] = x[i];
                }
                else if (std::imag(num) != 0)
                {
                    ptr[i] = x[i];
                }
                else if (x[i] < 0)
                {
                    ptr[i] = -1;                    
                }
                else if (x[i] > 0)
                {
                    ptr[i] = 1;
                } 
                else if (x[i] == 0)
                {
                    ptr[i] = 0;
                }                            
            }

            return Collective<E>{ptr, x.getShape().copy()};
        }
    
        template<typename E>
        static struct Collective<E> sin(Collective<E> x) 
        {                        
            E* ptr = NULL;             
            Collective<E> ret; 

            try 
            {
                ptr = cc_tokenizer::allocator<E>().allocate(x.getShape().getN());
                ret = Collective<E>{ptr, *(x.getShape().copy())};
            }
            catch (std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::sin() Error: ") + cc_tokenizer::String<char>(e.what()));\
            }
            catch (std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::sin() Error: ") + cc_tokenizer::String<char>(e.what()));\
            }
            
            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < x.getShape().getN(); i++)
            {                
                ret[i] = std::sin(x[i]);
            }

            return ret;
        }

        template<typename E>
        static Collective<E> subtract(Collective<E>& x1, Collective<E>& x2) throw (ala_exception)
        {            
            /*if (!(x1.getShape() == x2.getShape()))
            {
                throw ala_exception("Numcy::subtract() Error: Shapes of operands must be equal.");
            }*/

            if (!x1.getShape().getN() || !x2.getShape().getN())
            {
                throw ala_exception("Numcy::subtract() Error: Both operands must have at least one element.");
            }

            if (x2.getShape().getN() > 1)
            {
                /*
                // TODO, make sure both shapes are equal to each other
                if (x2.getShape().getN() != x1.getShape().getN())
                {
                    throw ala_exception("Numcy::subtract() Error: ");                    
                }
                 */

                if (!(x1.getShape() == x2.getShape()))
                {
                    throw ala_exception("Numcy::subtract() Error: Shapes of operands must be equal.");
                }
            }

            /*
                We are here, now either the shapes of both matrices is same or the x2 is a scalar
             */

            E* ptr = NULL;

            try 
            {
                /*
                    We used the size of matrix x1, because x2 could be scalar
                 */
                ptr = cc_tokenizer::allocator<E>().allocate(x1.getShape().getN());
            }
            catch (std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::subtract() Error: ") + cc_tokenizer::String<char>(e.what()));
            } 
            catch (std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::subtract() Error: ") + cc_tokenizer::String<char>(e.what()));                
            }

            if (x2.getShape().getN() > 1)
            {
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < x1.getShape().getN(); i++)
                {
                    ptr[i] = x1[i] - x2[i];
                }
            }
            else 
            {
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < x1.getShape().getN(); i++)
                {
                    ptr[i] = x1[i] - x2[0];
                }
            }
                        
            return Collective<E>{ptr, x1.getShape().copy()};            
        }

        template<typename E>
        static Collective<E> sum(Collective<E>& m1, Collective<E>& m2, AXIS axis = AXIS_NONE) throw (ala_exception)
        {   
            if (!m1.getShape().getN() || !m2.getShape().getN())
            {
                throw ala_exception("Numcy::sum() Error: Atleast one of the vectors is empty.");
            }

            E* ptr = NULL;
            DIMENSIONS dim;

            switch (axis)
            {
                case AXIS_NONE:
                   if (!(m1.getShape() == m2.getShape()))
                   {                
                        if (!(m2.getShape().getN() == 1))
                        {
                            throw ala_exception("Numcy::sum() Error: Shape of both input vectors is either not same or, the size of second vector argument is not 1.");
                        }
                    }

                    try 
                    {
                        ptr = cc_tokenizer::allocator<E>().allocate(m1.getShape().getN());
                        dim = *(m1.getShape().copy());
                    } 
                    catch (std::length_error& e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Numcy::sum() Error: ") + cc_tokenizer::String<char>(e.what()));
                    }
                    catch (std::bad_alloc& e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Numcy::sum() Error: ") + cc_tokenizer::String<char>(e.what()));
                    }

                    if (m2.getShape().getN() == 1)
                    {
                        for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < m1.getShape().getN(); i++)
                        {
                            ptr[i] = m1[i] + m2[0];
                        }
                    }
                    else
                    {
                        for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < m1.getShape().getN(); i++)
                        {
                            ptr[i] = m1[i] + m2[i];
                        }
                    }
                break;
                case AXIS_ROWS:
                    if (m1.getShape().getNumberOfColumns() != m2.getShape().getNumberOfColumns())
                    {
                        throw ala_exception("Numcy::sum() Error: Summation across rows assumes same number of columns for both input vectors.");
                    }

                    //cc_tokenizer::string_character_traits<char>::size_type n = (m1.getShape().getN() > m2.getShape().getN()) ? m1.getShape().getN() : m2.getShape().getN();

                    try 
                    {
                        ptr = cc_tokenizer::allocator<E>().allocate((m1.getShape().getN() > m2.getShape().getN()) ? m1.getShape().getN() : m2.getShape().getN());
                        dim = (m1.getShape().getN() > m2.getShape().getN()) ? *(m1.getShape().copy()) : *(m2.getShape().copy());
                    } 
                    catch (std::length_error& e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Numcy::sum() Error: ") + cc_tokenizer::String<char>(e.what()));
                    }
                    catch (std::bad_alloc& e)
                    {
                        throw ala_exception(cc_tokenizer::String<char>("Numcy::sum() Error: ") + cc_tokenizer::String<char>(e.what()));
                    }    

                    if (m1.getShape().getN() > m2.getShape().getN())
                    {
                        for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < (m1.getShape().getN() / m2.getShape().getN()); i++)
                        {
                            for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < m2.getShape().getN(); j++)
                            {
                                ptr[i*m2.getShape().getN() + j] = m1[i*m2.getShape().getN() + j] + m2[j];
                            }
                        }
                    }
                    else
                    {
                        for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < (m2.getShape().getN() / m1.getShape().getN()); i++)
                        {
                            for (cc_tokenizer::string_character_traits<char>::size_type j = 0; j < m1.getShape().getN(); j++)
                            {
                                ptr[i*m1.getShape().getN() + j] = m1[i*m1.getShape().getN() + j] + m1[j];
                            }
                        }
                    }

                break;
            }
/*
            if (!(m1.getShape() == m2.getShape()))
            {                
                if (!(m2.getShape().getN() == 1))
                {
                    throw ala_exception("Numcy::sum() Error: Shape of both input vectors is either not same or, the size of second vector argument is not 1.");
                }
            }

            E* ptr = NULL;

            try
            {
                ptr = cc_tokenizer::allocator<E>().allocate(m1.getShape().getN());
            }
            catch(std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::sum() Error: ") + cc_tokenizer::String<char>(e.what()));
            }

            if (m2.getShape().getN() == 1)
            {
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < m1.getShape().getN(); i++)
                {
                    ptr[i] = m1[i] + m2[0];
                }
            }
            else
            {
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < m1.getShape().getN(); i++)
                {
                    ptr[i] = m1[i] + m2[i];
                }
            }

            return Collective<E>{ptr, m1.getShape().copy()};
*/
            return Collective<E>{ptr, *(dim.copy())};
        } 

        template<typename E>
        static Collective<E> sum(Collective<E>& a, AXIS axis = AXIS_NONE) throw (ala_exception)
        {
            if (!a.getShape().getN())
            {
                throw ala_exception("Numcy::sum() Error: Malformed shape of the array received as one of the arguments");
            }

            E* ptr = NULL;

            try
            {
                ptr = cc_tokenizer::allocator<E>().allocate(1);
            }
            catch(std::length_error& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::sum() Error: ") + cc_tokenizer::String<char>(e.what()));
            }
            catch(std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::sum() Error: ") + cc_tokenizer::String<char>(e.what()));
            }

            *ptr = 0;
            
            switch (axis) 
            {
               case AXIS_NONE: // The default, axis=None, will sum all of the elements of the input array
                   for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < a.getShape().getN(); i++)
                   {
                       *ptr = *ptr + a[i];  
                   } 
               break;
               default:
                   throw ala_exception("Numcy::sum() Error: Unknown axis."); 
               break; 
            }

            return Collective<E>{ptr, DIMENSIONS{1, 1, NULL, NULL}};
        }

        template<typename E = double>
        static Collective<E> transpose(Collective<E>& m) throw (ala_exception)
        {
            if (!m.getShape().getN())
            {
                throw ala_exception("Numcy::transpose() Error: Input vector is empty.");
            }

            E* ptr = NULL;

            try 
            {
                ptr = cc_tokenizer::allocator<E>().allocate(m.getShape().getNumberOfColumns()*m.getShape().getDimensionsOfArray().getNumberOfInnerArrays());                
            }
            catch (std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::transpose() Error: ") + cc_tokenizer::String<char>(e.what()));
            } 

            for (cc_tokenizer::String<char>::size_type row = 0; row < m.getShape().getDimensionsOfArray().getNumberOfInnerArrays(); row++)
            {
                for (cc_tokenizer::String<char>::size_type column = 0; column < m.getShape().getNumberOfColumns(); column++)
                {
                    ptr[column*m.getShape().getDimensionsOfArray().getNumberOfInnerArrays() + row] =  m[row*m.getShape().getNumberOfColumns() + column];
                }
            }

            return Collective<E>{ptr, DIMENSIONS{m.getShape().getDimensionsOfArray().getNumberOfInnerArrays(), m.getShape().getNumberOfColumns(), NULL, NULL}};
        }

        /*
            Extracts the upper triangular part of a 2-dimensional array.
    
            @m: Input matrix or array, must be at least 2-dimensional.
            @k: Optional integer parameter specifying the diagonal below which values are zeroed out.
                Default value is 0, zeroing out values below the main diagonal. 
                For a positive integer k, it zeros out values below the k-th diagonal (above the main diagonal).
                For a negative integer k, it zeros out values below the -k-th diagonal (below the main diagonal).

            Here's an example to illustrate how Numcy::triu() works:

            ```C++    
            Numcy::triu(Numcy::ones<t>({3, 3, NULL, NULL}), 0, true);
            ```
            Output:
            1 1 1
            0 1 1
            0 0 1

            In this example, Numcy::triu() zeroed out the values below the main diagonal, resulting in the upper triangular part of the original matrix. 
            You can also specify a different value of k to zero out values below or above a different diagonal.
         */
        template<typename E>
        static Collective<E> triu(Collective<E>& m, int k = 0, bool verbose = false) throw (ala_exception)
        {             
            if (!m.getShape().getN() || !m.getShape().getDimensionsOfArray().getNumberOfInnerArrays())
            {
                throw ala_exception("Numcy::triu() Error: Dimensional data is empty. Unable to proceed.");
            }
                        
            // For arrays with ndim exceeding 2, triu will apply to the final two axes.
            cc_tokenizer::string_character_traits<char>::size_type rows = NUMBER_OF_ROWS_IN_INNER_MOST_ARRAY(m), columns = NUMBER_OF_COLUMNS_IN_INNER_MOST_ARRAYS(m);
                                    
            E* ptr = cc_tokenizer::allocator<E>().allocate(m.getShape().getN());
            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < m.getShape().getN(); i++)
            {
                ptr[i] = m[i];
            }

            // Iterate over innermost arrays.            
            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < (m.getShape().getN() / (rows*columns));  i++)
            {   
                // Pointer to the start of the innermost array in the modified memory location
                E* innerMostArray = ptr + (i*rows*columns);

                // Iterate over rows of the innermost array
                for (cc_tokenizer::string_character_traits<char>::size_type q = (k < 0) ? k*-1 : 0; q < rows; q++) 
                {   
                    // Zero out elements below the specified diagonal                                                                                                                                                                              
                    for (cc_tokenizer::string_character_traits<char>::size_type i =  0; ((i < (k + q)) && (i < columns)); i++)                               
                    {
                        innerMostArray[q*columns + i] = 0;
                    }                                                
                }
            }

            /*
                Debugging Output (Verbose Mode):
                This block of code generates detailed output for debugging purposes when 'verbose' mode is enabled.
                It prints the modified array, row by row, in a human-readable format, facilitating inspection and analysis.
                Please note that this code is intended for debugging only and provides detailed information about
                the modified array's structure. In the final production code, it will be removed to optimize performance
                and reduce unnecessary output.
             */
            if (verbose) 
            {
                for (cc_tokenizer::string_character_traits<char>::size_type i = 0; /*(i < n)*/ i < (m.getShape().getN() / (rows*columns)); i++)
                {             
                    E* innerMostArray = ptr + (i*rows*columns); 
                    for (cc_tokenizer::string_character_traits<char>::size_type q = 0; q < rows; q++)
                    {
                        for (cc_tokenizer::string_character_traits<char>::size_type r = 0; r < columns; r++)
                        {
                            std::cout<<innerMostArray[q*columns + r]<<" ";                        
                        }
                        std::cout<<std::endl;
                    }                                 
                }
            }

            return Collective<E>{ptr, {m.getShape().getNumberOfColumns(), m.getShape().getDimensionsOfArray().getNumberOfInnerArrays(), NULL, NULL}};
        }
                                                                
        /*            
            Function: zeros
    
            Description:
            This static template function is used to create and initialize a one-dimensional array of a specified data type (default is float) with zeros.
    
            Parameters:
            - shape: An object representing the dimensions of the desired array.
            - columnsOrRowMajor: An optional parameter specifying whether the array should be arranged in row-major or column-major order (default is row-major).
    
            Returns:
            - E*: A pointer to the dynamically allocated array of type E (float by default), initialized with zeros.
    
            Throws:
            - ala_exception: If the shape provided has a size of zero, indicating a malformed array request.
    
            Working:
            1. It first determines the size 'n' of the desired array using the 'shape' object.
            2. It allocates memory for the array using the custom allocator 'alloc_obj'.
            3. It initializes the allocated memory with zero values.
            4. Finally, it returns a pointer to the created array.
    
            Note:
            - This function is typically used to create arrays filled with zeros for numerical computations.
            - The 'columnsOrRowMajor' parameter allows specifying the memory layout of the array (row-major or column-major) for advanced use cases.
         */
        template <typename E = double>
        static Collective<E> zeros(DIMENSIONS shape, REPLIKA_ROW_MAJOR_OR_COLUMN_MAJOR columnsOrRowMajor = REPLIKA_ROW_MAJOR) throw (ala_exception)
        {
            cc_tokenizer::string_character_traits<char>::size_type n = shape.getN();
                        
            // n should not be zero
            if (n == 0)
            {
                throw ala_exception("Numcy::zeros(): Malformed shape of return array.");
            }
                        
            E* ptr = NULL;

            try 
            {
                ptr = cc_tokenizer::allocator<E>().allocate(n);
            }
            catch (std::bad_alloc& e)
            {
                throw ala_exception(cc_tokenizer::String<char>("Numcy::zeros() Error: ") + cc_tokenizer::String<char>(e.what()));
            }

            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < n; i++)
            {
                ptr[i] = 0x00;
            }
            
            return Collective<E>{ptr, shape.copy()};
        }

        template <typename E = float>
        static E* zeros_old(DIMENSIONS shape, REPLIKA_ROW_MAJOR_OR_COLUMN_MAJOR columnsOrRowMajor = REPLIKA_ROW_MAJOR) throw (ala_exception)
        {
            cc_tokenizer::string_character_traits<char>::size_type n = shape.getN();
            E* ptr = NULL;

            // n should not be zero
            if (n == 0)
            {
                throw ala_exception("Numcy::zeros(): Malformed shape of return array");
            }
            
            cc_tokenizer::allocator<char> alloc_obj;
            ptr = reinterpret_cast<E*>(alloc_obj.allocate(sizeof(E)*n));

            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < n; i++)
            {
                ptr[i] = 0x00;
            }

            return ptr;
        }        
};

#endif