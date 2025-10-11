/*
    Numcy/Dimensions.hh
    Q@khaa.pk
 */

#include "header.hh"

#ifndef KHAA_PK_DIMENSIONS_HEADER_HH
#define KHAA_PK_DIMENSIONS_HEADER_HH

struct Dimensions;

// Linked List of 2D Slices. Each node represents one 2D matrix within a higher-dimensional tensor
typedef struct DimensionsProperties 
{
    private:
        cc_tokenizer::string_character_traits<char>::size_type columns; // Width of a 2D slice
        cc_tokenizer::string_character_traits<char>::size_type rows; // Height of 2D slice, number of such slices
        struct DimensionsProperties* next; // Next slice in tensor
        struct DimensionsProperties* prev; // Previous lice in tensor
    
        /*
         * Reference counting tracks the number of active references (pointers or other ways to access) to an object.
         * When an object is first created, there are no references to it, so the count should be 0.
         *  
         * The responsibility of managing the reference count lies with the class itself...
         * the incrementRefernceCount() and decrementReferenceCount() methods and the implementation of 
         * destructor method         
         */        
        cc_tokenizer::string_character_traits<char>::size_type reference_count; // Reference counting per slice

        friend struct Dimensions; // To allow Dimensions to access private members

    public:
        void incrementReferenceCount(void) 
        {
            this->reference_count++;
        }

        DimensionsProperties* getNext(void) 
        {
            return this->next;
        }

        DimensionsProperties* getPrevious(void)
        {
            return prev;
        }

}DIMENSIONSPROPERTIES;
typedef DIMENSIONSPROPERTIES* DIMENSIONSPROPERTIES_PTR;

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
    private:
        DimensionsProperties* properties;
    
    public:        
        /**
         * @brief Default constructor for Dimensions class.
         * 
         * Creates a single linked list node representing one 2D tensor slice with zero dimensions.
         * Allocates memory for the DimensionsProperties structure using the custom cc_tokenizer 
         * allocator and initializes all fields to default values.
         * 
         * Initialization:
         * - columns: 0 (no columns initially)
         * - rows: 0 (no rows initially) 
         * - next: NULL (no next slice in the linked list)
         * - prev: NULL (no previous slice in the linked list)
         * - reference_count: 1 (single reference to this slice)
         * 
         * This constructor creates the foundation for building multidimensional tensor 
         * representations where each node in the linked list represents a 2D slice of 
         * the overall tensor structure.
         * 
         * @throws ala_exception if memory allocation fails (std::bad_alloc) or if 
         *         length constraints are violated (std::length_error). Exception 
         *         messages include original error details for debugging.
         * 
         * @note Uses cc_tokenizer::allocator<char> for raw memory allocation with 
         *       reinterpret_cast to convert to the target type pointer.
         * @note The allocated memory must be properly deallocated in the destructor 
         *       when reference count reaches zero.
         * @note This creates an isolated node - use additional methods to build 
         *       the complete linked list structure for multidimensional tensors.
         */
        Dimensions (void) throw (ala_exception)
        {            
            try
            {
                this->properties = reinterpret_cast<DIMENSIONSPROPERTIES_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(DIMENSIONSPROPERTIES))); 
                
                this->properties->columns = 0;
                this->properties->rows = 0;            
                this->properties->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT;

                this->properties->next = NULL;
                this->properties->prev = NULL;
            }
            catch (std::bad_alloc& e)
            {
                // CRITICAL: Memory allocation failure - system should terminate immediately
                // NO cleanup performed - this is a fatal error requiring process exit
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::DIMENSIONS() Error: ") + cc_tokenizer::String<char>(e.what())); 
            }
            catch (std::length_error& e)
            {
                // CRITICAL: Length constraint violation - system should terminate immediately
                // NO cleanup performed - this is a fatal error requiring process exit
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::DIMENSIONS() Error: ") + cc_tokenizer::String<char>(e.what())); 
            }                 
        }

        /**
         * @brief Custom constructor for creating a Dimensions node with explicit linkage.
         *
         * This constructor provides fine-grained control over the creation of individual nodes
         * within the Dimensions linked list structure. It allows direct specification of all
         * node properties including dimensional values, linkage pointers, and reference count.
         * This is primarily intended for advanced use cases such as manual linked list 
         * construction, node insertion, or custom tensor building operations.
         *
         * Use Cases:
         * - Manual construction of complex tensor structures
         * - Insertion of nodes into existing linked lists
         * - Creation of nodes with specific reference counts for shared ownership
         * - Building custom dimensional arrangements not covered by other constructors
         * - Testing and debugging of linked list structures
         *
         * Parameters:
         * @param c Columns value for this node (typically meaningful only for final nodes)
         * @param r Rows value for this node (represents current dimension size)
         * @param n Next pointer - links to the subsequent node in the tensor structure
         * @param p Previous pointer - links to the preceding node in the tensor structure  
         * @param rc Reference count (defaults to 1) - tracks shared ownership of this node
         *
         * Memory Management:
         * - Allocates memory for a single DimensionsProperties node
         * - Uses cc_tokenizer::allocator<char> for consistent allocation pattern
         * - Caller responsible for maintaining proper linked list integrity
         * - No automatic validation of pointer relationships
         *
         * Linkage Responsibilities:
         * - This constructor does NOT automatically update neighboring nodes' pointers
         * - Caller must manually ensure bidirectional linkage consistency:
         *   - If n != NULL, caller should set n->prev = this->properties
         *   - If p != NULL, caller should set p->next = this->properties
         * - Failure to maintain proper linkage will corrupt the linked list structure
         *
         * Reference Count Considerations:
         * - Default reference count of 1 assumes this node will have one owner
         * - Custom reference counts enable advanced sharing scenarios
         * - Reference count of 0 may cause immediate deallocation in some operations
         *
         * Exception Handling:
         * - Follows the class's fatal error philosophy
         * - No cleanup performed on critical exceptions (bad_alloc, length_error)
         * - System should terminate immediately on allocation failures
         * - Exception messages include full method signature for debugging
         *
         * @throws ala_exception Wraps std::bad_alloc or std::length_error with detailed context
         *
         * @warning This is a low-level constructor requiring careful manual linkage management
         * @warning Improper linkage will corrupt the linked list and cause undefined behavior
         * @warning Not thread-safe - external synchronization required
         * @note Prefer other constructors unless you need explicit control over node linkage
         * @note Always ensure bidirectional linkage consistency after construction
         *
         * Example Usage:
         * @code
         * // Create a middle node with proper linkage
         * DIMENSIONSPROPERTIES_PTR prev_node = // ... existing node
         * DIMENSIONSPROPERTIES_PTR next_node = // ... existing node
         * 
         * Dimensions middle_dim(0, 5, next_node, prev_node, 1);
         * 
         * // CRITICAL: Update neighboring nodes for bidirectional linkage
         * prev_node->next = middle_dim.properties;
         * next_node->prev = middle_dim.properties;
         * @endcode
         */
        Dimensions(cc_tokenizer::string_character_traits<char>::size_type c, cc_tokenizer::string_character_traits<char>::size_type r, DIMENSIONSPROPERTIES_PTR n, DIMENSIONSPROPERTIES_PTR p, cc_tokenizer::string_character_traits<char>::size_type rc = NUMCY_DEFAULT_REFERENCE_COUNT)
        {
            try
            {
                this->properties = reinterpret_cast<DIMENSIONSPROPERTIES_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(DIMENSIONSPROPERTIES))); 
                                
                this->properties->columns = c;
                this->properties->rows = r;            
                this->properties->reference_count = rc;

                this->properties->next = n;
                this->properties->prev = p;
            }
            catch (std::bad_alloc& e)
            {
                // CRITICAL: Memory allocation failure - system should terminate immediately
                // NO cleanup performed - this is a fatal error requiring process exit
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::DIMENSIONS(cc_tokenizer::string_character_traits<char>::size_type, cc_tokenizer::string_character_traits<char>::size_type, DIMENSIONSPROPERTIES_PTR, DIMENSIONSPROPERTIES_PTR, cc_tokenizer::string_character_traits<char>::size_type) Error: ") + cc_tokenizer::String<char>(e.what())); 
            }
            catch (std::length_error& e)
            {
                // CRITICAL: Length constraint violation - system should terminate immediately
                // NO cleanup performed - this is a fatal error requiring process exit
                throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::DIMENSIONS(cc_tokenizer::string_character_traits<char>::size_type, cc_tokenizer::string_character_traits<char>::size_type, DIMENSIONSPROPERTIES_PTR, DIMENSIONSPROPERTIES_PTR, cc_tokenizer::string_character_traits<char>::size_type) Error: ") + cc_tokenizer::String<char>(e.what())); 
            }                        
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
         * Exception Handling:
         * - In case of critical exceptions (bad_alloc, length_error), NO cleanup is performed.
         * - These are considered fatal system errors that require immediate termination.
         * - The calling system should catch these exceptions and perform emergency shutdown.
         * - Memory leaks are acceptable in these scenarios as the process should exit immediately.
         *
         * @param dimensionsOfArray A container (DIMENSIONSOFARRAY) holding integer-like
         *        values where:
         *        - The first element defines the `rows` of the root object.
         *        - Intermediate elements define the `rows` of linked nodes.
         *        - The last element defines the `columns` of the final node.
         *
         * @throws ala_exception Wraps std::bad_alloc, std::length_error, or propagates existing ala_exceptions
         * @warning Fatal exceptions require system termination - no recovery possible
         */
        Dimensions (DIMENSIONSOFARRAY& dimensionsOfArray)
        {        
            if (dimensionsOfArray.size() > 1) // dimensionsOfArray.size() is atleast 2
            {
                cc_tokenizer::string_character_traits<char>::size_type i = 0;            
                DimensionsProperties* current = NULL;

                try 
                {
                    // Allocate memory for the root properties node
                    this->properties = reinterpret_cast<DIMENSIONSPROPERTIES_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(DIMENSIONSPROPERTIES)));

                    // Iterate through all dimensions except the last one
                    while (i < (dimensionsOfArray.size() - 1)) 
                    {
                        if (i == 0) // Initialize the root node (first dimension)
                        {
                            // Set the first dimension as rows for the root node
                            this->properties->rows = dimensionsOfArray[0]; 
                            this->properties->columns = 0; // Will be set later and that is if the node is the last node as well
                        
                            // Initialize linkage pointers for root node
                            this->properties->next = NULL;
                            this->properties->prev = NULL;

                            // Initialize reference count
                            this->properties->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT;

                            // Set current pointer to root for subsequent operations
                            current = this->properties; // To deal with the edge case of a single link that is when we are dealing with just a 2D array
                        }
                        else
                        {
                            if (this->properties->next == NULL) // Creating the second node 
                            {
                                // Allocate memory for the second node
                                this->properties->next = reinterpret_cast<DIMENSIONSPROPERTIES_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(DIMENSIONSPROPERTIES)));

                                // Move current pointer to the newly allocated node
                                current = this->properties->next;

                                // Initialize the second node
                                current->next = NULL;
                                current->prev = this->properties; // Link back to root
                                current->rows = dimensionsOfArray[1]; // Set dimension value and that "1" makes it a second node
                                current->columns = 0; // Will be set later and that is if the node is the last node
                                current->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT;                       
                            }
                            else // Creating subsequent nodes (third, fourth, etc.) 
                            {
                                // Allocate memory for the next node in the chain
                                current->next = reinterpret_cast<DIMENSIONSPROPERTIES_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(DIMENSIONSPROPERTIES)));

                                // Establish bidirectional linkage
                                current->next->prev = current;
                                current = current->next;

                                // Initialize the new node
                                current->next = NULL;
                                current->rows = dimensionsOfArray[i]; // Set dimension value
                                current->columns = 0; // Will be set later for the last node
                                current->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT;                        
                            }
                        }

                        i = i + 1; // Move to next dimension
                    }
                                
                    // Set the columns value for the final node (last dimension)
                    // This can never be NULL even in edge case of single link (2D array)
                    current->columns = dimensionsOfArray[dimensionsOfArray.size() - 1];
                }
                catch (std::bad_alloc& e)
                {
                    // CRITICAL: Memory allocation failure - system should terminate immediately
                    // NO cleanup performed - this is a fatal error requiring process exit
                    throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::DIMENSIONS(DIMENSIONSOFARRAY&) Error: ") + cc_tokenizer::String<char>(e.what())); 
                }
                catch (std::length_error& e)
                {
                    // CRITICAL: Length constraint violation - system should terminate immediately
                    // NO cleanup performed - this is a fatal error requiring process exit
                    throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::DIMENSIONS(DIMENSIONSOFARRAY&) Error: ") + cc_tokenizer::String<char>(e.what())); 
                }
                catch (ala_exception& e)
                {
                    // Propagate existing ala_exception with additional context
                    // NO cleanup performed assuming this is also a critical error
                    throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::DIMENSIONS(DIMENSIONSOFARRAY&) -> ") + cc_tokenizer::String<char>(e.what())); 
                }                                            
            }
            else
            {
                // Handle case where dimensionsOfArray.size() <= 1
                // This is an invalid state, multi dimensional arrays require at least 2 dimensions
                throw ala_exception("DIMENIONS::DIMENSIONS(DIMENSIONSOFARRAY&) Error: Empty or single dimension array provided. At least 2 dimensions (rows and columns) required for multi-dimensional array construction.");
            }       
        }
    
    /**
     * @brief Copy constructor that creates a deep copy of another Dimensions object.
     *
     * This constructor creates a completely independent copy of the provided Dimensions
     * object by utilizing the copy() method to duplicate the entire linked list structure.
     * The new instance will have its own separate memory allocation and can be modified
     * independently without affecting the original object.
     *
     * Behavior:
     * - Delegates the actual copying work to the copy() method of the source object
     * - Creates a new independent linked list with identical dimensional structure
     * - Each node in the copy has its own memory allocation and reference count
     * - The resulting object is completely separate from the original
     *
     * Memory Management:
     * - Uses the existing copy() method which handles all memory allocation
     * - No direct memory management required in this constructor
     * - The copy() method ensures proper linked list structure and reference counts
     *
     * Exception Handling:
     * - Propagates any ala_exception thrown by the copy() method
     * - Critical exceptions (bad_alloc, length_error) from copy() are fatal
     * - No cleanup performed on failure - system should terminate on critical errors
     *
     * @param ref The source Dimensions object to copy from
     *
     * @throws ala_exception Propagates exceptions from copy() method with additional context
     * 
     * @warning Fatal exceptions from copy() require system termination
     * @note Creates a completely independent copy - changes to either object won't affect the other
     */
    Dimensions(const Dimensions& ref)
    {                          
        try 
        {
            this->properties = ref.copy();
        }
        catch (ala_exception& e)
        {
            // Propagate existing ala_exception with additional context
            // NO cleanup performed assuming this is also a critical error
            throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::DIMENSIONS(DIMENSIONS&) -> ") + cc_tokenizer::String<char>(e.what()));
        }                          
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
    /*~Dimensions()
    {
        DIMENSIONSPROPERTIES_PTR current = this->properties;

        while (current != NULL)
        {
            if (current->reference_count == 0)
            {
                if (current->prev != NULL)
                {                    
                    current->prev->next = current->next;

                    if (current->next != NULL)
                    {
                        current->next->prev = current->prev;
                    }

                     DIMENSIONSPROPERTIES_PTR temp = current->prev->next;

                    cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(current), sizeof(DIMENSIONSPROPERTIES));

                    current = temp;;                    
                }
                else // current->prev is NULL 
                {
                    if (current->next != NULL)
                    {
                        DIMENSIONSPROPERTIES_PTR temp = current->next;

                        cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(current), sizeof(DIMENSIONSPROPERTIES));

                        this->properties = temp;

                        this->properties->prev = NULL;

                        current = temp;
                    }
                    else 
                    {
                        cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(current), sizeof(DIMENSIONSPROPERTIES));

                        current = NULL;

                        this->properties = NULL;
                    }
                }
            }
            else 
            {
                current = current->next;
            }        
        }     
    }*/

    /**
     * @brief Destructor for the Dimensions class.
     *
     * This destructor handles cleanup by delegating to the existing reference counting
     * system. It decrements the reference count for all nodes in the linked list, and
     * the decrementReferenceCount() method automatically deallocates any nodes that
     * reach a reference count of zero.
     *
     * Behavior:
     * - Calls decrementReferenceCount() to decrement all node reference counts
     * - Nodes with reference_count reaching 0 are automatically deallocated
     * - Nodes still referenced by other objects remain alive
     * - Maintains proper linked list structure during cleanup
     * - Results in clean, predictable memory management
     *
     * Advantages of this approach:
     * - Reuses existing, tested reference counting logic
     * - No code duplication - follows DRY principle
     * - Consistent behavior with manual decrementReferenceCount() calls
     * - Simple, clear, and maintainable
     * - Automatic handling of all edge cases (head removal, complete cleanup, etc.)
     *
     * Memory Management:
     * - Properly handles shared ownership scenarios
     * - Only deallocates nodes when safe to do so (reference_count == 0)
     * - Maintains system integrity even with complex object relationships
     * - No risk of double-deallocation or dangling pointers
     *
     * @note This destructor relies on the correctness of decrementReferenceCount()
     * @note Objects sharing nodes will keep them alive until all references are gone
     * @warning Not thread-safe - external synchronization required for multi-threaded usage
     */
     ~Dimensions() 
     {
         decrementReferenceCount(); // Decrement first, then cleanup happens automatically
     }

    /**
     * @brief Increments the reference count for all nodes in the linked list structure.
     *
     * This method traverses the entire linked list of Dimensions properties starting
     * from the current object and increments the reference count of each node by 1.
     * This is typically used when creating a new reference to the shared dimensional
     * structure to track how many objects are using the same linked list.
     *
     * Behavior:
     * - Starts from the current object's properties node
     * - Traverses forward through the entire linked list via 'next' pointers
     * - Increments reference_count by 1 for each node encountered
     * - Stops when reaching the end of the list (next == NULL)
     *
     * Thread Safety:
     * - This method is NOT thread-safe
     * - Concurrent calls may result in race conditions on reference_count values
     * - External synchronization required for multi-threaded usage
     *
     * Usage Context:
     * - Called when a new object begins sharing this dimensional structure
     * - Should be paired with corresponding decrementReferenceCount() calls
     * - Part of manual reference counting system for shared ownership
     *
     * @note This method assumes the linked list structure is valid and not corrupted
     * @note No bounds checking or overflow protection on reference_count values
     * @warning Not thread-safe - external synchronization required
     */
    void incrementReferenceCount(void) 
    {        
        DIMENSIONSPROPERTIES_PTR current = this->properties;

        // Traverse the entire linked list forward
        while(current != NULL)
        {
            // Increment reference count for current node
            current->reference_count++;

            // Move to next node in the linked list
            current = current->next;            
        }        
    }

    /**
     * @brief Decrements reference count for all nodes and deallocates nodes with zero references.
     *
     * This method traverses the entire linked list of Dimensions properties and decrements
     * the reference count of each node. When a node's reference count reaches zero, it is
     * immediately removed from the linked list and its memory is deallocated. The method
     * handles the complex task of maintaining linked list integrity while removing nodes.
     *
     * Behavior:
     * - Traverses the linked list starting from the current object's properties
     * - Decrements reference_count for each node (with underflow protection)
     * - Immediately deallocates nodes when reference_count reaches 0
     * - Updates linked list pointers to maintain structural integrity
     * - Handles special cases for head node removal and complete list deletion
     *
     * Memory Management:
     * - Uses cc_tokenizer::allocator<char>().deallocate() for memory cleanup
     * - Maintains doubly-linked list structure during node removal
     * - Updates this->properties when head node is removed
     * - Sets this->properties to NULL when entire list is deallocated
     *
     * Edge Cases Handled:
     * - Removing head node (prev == NULL): Updates this->properties
     * - Removing tail node (next == NULL): Updates previous node's next pointer
     * - Removing middle nodes: Updates both prev->next and next->prev pointers
     * - Complete list deletion: Sets this->properties to NULL
     *
     * Thread Safety:
     * - This method is NOT thread-safe
     * - Concurrent access during deallocation can cause memory corruption
     * - External synchronization required for multi-threaded usage
     *
     * @note This method assumes the linked list structure is valid and not corrupted
    * @note Reference counts are protected against underflow (won't go below 0)
    * @warning Not thread-safe - external synchronization required
    * @warning After calling this method, the object may be in an empty state (properties == NULL)
    */
    void decrementReferenceCount(void)
    {
        DIMENSIONSPROPERTIES_PTR current = this->properties;

        // Traverse the entire linked list
        while (current != NULL)
        {
            // Decrement reference count with underflow protection
            if (current->reference_count > 0)
            {
                current->reference_count--;
            }

            // If reference count reaches zero, deallocate the node
            if (current->reference_count == 0)
            {                
                if (current->prev != NULL) // Removing a middle or tail node
                {   
                    // Update previous node's next pointer to skip current node                 
                    current->prev->next = current->next;

                    // If current node has a next node, update its prev pointer
                    if (current->next != NULL)
                    {
                        current->next->prev = current->prev;
                    }

                    // Store the next node for continued traversal
                    DIMENSIONSPROPERTIES_PTR temp = current->prev->next;

                    // Deallocate current node's memory
                    cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(current), sizeof(DIMENSIONSPROPERTIES));

                    // Continue with next node
                    current = temp;;                    
                }
                else // current->prev is NULL, removing head node 
                {
                    if (current->next != NULL) // Head removal with remaining nodes
                    {
                        // Store pointer to new head node
                        DIMENSIONSPROPERTIES_PTR temp = current->next;

                        // Deallocate current head node
                        cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(current), sizeof(DIMENSIONSPROPERTIES));

                        // Update this object to point to new head
                        this->properties = temp;
                        this->properties->prev = NULL; // New head has no previous node

                        // Continue traversal from new head
                        current = temp;
                    }
                    else // Removing the only remaining node (complete list deletion)
                    {
                        // Deallocate the final node
                        cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(current), sizeof(DIMENSIONSPROPERTIES));

                        // Object now has no properties (empty state)
                        current = NULL;

                        this->properties = NULL;
                    }
                }
            }
            else // Reference count > 0, node stays alive 
            {
                // Move to next node without modification
                current = current->next;
            }           
        }
    }

    /**
     * @brief Creates a deep copy of the entire Dimensions linked list structure.
     *
     * This method traverses the complete linked list of Dimensions objects starting
     * from the current instance and creates an identical copy with separate memory
     * allocation for each node. The copy maintains the same dimensional structure
     * and values as the original but is completely independent.
     *
     * Process:
     * 1. Extracts dimensions array from the current linked list structure
     * 2. Allocates new memory for each node in the copy
     * 3. Reconstructs the doubly-linked list with identical values
     * 4. Sets reference counts to 1 for each new node
     * 5. Assigns the final dimension value to the last node's columns
     *
     * Memory Management:
     * - Uses `cc_tokenizer::allocator<char>` for consistent allocation
     * - Each node is independently allocated with its own reference count
     * - Creates a completely separate linked list structure
     *
     * Exception Handling:
     * - In case of critical exceptions (bad_alloc, length_error), NO cleanup is performed
     * - These are considered fatal system errors requiring immediate termination
     * - Partial allocations are left as-is since the system should exit immediately
     * - Memory leaks are acceptable when the process is about to terminate
     *
     * @return DIMENSIONSPROPERTIES_PTR Pointer to the root node of the newly created
     *         copy, or NULL if the original structure is empty or invalid
     *
     * @throws ala_exception Wraps std::bad_alloc, std::length_error, or propagates
     *         existing ala_exceptions from getDimensionsOfArray()
     * 
     * @warning Fatal exceptions require system termination - no recovery possible
     * @note The returned copy is completely independent and should be managed separately
     */
    DIMENSIONSPROPERTIES_PTR copy(void) const throw (ala_exception)
    {
        DIMENSIONSPROPERTIES_PTR ret = NULL, current = NULL;  
        
        // Extract dimensions array from current linked list structure
        // This may throw ala_exception if the structure is corrupted
        DIMENSIONSOFARRAY dimensionsOfArray = this->getDimensionsOfArray();

        // Validate that we have sufficient dimensions for processing
        if (dimensionsOfArray.size() < 2)
        {
            throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::copy() Error: Invalid dimensions array requires at least 2 dimensions"));
        }

        try 
        {
            // Iterate through all dimensions except the last one (which becomes columns)
            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < (dimensionsOfArray.size() - 1); i++)
            {
                if (ret == NULL)
                {
                    // Allocate memory for the root node
                    ret = reinterpret_cast<DIMENSIONSPROPERTIES_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(DIMENSIONSPROPERTIES)));

                    // Initialize root node properties
                    ret->columns = 0; // Will be set later for the final node
                    ret->rows = dimensionsOfArray[i]; // Set current dimension values
                    ret->next = NULL; // No next node yet
                    ret->prev = NULL; // Root has no previous node
                    ret->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT; // Initialize reference count
                
                    // Set current pointer for subsequent operations
                    current = ret;                                    
                }
                else // Creating subsequent nodes in the linked list
                {
                    // Allocate memory for the next node
                    current->next = reinterpret_cast<DIMENSIONSPROPERTIES_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(DIMENSIONSPROPERTIES)));

                    // Initialize the new node properties
                    current->next->columns = 0; // Will be set later for the final node
                    current->next->rows = dimensionsOfArray[i]; // Set current dimension value
                    current->next->next = NULL; // No further nodes yet
                    current->next->prev = current; // Link back to previous node
                    current->next->reference_count = NUMCY_DEFAULT_REFERENCE_COUNT; // Initialize reference count
                
                    // Move current pointer to the newly created node
                    current = current->next;                    
                }
            }

            // Set the columns value for the final node (last dimension)
            if (current != NULL)
            {
                current->columns = dimensionsOfArray[dimensionsOfArray.size() - 1];
            }
        }
        catch (std::length_error& e)
        {
            // CRITICAL: Length constraint violation - system should terminate immediately
            // NO cleanup performed - this is a fatal error requiring process exit
            throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::copy() Error: ") + cc_tokenizer::String<char>(e.what())); 
        }
        catch (std::bad_alloc& e)
        {
            // CRITICAL: Memory allocation failure - system should terminate immediately
            // NO cleanup performed - this is a fatal error requiring process exit
            throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::copy() Error: ") + cc_tokenizer::String<char>(e.what())); 
        }
        catch (ala_exception& e)
        {
            // Propagate existing ala_exception with additional context
            // NO cleanup performed assuming this is also a critical error
            throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::copy(E*, DIMENSIONS) -> ") + cc_tokenizer::String<char>(e.what()));
        }
        
        // Return pointer to the root of the newly created copy
        return ret;
    }

    /**
     * @brief Calculates the total number of elements in the multi-dimensional array.
     *
     * This method computes the total capacity of the multi-dimensional array by
     * multiplying all dimensions together, including both the rows of each dimension
     * and the columns of the final dimension. This gives the total number of elements
     * that can be stored in the complete array structure.
     *
     * Calculation Logic:
     * - Multiplies the 'rows' value of each node in the linked list
     * - For the final node, also multiplies by its 'columns' value
     * - Result = d1 × d2 × d3 × ... × dn-1 × columns
     *
     * Examples:
     * - 2D array [10, 5]: returns 10 × 5 = 50 total elements
     * - 3D array [4, 3, 7]: returns 4 × 3 × 7 = 84 total elements
     * - 4D array [2, 5, 3, 8]: returns 2 × 5 × 3 × 8 = 240 total elements
     *
     * Performance:
     * - Time complexity: O(n) where n is the number of linked nodes
     * - Space complexity: O(1) - only uses local variables
     * - Single traversal through the linked list
     *
     * Thread Safety:
     * - Safe for concurrent reads (const method)
     * - No modification of shared state during calculation
     * - Does not require external synchronization for read-only access
     *
     * @return Total number of elements in the multi-dimensional array structure
     *         Returns 0 if the structure is empty (properties == NULL)
     *
     * @note This method is const and thread-safe for concurrent reads
     * @note Useful for determining memory requirements or iteration bounds
    * @warning Potential integer overflow for very large dimensional products
    */
    cc_tokenizer::string_character_traits<char>::size_type getN(void) const
    {        
         DIMENSIONSPROPERTIES_PTR current = this->properties;

         // Handle empty structure
         if (current == NULL)
         {
             return 0;
         }

         // Initialize result with first node's rows value
         cc_tokenizer::string_character_traits<char>::size_type n = current->rows;
         cc_tokenizer::string_character_traits<char>::size_type columns = current->columns;
    
         // Move to next node
         current = current->next;
    
         // Multiply by remaining nodes' rows values
         while (current != NULL)
         {
             n = n * current->rows;
             columns = current->columns;  // Keep track of final node's columns
             current = current->next;            
         }

         // Multiply by the final columns value
         n = n * columns;

         return n;
     }

    /**
     * @brief Returns the number of columns in the innermost dimension of the array.
     *
     * This method traverses to the final node in the linked list and returns its
     * 'columns' value, which represents the size of the innermost array dimension.
     * In a multi-dimensional array structure, this is always the last dimension.
     *
     * Design Philosophy:
     * - All arrays are considered at least two-dimensional (rows × columns)
     * - Even a simple array[10] is treated as 1 row × 10 columns
     * - The columns value is only meaningful in the final node of the linked list
     *
     * Behavior:
     * - Traverses to the last node in the linked list (where next == NULL)
     * - Returns the 'columns' value from that final node
     * - Only the final node's columns value is meaningful (others are typically 0)
     *
     * Examples:
     * - 2D array with final node {rows=5, columns=10}: returns 10
     * - 3D array with final node {rows=3, columns=7}: returns 7
     * - Single array[15] represented as {rows=1, columns=15}: returns 15
     *
     * Preconditions:
     * - The linked list must be properly formed and non-empty
     * - At least one node must exist (properties != NULL)
     * - The linked list must have a proper termination (final node with next == NULL)
     *
     * @return Number of columns in the innermost array dimension
     *         Returns 0 if the structure is empty or malformed
     *
     * @note Caller must ensure the linked list is properly formed before calling
     * @note Only the final node's columns value is returned - intermediate nodes are ignored
     * @warning Returns 0 for empty structures - caller should validate before use
     */
     cc_tokenizer::string_character_traits<char>::size_type getNumberOfColumns(void) const
     {
         DIMENSIONSPROPERTIES_PTR current = this->properties;

         // Handle empty structure
         if (current == NULL)
         {
            return 0;
         }

         // Traverse to the final node in the linked list
         while (current->next != NULL)
         {
             current = current->next;
         }

         // Return the columns value from the final node
         return current->columns;
     }
     /*
        Alternative Name Consideration:
        - getInnerMostArraySize()
        - getColumnCount()
        - getFinalDimensionSize()
      */    
   
    /**
     * @brief Calculates the total number of rows in the multi-dimensional array structure.
     *
     * This method computes the total number of "rows" by multiplying all dimension sizes
    * except the last one (which represents columns). For a multi-dimensional array,
     * this gives the total number of row-like structures across all dimensions.
     *
     * Calculation Logic:
     * - Excludes the final dimension (columns) from the calculation
     * - Multiplies all other dimensions together
     * - For array [d1, d2, d3, ..., dn-1, columns]: returns d1 × d2 × d3 × ... × dn-1
     *
     * Examples:
     * - 2D array [10, 5]: returns 10 (10 rows, 5 columns each)
     * - 3D array [4, 3, 7]: returns 4 × 3 = 12 (12 row-structures, 7 columns each)
     * - 4D array [2, 5, 3, 8]: returns 2 × 5 × 3 = 30 (30 row-structures, 8 columns each)
     *
     * Memory Management:
     * - DIMENSIONSOFARRAY handles its own memory cleanup via destructor
     * - No manual memory management required in this method
     * - dimArray will automatically deallocate when it goes out of scope
     *
     * Error Handling:
     * - Propagates exceptions from getDimensionsOfArray() with additional context
     * - DIMENSIONSOFARRAY destructor handles cleanup even in exception scenarios
     *
     * @return Total number of rows across all dimensions (product of all dimensions except columns)
     *
     * @throws ala_exception Propagates exceptions from getDimensionsOfArray()
     * 
     * @note For a 1D array, this would return 1 (conceptually 1 row)
     * @note DIMENSIONSOFARRAY destructor automatically handles memory cleanup
     * @warning Potential integer overflow for very large dimensional products
     */
    cc_tokenizer::string_character_traits<char>::size_type getNumberOfRows(void) throw (ala_exception)
    {
        try
        {
            cc_tokenizer::string_character_traits<char>::size_type ret = 1;

            // Get the dimensions array (this allocates memory)
            DIMENSIONSOFARRAY dimArray = getDimensionsOfArray();

            // Multiply all dimensions except the last one (which is columns)
            // dimArray.size() - 1 excludes the final dimension (columns)
            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < dimArray.size() - 1; i++)
            {
                ret = ret * dimArray[i];
            }

            return ret;

            // dimArray destructor called here for automatic cleanup
        }
        catch (ala_exception& e)
        {
            // Propagate existing ala_exception with additional context
            // NO cleanup performed assuming this is also a critical error
            throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::getNumberOfRows() -> ") + cc_tokenizer::String<char>(e.what()));
        }           
    }

    cc_tokenizer::string_character_traits<char>::size_type* getReferenceCounts(void) const throw (ala_exception)
    {
        cc_tokenizer::string_character_traits<char>::size_type* ptr_refCounts = NULL; 

        try
        {
            ptr_refCounts = cc_tokenizer::allocator<cc_tokenizer::string_character_traits<char>::size_type>().allocate(this->getNumberOfLinks());
            
            cc_tokenizer::string_character_traits<char>::size_type i = 0;
            DIMENSIONSPROPERTIES_PTR current = this->properties;

            while (1)
            {
                if (current == NULL)
                {
                    break;
                }
                
                ptr_refCounts[i] = current->reference_count;

                current = current->next;
                i++;
            }
        }
        catch (std::length_error& e)
        {
            // CRITICAL: Length constraint violation - system should terminate immediately
            // NO cleanup performed - this is a fatal error requiring process exit
            throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::getReferenceCount() Error: ") + cc_tokenizer::String<char>(e.what()));                
        }
        catch (std::bad_alloc& e)
        {
            // CRITICAL: Memory allocation failure - system should terminate immediately
            // NO cleanup performed - this is a fatal error requiring process exit
            throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::getReferenceCount() Error: ") + cc_tokenizer::String<char>(e.what()));                
        }
        catch (ala_exception& e)
        {
            // Propagate existing ala_exception with additional context
            // NO cleanup performed assuming this is also a critical error
            throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::getReferenceCount() -> ") + cc_tokenizer::String<char>(e.what()));
        }
        
        return ptr_refCounts;                
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
    /*DIMENSIONSOFARRAY getDimensionsOfArray(void) const throw (ala_exception)
    {
        cc_tokenizer::string_character_traits<char>::size_type n = getNumberOfLinks();

        
        // The last link of linked list has number of columns of the inner most array, that is why this method would return atleast 2 as the dimensions of the array and it implies having both rows and columns in the last link of either multilink or single link list.
        
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
        DIMENSIONSPROPERTIES_PTR current = this->properties;

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
    }*/

    /**
     * @brief Extracts dimensional information from the linked list into a flat array structure.
     *
     * This method traverses the entire linked list of DIMENSIONSPROPERTIES and creates
     * a DIMENSIONSOFARRAY containing all dimension sizes in sequential order. The result
     * represents the complete shape of a multi-dimensional array.
     *
     * Structure Conversion:
     * - Each node's 'rows' value becomes one dimension in the output array
     * - The final node's 'columns' value becomes the last dimension (innermost array size)
     * - Total dimensions = number of links + 1 (to include columns)
     *
     * Example (from comment):
     * Linked list: dim(rows=9) -> dim(rows=78) -> dim(rows=10) -> dim(rows=3, cols=10)
     * Result array: [9, 78, 10, 3, 10]
     * Meaning: 4D array with dimensions 9×78×10×3×10
     *
     * Memory Management:
     * - Allocates memory for the output array using cc_tokenizer::allocator
     * - Caller is responsible for deallocating the returned array
     * - Uses reinterpret_cast for consistent allocation pattern
     *
     * Error Handling:
     * - Validates that the linked list structure is not empty/corrupted
     * - Throws ala_exception for invalid states (0 links indicates corruption)
     * - No cleanup on allocation failure (follows fatal error pattern)
     *
     * @return DIMENSIONSOFARRAY A structure containing:
     *         - Pointer to allocated array of dimension sizes
     *         - Count of total dimensions (link count + 1)
     *
     * @throws ala_exception If link count is 0 (indicating corrupted structure)
     *                      If memory allocation fails (fatal error)
     * 
     * @note Returned array must be deallocated by caller
     * @note Minimum return size is 2 (at least rows and columns)
     * @warning Memory allocation failure is considered fatal - no cleanup performed
     */
     DIMENSIONSOFARRAY getDimensionsOfArray(void) const throw (ala_exception)
     {
        try
        {
             // Get the number of links in the chain
             cc_tokenizer::string_character_traits<char>::size_type n = getNumberOfLinks();
             
             /*
             * The last link contains the columns value for the innermost array.
             * Total dimensions = number of links + 1 (to include final columns value)
             * This ensures we always return at least 2 dimensions (rows + columns)
             */
             if (n != 0)
             {
                 n = n + 1;  // Add 1 for the final columns dimension
             }
             else
             {
                 // Zero links indicates a corrupted or uninitialized structure
                 throw ala_exception(cc_tokenizer::String<char>("Dimensions::getDimensionsOfArray() Error: Malformed shape of array, link count is 0"));
             }

             // Allocate memory for the dimensions array
             cc_tokenizer::string_character_traits<char>::size_type *ptr = reinterpret_cast<cc_tokenizer::string_character_traits<char>::size_type*>(cc_tokenizer::allocator<char>().allocate(n * sizeof(cc_tokenizer::string_character_traits<char>::size_type)));

             // Traverse the linked list and populate the dimensions array
             cc_tokenizer::string_character_traits<char>::size_type i = 0;
             DIMENSIONSPROPERTIES_PTR current = this->properties;

             // Extract dimensions from each node
             while (true)  // Using 'true' instead of '1' for clarity
             {
                 // Store the rows value as the current dimension
                 ptr[i] = current->rows;

                 // Check if this is the final node
                 if (current->next == NULL)
                 {
                     // Store the columns value as the final dimension
                     ptr[i + 1] = current->columns;
                     break;  // Exit the loop
                 }
            
                 // Move to the next node
                 current = current->next;
                 i = i + 1;
             }

             // Return the populated dimensions array structure
             return {ptr, n};
         }
         catch (std::bad_alloc& e)
         {    
             // CRITICAL: Memory allocation failure - system should terminate immediately
             // NO cleanup performed - this is a fatal error requiring process exit
             throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::getDimensionsOfArray() Error: ") + cc_tokenizer::String<char>(e.what()));
         }
        catch (std::length_error& e)
        {
            // CRITICAL: Length constraint violation - system should terminate immediately
            // NO cleanup performed - this is a fatal error requiring process exit
            throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::getDimensionsOfArray() Error: ") + cc_tokenizer::String<char>(e.what())); 
        }
         catch (ala_exception& e)
         {
            // Propagate existing ala_exception with additional context
             throw ala_exception(cc_tokenizer::String<char>("DIMENSIONS::getDimensionsOfArray() -> ") + cc_tokenizer::String<char>(e.what()));
         }
     }

    /*        
        Returns number of links in a linked list and this number would atleast be 1 because each linked list has a head where the previous pointer is NULL.
     */
    /**
     * @brief Counts the number of nodes in the linked list structure.
     *
     * This method traverses the entire linked list of DIMENSIONSPROPERTIES starting
     * from the current object's properties and counts the total number of nodes.
     * This count represents the number of dimensional links in the structure.
     *
     * Behavior:
     * - Starts from this->properties (the head of the linked list)
     * - Traverses forward through all nodes via 'next' pointers
     * - Increments counter for each node encountered
     * - Stops when reaching the end (next == NULL)
     * - Returns 0 if properties is NULL (empty/uninitialized structure)
     *
     * Relationship to Dimensions:
     * - Total dimensions = getNumberOfLinks() + 1
     * - The +1 accounts for the final 'columns' value in the last node
     * - Each link represents one dimension, plus columns makes the complete shape
     *
     * Examples:
     * - Single node: returns 1 → total dimensions = 2 (rows + columns)
     * - Two nodes: returns 2 → total dimensions = 3 (3D array)
     * - Three nodes: returns 3 → total dimensions = 4 (4D array)
     *
     * Performance:
     * - Time complexity: O(n) where n is the number of linked nodes
     * - Space complexity: O(1) - only uses a counter variable
     * - No memory allocation or complex operations
     *
     * Thread Safety:
     * - Safe for concurrent reads (const method)
     * - No modification of shared state during traversal
     * - Does not require external synchronization for read-only access
     *
     * @return Number of nodes in the linked list (0 if empty structure)
     *
     * @note This method is const and thread-safe for concurrent reads
     * @note Return value of 0 indicates an empty/uninitialized Dimensions object
     * @see getDimensionsOfArray() which uses this count to determine total dimensions
     */
     cc_tokenizer::string_character_traits<char>::size_type getNumberOfLinks(void) const
     {
         cc_tokenizer::string_character_traits<char>::size_type n = 0;
         DIMENSIONSPROPERTIES_PTR current = this->properties;

         // Traverse the linked list and count nodes
         while (current != NULL)
         {
             n++;  // Increment count for current node
             current = current->next;  // Move to next node               
         }

         return n;  // Return total number of links
     }

     DIMENSIONSPROPERTIES_PTR getNext() 
     {
        if (this->properties != NULL)
        {
            return this->properties->next;
        }

        return NULL;
     }

     DIMENSIONSPROPERTIES_PTR getPrev() 
     {
        if (this->properties != NULL)
        {
            return this->properties->prev;
        }

        return NULL;
     }

    /**
     * @brief Assignment operator with INTENTIONAL shared ownership semantics.
     *
     * DESIGN DECISION: This class uses MIXED COPY SEMANTICS by design:
     * - Copy constructor creates DEEP, INDEPENDENT copies
     * - Assignment operator creates SHALLOW, SHARED ownership
     *
     * This provides different behaviors for different use cases:
     * - Use copy constructor when you need independent objects
     * - Use assignment when you want efficient shared ownership
     *
     * Current Behavior (INTENTIONAL SHARED OWNERSHIP):
     * - Releases current object's reference to its linked list
     * - Makes this object share the same linked list nodes as the source object
     * - Increments reference counts to track shared ownership
     * - Multiple objects now share the same underlying structure
     *
     * Shared Ownership Implications:
     * - Changes to dimensional structure affect ALL objects sharing the same nodes
     * - Memory is automatically managed through reference counting
     * - Objects sharing nodes will keep them alive until all references are gone
     * - More memory efficient than deep copying for read-heavy workloads
     *
     * Usage Examples:
     * Dimensions a(dimensionsArray);     // Create original
     * Dimensions b = a;                  // DEEP copy - b is independent
     * Dimensions c;
     * c = a;                             // SHALLOW copy - c shares with a
     * 
     * // Now: modifying 'a' affects 'c' but NOT 'b'
     *
     * Thread Safety:
     * - Reference counting operations are NOT thread-safe
     * - Concurrent access to shared nodes requires external synchronization
     * - Consider the implications of shared mutable state in multi-threaded code
     *
     * @param other The source Dimensions object to share ownership with
     * @return Reference to this object for chaining assignments
     * 
     * @warning This creates shared ownership - modifications affect all sharing objects
     * @warning Not thread-safe - external synchronization required
     * @note This behavior is intentionally different from the copy constructor
     * @see Copy constructor for deep copy semantics
     */
     Dimensions& operator= (const Dimensions& other)
     {        
         // Check for self-assignment
         if (this == &other)
         {                        
             return *this;    
         }

         if (this->properties == NULL || other.properties == NULL || this->properties == other.properties)
         {           
            return *this;
         }
    
         //std::cout<< "OK HERE 1" << std::endl;

         // Release this object's current references
         // This will deallocate nodes if we were the last reference holder
         this->decrementReferenceCount();

         //std::cout<< "OK HERE 2" << std::endl;

         // Share ownership with the source object (shallow copy)
         // Both objects now point to the same linked list structure
         this->properties = other.properties;

         // Increment reference count to reflect the new shared ownership
         // All nodes in the linked list now have one more reference
         this->incrementReferenceCount();
                              
         return *this;        
     }
     
     Dimensions& operator= (DIMENSIONSPROPERTIES_PTR other)
     {
        // Check for self-assignment
        if (this->properties == other)
        {
            return *this;
        }

        // Release this object's current references
        // This will deallocate nodes if we were the last reference holder
        this->decrementReferenceCount();
       
        // Share ownership with the source object (shallow copy)
        // Both objects now point to the same linked list structure
        this->properties = other;

        // Increment reference count to reflect the new shared ownership
        // All nodes in the linked list now have one more reference
        this->incrementReferenceCount();

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
                ret = this->properties->rows;
            break;
            case NUMCY_DIMENSIONS_SHAPE_COLUMNS:
                ret = this->properties->columns;
            break;
        }

        return ret;     
    }

    /**
     * @brief Overloaded equality operator for comparing two Dimensions objects.
     *
     * This operator performs a comprehensive comparison of two Dimensions objects by
     * examining their dimensional structure and values. Two Dimensions objects are
     * considered equal if they have identical dimensional shapes and sizes.
     *
     * Comparison Process:
     * 1. Quick validation: Compare total element count (getN()) for fast inequality detection
     * 2. Structure validation: Compare number of links to ensure same dimensional depth
     * 3. Detailed comparison: Traverse both linked lists simultaneously comparing each node
     * 4. Node comparison: Check both rows and columns values for each corresponding node
     *
     * Comparison Criteria:
     * - Both objects must have the same total number of elements
     * - Both objects must have the same number of dimensional links
     * - Each corresponding node must have identical rows and columns values
     * - The linked list structures must be of identical length
     *
     * Performance Optimizations:
     * - Early exit on total element count mismatch (fastest check)
     * - Early exit on link count mismatch (avoids unnecessary traversal)
     * - Short-circuit evaluation during node-by-node comparison
     *
     * Edge Cases Handled:
     * - Both objects empty (properties == NULL): considered equal
     * - One empty, one non-empty: returns false
     * - Different structure lengths: returns false
     *
     * @param other The Dimensions object to compare against (should be const Dimensions&)
     * @return true if both objects have identical dimensional structure and values,
     *         false otherwise
     *
     * @note Parameter should be 'const Dimensions&' for proper const-correctness
     * @note This is a deep structural comparison, not reference/pointer comparison
     * @warning Not thread-safe if either object is being modified during comparison
     */
    bool operator==(const Dimensions& other) const  // Fixed: should be const Dimensions&
    {
        /*
         * Quick elimination: Compare total element counts first
         * This is the fastest way to detect inequality for different-sized arrays
         */
        if (getN() != other.getN())
        {
            return false;
        }

        /*
         * Structure validation: Check number of links
         * Both objects should have the same dimensional depth
         */
        if (getNumberOfLinks() != other.getNumberOfLinks())
        {
            return false;
        }
                
        /*
         * Detailed node-by-node comparison:
         * Traverse both linked lists simultaneously and compare each node
         */    
        DIMENSIONSPROPERTIES_PTR receiver = this->properties;
        DIMENSIONSPROPERTIES_PTR other_receiver = other.properties;

        // Handle case where both objects are empty
        if (receiver == NULL && other_receiver == NULL)
        {
            return true;  // Both empty - considered equal
        }

        // Traverse both lists simultaneously
        while (receiver != NULL && other_receiver != NULL)
        { 
            // Compare both rows and columns for current nodes
            if (receiver->rows != other_receiver->rows || receiver->columns != other_receiver->columns)
            {
                return false;  // Found a mismatch
            }

            // Move to next nodes in both lists
            receiver = receiver->next;
            other_receiver = other_receiver->next;
        }
               
        // At this point, both pointers should be NULL if lists are same length
        // If one is NULL and other isn't, the lists have different lengths
        return (receiver == NULL && other_receiver == NULL);
    }

} DIMENSIONS;

typedef Dimensions* DIMENSIONS_PTR;
typedef const Dimensions* CONST_DIMENSIONS_PTR;

#endif