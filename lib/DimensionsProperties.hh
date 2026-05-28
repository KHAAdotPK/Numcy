/*
 * Numcy/DimensionsProperties.hh     
 *
 * Q@hackers.pk
*/

/*
    To stop every translation unit that includes Dimensions.hh from always processing 
    DimensionsProperties.hh, even in circular or repeated inclusion scenarios.
    We use the #include guard. 
*/

/*
   From the point of view of templatd types, CUDA APIs expect specific types, size_t or int64_t.
 */

#ifndef NUMCY_DIMENSIONS_PROPERTIES_HH
#define NUMCY_DIMENSIONS_PROPERTIES_HH

/*
    Linked List of 2D Slices. Each node represents one 2D matrix within a higher-dimensional tensor
*/
template <typename T = size_t>
class DimensionsProperties 
{
        T columns; // Width of a 2D slice
        T rows; // Height of 2D slice, number of such slices
        DimensionsProperties<T>* next; // Next slice in tensor
        DimensionsProperties<T>* prev; // Previous slice in tensor
              
        /*
         * Reference counting tracks the number of active Dimensions objects sharing this node.
         * When a node is first created, it is owned by exactly one Dimensions object,
         * so the count starts at 1.
         *
         * The count is incremented when a new Dimensions object copies or takes shared
         * ownership of this node, and decremented when a Dimensions object releases it.
         * When the count reaches 0, no Dimensions object owns this node and it is safe to delete.
         *
         * The responsibility of managing the reference count lies with the Dimensions class
         * via incrementReferenceCount() and decrementReferenceCount().
         */        
        size_t reference_count;

    public:
        /*
         * Constructor for creating a new DimensionsProperties node.
         * Initializes the columns and rows, and sets up the linked list pointers.
         */
        DimensionsProperties(T c, T r) : columns(c), rows(r), next(nullptr), prev(nullptr), reference_count(1)
        {            
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

        T getColumns(void) const
        {
            return this->columns;
        }

        T getRows(void) const
        {
            return this->rows;
        }

        DimensionsProperties<T>* getNext(void) const
        {
            return this->next;
        }

        DimensionsProperties<T>* getPrevious(void) const
        {
            return this->prev;
        }

        size_t getReferenceCount(void) const
        {
            return this->reference_count;
        }

        void setColumns(T c)
        {
            this->columns = c;
        }

        void setRows(T r)
        {
            this->rows = r;
        }

        void setNext(DimensionsProperties<T>* n)
        {
            this->next = n;
        }

        void setPrev(DimensionsProperties<T>* p)
        {
            this->prev = p;
        }
};

#endif

/*
 * -------------------------------------------------------------------------
 * COMPILATION STANDARDS AND CODING CONVENTIONS
 * -------------------------------------------------------------------------
 *
 * This file is compiled with a strict set of warning flags that enforce
 * modern, safe, and portable C++ coding practices. The following notes
 * document the conventions that all code in this file must follow as a
 * direct consequence of those flags.
 *
 *
 * CASTS
 * -----
 * C-style casts are forbidden. The flag -Wold-style-cast treats them as
 * errors. C-style casts like (int)x or (size_t)y are dangerous because
 * they silently perform any of several different kinds of cast depending
 * on context, with no indication of intent and no compiler verification
 * that the cast is safe.
 *
 * The following cast forms must be used instead:
 *
 *     static_cast<T>(x)       — for safe, well-defined conversions between
 *                               related types, such as size_t to int or
 *                               double to float. The compiler verifies the
 *                               conversion is meaningful.
 *
 *     reinterpret_cast<T>(x)  — for low-level reinterpretation of bits,
 *                               such as casting a void* to a typed pointer.
 *                               Use only at FFI boundaries and memory
 *                               management code.
 *
 *     const_cast<T>(x)        — for removing const qualification. Use only
 *                               when interfacing with legacy APIs that are
 *                               not const-correct.
 *
 *     dynamic_cast<T>(x)      — for safe downcasting in a class hierarchy.
 *                               Returns nullptr if the cast is invalid.
 *
 *
 * FUNCTIONAL CASTS IN TEMPLATES
 * ------------------------------
 * Inside template code, the functional cast form T(x) is preferred over
 * static_cast<T>(x) when constructing a zero or default value of the
 * template type. For example:
 *
 *     T(0)     — constructs a zero value of type T, works for any T
 *                that is constructible from an integer literal.
 *
 * T(0) is not a C-style cast. It is a constructor call and is not flagged
 * by -Wold-style-cast. It is the idiomatic C++ way to express a typed
 * zero value inside a template, because it works correctly whether T is
 * a primitive type like int or size_t, or a user-defined numeric type.
 *
 *
 * TYPE CONVERSIONS
 * ----------------
 * Implicit conversions that may lose data are forbidden. The flags
 * -Wconversion and -Wsign-conversion treat them as errors. All narrowing
 * conversions (such as assigning a size_t to an int) and all signed to
 * unsigned conversions must be made explicit with static_cast.
 *
 *
 * WARNINGS AS ERRORS
 * ------------------
 * The flag -Werror promotes every compiler warning to a hard error.
 * Code that produces any warning will not compile. This is intentional.
 * Warnings in C++ are not suggestions — they indicate real problems that
 * will cause bugs, undefined behavior, or portability failures in
 * production. Every warning must be resolved before the code is committed.
 *
 *
 * RUNTIME SANITIZERS
 * ------------------
 * This code is compiled with -fsanitize=address,undefined during
 * development and testing. These sanitizers instrument the binary at
 * runtime to detect:
 *
 *     AddressSanitizer (ASan)
 *         — heap use-after-free
 *         — buffer overflows (heap and stack)
 *         — double-free errors
 *         — use of stack memory after the function returns
 *
 *     UndefinedBehaviorSanitizer (UBSan)
 *         — signed integer overflow
 *         — null pointer dereference
 *         — misaligned memory access
 *         — out-of-bounds array indexing
 *
 * Any code that compiles and runs cleanly under both sanitizers is
 * considered verified. A sanitizer error is treated with the same
 * urgency as a compilation error — it must be resolved immediately.
 *
 *
 * FULL COMPILATION COMMAND
 * ------------------------
 * g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wconversion
 *     -Wsign-conversion -Wshadow -Wnon-virtual-dtor -Wold-style-cast
 *     -Wcast-align -Wunused -Woverloaded-virtual -Wnull-dereference
 *     -Wdouble-promotion -Wformat=2 -Wmisleading-indentation
 *     -Wduplicated-cond -Wduplicated-branches -Wlogical-op
 *     -Wuseless-cast -Weffc++ -O2 -fsanitize=address,undefined
 *
 * -------------------------------------------------------------------------
 */