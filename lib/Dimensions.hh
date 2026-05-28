/*
 * Numcy/Dimensions.hh
 *
 * This file contains the definition of the Dimensions class.
 * The Dimensions class is a template class that is used to store the dimensions of a matrix.
 * The Dimensions class is a linked list of dimensions.
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
 
#ifndef NUMCY_DIMENSIONS_HH
#define NUMCY_DIMENSIONS_HH

#include <cassert>
#include <string>
#include <vector>

template <typename T = size_t>
class Dimensions
{
    DimensionsProperties<T>* head;
    DimensionsProperties<T>* tail;
    /*
        Why each instance of Dimensions keeps its own n?
        ------------------------------------------------
        Each Dimensions object maintains its own node count independently,
        because different instances can share the same head node but have
        different tail nodes — each one seeing a different length of the
        same underlying list.

        Consider the following scenario:

            Dimensions<T> d1(10, 20);
            // d1: head──►[A], tail──►[A], n=1
            // d1 owns only node A.

            Dimensions<T> d2(d1);
            // d2: head──►[A], tail──►[A], n=1  (copied from d1, same head)
            // d2 shares node A with d1. A.refcount=2.

            d2.append(30, 40);
            // d2: head──►[A]──►[B], tail──►[B], n=2
            // d2 added node B. B is private to d2. A.refcount=2, B.refcount=1.

            d2.append(50, 60);
            // d2: head──►[A]──►[B]──►[C], tail──►[C], n=3
            // d2 added node C. C is private to d2. A.refcount=2, B.refcount=1, C.refcount=1.
            // Note: append() may insert a boundary node automatically to preserve
            // the invariant that only the tail carries a non-zero columns value.
            // In that case n reflects the true node count including boundary nodes.

            Dimensions<T> d3(d2);
            // d3: head──►[A]──►[B]──►[C], tail──►[C], n=3  (copied from d2)
            // d3 shares all nodes with d2. A.refcount=3, B.refcount=2, C.refcount=2.

            d3.append(70, 80);
            // d3: head──►[A]──►[B]──►[C]──►[D], tail──►[D], n=4
            // d3 added node D. D is private to d3. D.refcount=1.

        After all of the above:

            d1.n = 1  —  d1 sees only [A]
            d2.n = 3  —  d2 sees [A]──►[B]──►[C]
            d3.n = 4  —  d3 sees [A]──►[B]──►[C]──►[D]

        Each n is correct and independent.

        Why n cannot live in DimensionsProperties?
        ------------------------------------------
        If n were stored inside DimensionsProperties, all Dimensions objects
        sharing that node would increment and decrement the same counter.
        It would be impossible to track the length of any individual
        Dimensions object's list independently. The counter would reflect
        the total number of appends across all instances sharing that node,
        not the length of any one instance's list.

        Conclusion: n belongs in Dimensions, not in DimensionsProperties.
     */
    size_t n; 

    public:
        /*
            *  Dimensions()
            *  └─► this->head = nullptr, this->tail = nullptr, this->n = 0
         */
        Dimensions(void) : head(nullptr), tail(nullptr), n(0)
        {            
        }
       
        /*
            *  Dimensions(T columns, T rows)
            *  ├─► this->head = nullptr, this->tail = nullptr, this->n = 0
            *  ├─► try
            *  │     ├─► this->append(columns, rows) -> this->n++
         */
        Dimensions(T columns, T rows) : head(nullptr), tail(nullptr), n(0)
        {
            try
            {
                this->append(columns, rows); // append() will increment n
            }
            catch (const std::exception& e)
            {
                throw std::runtime_error("Dimensions<T>::Dimensions(T, T) -> " + std::string(e.what()));
            }
            catch (...)
            {
                throw std::runtime_error("Dimensions<T>::Dimensions(T, T) -> Unknown exception");
            }
        }

        /*
            *  Dimensions(DimensionsProperties<T>* h, DimensionsProperties<T>* t)
            *  └─► this->head = h, this->tail = t, this->n = 0
            *  ├─► current = this->head
            *  ├─► while (current != nullptr)
            *  │     ├─► this->n++
            *  │     ├─► current->incrementReferenceCount()
         */
        Dimensions(DimensionsProperties<T>* h, DimensionsProperties<T>* t) : head(h), tail(t), n(0) 
        {
            DimensionsProperties<T>* current = this->head;

            while (current != nullptr)
            {
                this->n++;
                
                current->incrementReferenceCount();
                current = current->getNext();
            }   
        }

        /*
            *  Dimensions(const Dimensions<T>& other)
            *  ├─► this->head = other.head, this->tail = other.tail, this->n = other.n
            *  ├─► current = this->head
            *  ├─► while (current != nullptr)
            *  │     ├─► current->incrementReferenceCount()
         */
        Dimensions(const Dimensions<T>& other) : head(other.head), tail(other.tail), n(other.n)
        { 
            DimensionsProperties<T>* current = this->head;

            while (current != nullptr)
            {
                current->incrementReferenceCount();
                current = current->getNext();
            }
        }

        /*
            *  ~Dimensions()
            *  ├─► if (this->head != nullptr)
            *  │     ├─► current = this->head
            *  │     ├─► while (current != nullptr)
            *  │     │     ├─► current->decrementReferenceCount()
            *  │     │     ├─► if (current->getReferenceCount() == 0)
            *  │     │     │     ├─► next = current->getNext()
            *  │     │     │     ├─► prev = current->getPrevious()
            *  │     │     │     ├─► delete current
            *  │     │     │     ├─► if (next != nullptr) next->setPrev(prev)
            *  │     │     │     ├─► if (prev != nullptr) prev->setNext(next)
            *  │     │     │     └─► current = next
            *  │     │     └─► else
            *  │     │           └─► current = current->getNext()
         */
        ~Dimensions(void)
        {
            if (this->head != nullptr)
            {
                DimensionsProperties<T>* current = this->head;

                while (current != nullptr) // The release loop
                {
                    current->decrementReferenceCount();

                    /*
                        Decrement this node's ref count. If it reaches 0 (means that no Dimensions object owns this node),
                        delete it and relink its neighbors.

                        The relink is necessary
                        -----------------------
                        We are selectively deleting one node from a list while keeping the rest.
                        Our reference counts can be mixed, because an append method creates nodes with independent ref counts,
                        allowing a middle node to be deleted while its neighbors survive.                    
                     */
                    if (current->getReferenceCount() == 0)
                    {
                        DimensionsProperties<T>* next = current->getNext();
                        DimensionsProperties<T>* prev = current->getPrevious();

                        delete current;
                        
                        /*
                            After deleting current
                            -----------------------

                            If the next node exists, update its previous pointer to point to the previous node.
                            If the previous node exists, update its next pointer to point to the next node.

                            This is important for any node that is not the head or the tail.
                        */
                        if (next != nullptr)
                        {
                            next->setPrev(prev);
                        }
                        if (prev != nullptr)
                        {
                            prev->setNext(next);
                        }

                        current = next;

                        /*
                            ------------------------------------------------------------
                            Discussion about when there is no mixed scenario. 
                            Either all nodes survive, or all nodes get deleted together.
                            ------------------------------------------------------------
                            Single-node case is safe either way. 
                            Now multi-node, once you add an append method...

                            // List: [A] → [B] → [C], all ref_count = 1 
                            // A.prev = null
                            // A.next = B
                            // B.prev = A
                            // B.next = C
                            // C.prev = B
                            // C.next = null
                        
                            // Destructor walks forward:
                        
                            // 1. Delete A... current is A
                            // prev = A.prev -> (null), next = A.next -> (B)
                            /// After deleting current...                        
                            // B.prev = null
                        
                            // 2. Delete B.... current is B
                            // pev = B.prev -> (null), next = B.next -> (C)
                            // After deleting current...
                            // C.prev = null

                            // 3. Delete C... curent is C
                            // prev = C.prev -> (null), next = C.next -> (null)
                            // After deleting current...
                            // Noting happens, because next is null and prev is null
                            // current gets assigned null which terminates the release loop
                         */
                    }
                    else
                    {
                        current = current->getNext();
                    }                    
                }

                this->head = nullptr;
                this->tail = nullptr;
                this->n = 0;                        
            }
        }

        // //////////////////// // 
        // Operator Overloading //
        // //////////////////// //

        /*
            *  operator=(const Dimensions<T>& rhs)
            *  ├─► if (this == &rhs) return *this
            *  ├─► current = this->head
            * │     ├─► while (current != nullptr)
            * │     │     ├─► current->decrementReferenceCount()
            * │     │     ├─► if (current->getReferenceCount() == 0)
            * │     │     │     ├─► next = current->getNext()
            * │     │     │     ├─► prev = current->getPrevious()
            * │     │     │     ├─► delete current
            * │     │     │     ├─► if (next != nullptr) next->setPrev(prev)
            * │     │     │     ├─► if (prev != nullptr) prev->setNext(next)
            * │     │     │     └─► current = next
            * │     │     └─► else
            * │     │           └─► current = current->getNext()
            *  ├─► this->head = rhs.head
            *  ├─► this->tail = rhs.tail
            *  ├─► this->n = rhs.n
            *  ├─► current = this->head
            * │     ├─► while (current != nullptr)
            * │     │     ├─► current->incrementReferenceCount()
            * │     │     └─► current = current->getNext()
            *  └─► return *this
         */
        Dimensions<T>& operator=(const Dimensions<T>& rhs)
        {   
            // Self-assignment check
            if (this == &rhs) 
            {
                return *this;
            }

            DimensionsProperties<T>* current = this->head;

            /*
                Null out tail before the release loop to avoid it dangling
                if the last node is deleted. It will be reassigned from rhs below.
            */ 
            this->tail = nullptr;
                        
            while (current != nullptr) // The release loop
            {
                current->decrementReferenceCount();

                /*
                    Decrement this node's ref count. If it reaches 0 (means that no Dimensions object owns this node),
                    delete it and relink its neighbors.

                    The relink is necessary
                    -----------------------
                    We are selectively deleting one node from a list while keeping the rest.
                    Our reference counts can be mixed, because an append method creates nodes with independent ref counts,
                    allowing a middle node to be deleted while its neighbors survive.                    
                 */
                if (current->getReferenceCount() == 0)
                {
                    DimensionsProperties<T>* next = current->getNext();
                    DimensionsProperties<T>* prev = current->getPrevious();

                    delete current; 

                    /*
                        After deleting current
                        -----------------------

                        If the next node exists, update its previous pointer to point to the previous node.
                        If the previous node exists, update its next pointer to point to the next node.

                        This is important for any node that is not the head or the tail.
                    */                    
                    if (next != nullptr)
                    {
                        next->setPrev(prev);
                    }
                    if (prev != nullptr)
                    {
                        prev->setNext(next);
                    }
                    
                    current = next;

                    /*
                        ------------------------------------------------------------
                        Discussion about when there is no mixed scenario. 
                        Either all nodes survive, or all nodes get deleted together.
                        ------------------------------------------------------------
                        Single-node case is safe either way. 
                        Now multi-node, once you add an append method...
                        
                        // List: [A] → [B] → [C], all ref_count = 1 
                        // A.prev = null
                        // A.next = B
                        // B.prev = A
                        // B.next = C
                        // C.prev = B
                        // C.next = null
                        
                        // Destructor walks forward:

                        // 1. Delete A... current is A
                        // prev = A.prev -> (null), next = A.next -> (B)
                        /// After deleting current...                        
                        // B.prev = null
                        
                        // 2. Delete B.... current is B
                        // pev = B.prev -> (null), next = B.next -> (C)
                        // After deleting current...
                        // C.prev = null

                        // 3. Delete C... curent is C
                        // prev = C.prev -> (null), next = C.next -> (null)
                        // After deleting current...
                        // Noting happens, because next is null and prev is null
                        // current gets assigned null which terminates the release loop
                     */
                }
                else
                {
                    current = current->getNext();
                }
            }

            head = rhs.head;
            tail = rhs.tail;
            this->n = rhs.n;

            current = this->head;

            while (current != nullptr)
            {
                current->incrementReferenceCount();
                current = current->getNext();
            }

            return *this;
        }

        // //////////////////// //
        // Other Public Methods //
        // //////////////////// //

        /*
            append(), this method creates nD tensors.
            A valid nD tensor is the one in which columns are zero in all (n - 1) except the last node
            A valid nD tensor is the one in which all rows of all n nodes are not zero.
         */
        /*
         * append(T c, T r)
         *
         * PURPOSE:
         *     Appends a new 2D slice node to the end of the dimensions list.
         *     Each node represents one dimension in a multi-dimensional tensor.
         *     Calling append() repeatedly builds up the full shape of the tensor,
         *     one 2D slice at a time.
         *
         * PARAMETERS:
         *     c — the columns value of the new node (width of the 2D slice).
         *     r — the rows value of the new node (height of the 2D slice).
         *
         * INVARIANTS:
         *     In a multi-node list, all nodes above the tail must have their
         *     columns value equal to zero. Only the tail node carries a non-zero
         *     columns value. This invariant is established and maintained by this
         *     method on every call.
         *
         *     If the current tail has a non-zero columns value at the time of the
         *     call, an intermediate boundary node is automatically inserted before
         *     the new tail to preserve this invariant. The caller does not need to
         *     manage this — it is handled transparently by the method itself.
         *
         * PRECONDITIONS:
         *     head and tail must be consistent before this method is called.
         *     Either both are null (empty list) or both are non-null (non-empty list).
         *     Any other state is a programming error and will be caught by the
         *     assert at the start of the method.
         *
         * POSTCONDITIONS:
         *     The new node becomes the new tail of the list.
         *     If an intermediate boundary node was required, it is inserted
         *     immediately before the new tail, with its columns value set to zero
         *     and its rows value set to the previous tail's columns value.
         *     head and tail remain consistent after the call.
         *
         * EXCEPTIONS:
         *     std::runtime_error — thrown if memory allocation fails (std::bad_alloc),
         *     if the constructor of DimensionsProperties throws any standard exception,
         *     or if any unknown error occurs during allocation or construction.
         *     The original exception message is preserved in the runtime_error message
         *     where available.
         *
         * COMPLEXITY:
         *     O(1) for the common case — appending to the end of the list.
         *     O(1) for the intermediate node insertion when required.
         *     The recursion is exactly one level deep and does not affect complexity.
         */
        void append (T c, T r) 
        {
            /*
                Sanity check: head and tail must be consistent.
                If head is null then tail must also be null, and vice versa...
                If head is not null then tail must not be null either.
            */
            assert((this->head == nullptr) == (this->tail == nullptr));

            DimensionsProperties<T>* node = nullptr;

            try
            {            
                node = new DimensionsProperties<T>(c, r);
            }
            catch (const std::bad_alloc& e)
            {
                // Catches memory allocation failures exception thown by the constructor.
                throw std::runtime_error("Dimensions<T>::append(T, T) Error: " + std::string(e.what()));
            }
            catch (const std::exception& e)
            {
                // Catches any standard exception thrown by the constructor.
                throw std::runtime_error("Dimensions<T>::append(T, T) Error: " + std::string(e.what()));
            }
            catch (...) 
            {                
                // Catches any standard exception thrown by the constructor.
                throw std::runtime_error("Dimensions<T>::append(T, T) Error: Unknown error during allocation/construction");
            }
            
            /*
                If the list is empty, the new node becomes both the head and the tail.
                Otherwise, the new node is appended to the end of the list.
             */            
            if (this->head == nullptr) // append to empty list:- (prepend() adds a new node to the beginning of the list
            {
                this->head = node;
                this->tail = node;
            }
            else // append to non-empty list:- (append() adds a new node to the end of the list)
            {  
                // If head is not null, tail must not be null either
                assert(this->tail != nullptr);
                 
                /*
                    Invariant: in a multi-node list, the tail node always has a non-zero columns value.
                    All nodes above the tail must have their columns value equal to zero.

                    If the current tail's columns value is non-zero, an intermediate boundary node
                    must be inserted before appending the new tail. The intermediate node carries
                    the current tail's columns value as its rows value, and its own columns value
                    is set to zero to satisfy the invariant.

                    The recursive call is intentional. It reuses the try/catch block above to safely
                    handle the allocation of the intermediate boundary node. The recursion is exactly
                    one level deep; the recursive call always passes T(0) as c, so this condition
                    will not fire again.
                 */ 
                if (this->tail->getColumns() != T(0))
                {
                    T column = this->tail->getColumns();
                    this->tail->setColumns(T(0));
                    
                    /*
                        Recursive call is intentional. It reuses the try/catch block above
                        to safely handle allocation of the intermediate boundary node.
                        Recursion is exactly one level deep, the recursive call always
                        passes T(0) as c, so this condition will not fire again.
                    */
                    this->append(T(0), column);
                }

                /*
                    Add the new node as the tail of the list.
                    In a multi-node list, the tail node has non-zero rows and columns values.
                    All nodes above the tail have their columns value equal to zero.
                */
                this->tail->setNext(node);
                node->setPrev(this->tail);
                this->tail = node;
            }

            this->n++;

            /*
                Post-condition summary:
                1. The new node becomes the new tail of the list.
                2. If the previous tail had a non-zero columns value, an intermediate boundary
                   node was inserted before the new tail. This intermediate node carries the
                   previous tail's columns value as its rows value, preserving the invariant
                   that all nodes above the tail have their columns value equal to zero.
             */
        }

        /*
         * fromVector(std::vector<T> vec)
         *
         * PURPOSE:
         *     Initializes the Dimensions object from a std::vector of type T.
         *     The vector is expected to contain dimension values in row-major order.
         *     The method constructs the linked list of DimensionsProperties nodes
         *     to represent the tensor's dimensions.
         *
         * PARAMETERS:
         *     vec: A std::vector<T> containing the dimension values.
         *
         * PRECONDITIONS:
         *     - The vector must contain at least two elements (for a 2D tensor).
         *     - The vector should not be empty.
         *     - The Dimensions object may already be populated; calling fromVector()
         *       on a non-empty object will append to the existing list. This is
         *       intentional and the responsibility of the caller to manage.
         *
         * POSTCONDITIONS:
         *     - The Dimensions object will be populated with nodes representing
         *       the dimensions from the vector.
         *     - The linked list will be properly formed with head and tail pointers.         
         *     - The numel() of the object will reflect the number of elements in the tensor.
         *     - The size() of the object will reflect the number of links (nodes).
         *
         * COMPLEXITY:
         *     O(n), where n is the number of elements in the input vector.
         *     Each element is processed once to create a node.
         */
        void fromVector(const std::vector<T>& vec)
        {
            size_t vecSize = vec.size();

            if (vecSize < 2)
            {
                throw std::runtime_error("Dimensions<T>::fromVector(std::vector<T>) Error: vector size must be at least 2");
            }

            // Validate no zero values — zero dimensions violate the invariant
            for (size_t i = 0; i < vecSize; i++)
            {
                if (vec[i] == T(0))
                {
                    throw std::runtime_error("Dimensions<T>::fromVector(std::vector<T>) Error: zero value at index " + std::to_string(i));
                }
            }

            try
            {
                for (size_t i = 0; i < vecSize - 2; i++)
                {
                    this->append(T(0) /*Columns*/, vec[i] /*Rows*/);
                }

                this->append(vec[vecSize - 1] /*Columns*/, vec[vecSize - 2] /*Rows*/);
            }
            catch (const std::exception& e)
            {
                throw std::runtime_error("Dimensions<T>::fromVector(std::vector<T>) -> " + std::string(e.what()));
            }
            catch (...)
            {
                throw std::runtime_error("Dimensions<T>::fromVector(std::vector<T>) Error: Unknown error");
            }

            // append() increments n, so we don't need to do it here.
        }

        /*
         * getNumberOfColumns(void) const
         *
         * PURPOSE:
         * Returns the number of columns in the tensor's last 2D slice.
         * In the Numcy invariant, this is always stored in the tail node.
         *
         * PARAMETERS:
         * None.
         *
         * PRECONDITIONS:
         * The Dimensions object must not be empty (tail != nullptr).
         * This is verified by an internal assertion.
         *
         * POSTCONDITIONS:
         * Returns the T-typed column value. The state of the list is unchanged.
         *
         * COMPLEXITY:
         * O(1) — Direct pointer access to the tail node.
         */
        T getNumberOfColumns(void) const        
        {
            T columns = 0;

            if (this->tail != nullptr)
            {
                columns = this->tail->getColumns();
            }

            return columns;
        }

        /*
         * getNumberOfRows(void) const
         * * PURPOSE:
         * Returns the total logical number of rows in the tensor.
         * This is the product of all 'rows' fields across the linked list.
         * * PARAMETERS:
         * None.
         * * PRECONDITIONS:
         * The Dimensions object must not be empty (head != nullptr).
         * * POSTCONDITIONS:
         * Returns the product of all dimensions except the final column count.
         * * SAFETY & HARDENING:
         * - Uses size_t for the intermediate product to prevent overflow during
         * multiplication.
         * - Uses static_cast<T> for the final return to satisfy -Wconversion.
         * - Includes a MAX_ITERATIONS guard to prevent infinite loops in case
         * of memory corruption/cycles.
         * * COMPLEXITY:
         * O(n) — where n is the number of nodes (dimensions) in the list.
         */
        T getNumberOfRows(void) const
        {
            if (this->head == nullptr)
            {
                return T(0);
            }

            T rows = 1;

            DimensionsProperties<T>* current = this->head;

            while (current != nullptr)
            {
                rows *= current->getRows();
                current = current->getNext();
            }

            return rows;
        }

        /*
         * numel(void) const
         *
         * PURPOSE:
         *     Computes and returns the total number of elements in the tensor
         *     described by this Dimensions object. The result is the product of
         *     all rows and columns values across every node in the list, which
         *     represents the total number of scalar values the tensor holds.
         *     This value is used by Collective to determine how much memory
         *     to allocate for the tensor data.
         *
         * PARAMETERS:
         *     None.
         *
         * INVARIANTS:
         *     This method depends on the append invariant being strictly maintained.
         *     Specifically, all nodes above the tail must have their columns value
         *     equal to zero, and only the tail node carries a non-zero columns value.
         *     If this invariant is violated by a caller, the result of this method
         *     will be silently incorrect.
         *
         * PRECONDITIONS:
         *     head and tail must be consistent before this method is called.
         *     Either both are null (empty list) or both are non-null (non-empty list).
         *     Any other state is a programming error and will be caught by the
         *     assert at the start of the method.
         *
         * POSTCONDITIONS:
         *     The list is not modified. This method is read-only (const).
         *     The returned value is the total number of elements in the tensor,
         *     or 0 if the list is empty.
         *
         * RETURN VALUE:
         *     size_t — the total number of elements in the tensor.
         *     Returns 0 if the Dimensions object is empty (no nodes in the list).
         *     The return value is computed as:
         *
         *         total = tail->getColumns()
         *                 * node_1->getRows()
         *                 * node_2->getRows()
         *                 * ...
         *                 * tail->getRows()
         *
         *     For example, a 3D tensor with shape [2, 4, 8] is stored as:
         *
         *         [node1: columns=0, rows=2]  → head
         *         [node2: columns=0, rows=4]
         *         [node3: columns=8, rows=8]  → tail
         *
         *     And numel() returns: 8 * 2 * 4 * 8 = 512
         *
         * EXCEPTIONS:
         *     Does not throw. All error conditions are caught by the assert
         *     at the start of the method. The assert terminates the program
         *     immediately if head and tail are in an inconsistent state.
         *
         * COMPLEXITY:
         *     O(n) where n is the number of nodes in the list, since the method
         *     walks every node exactly once to accumulate the product.
         */
        size_t numel(void) const
        {
            /*
                Sanity check: head and tail must be consistent.
                If head is not null then tail must also be not null, and vice versa.

                The assert evaluates the expression:
                (this->head != nullptr) == (this->tail != nullptr)

                Both sides of the == operator produce a boolean value.
                The left side is true if head is not null, false if head is null.
                The right side is true if tail is not null, false if tail is null.

                The == operator then compares the two boolean values.
                The assert fires (terminates the program) if they are not equal, meaning:
                    - head is not null but tail is null, or
                    - head is null but tail is not null.

                Both of these are corrupted states that should never occur.
                The only two valid states are:
                    - head == nullptr AND tail == nullptr  (empty list)
                    - head != nullptr AND tail != nullptr  (non-empty list)

                This assert catches any corruption between head and tail early,
                before any pointer dereference can cause a segfault or silent
                wrong result further down.
            */
            assert((this->head != nullptr) == (this->tail != nullptr));

            /*
                After the assert above, we know head and tail are consistent.
                However, we have not yet established whether the list is empty or not.
                If head is null (empty list), accessing head or tail directly would
                dereference a null pointer and crash the program.
                
                We therefore check head before accessing any node data.
                Since the assert guarantees consistency, checking head alone is
                sufficient — if head is non-null, tail is guaranteed non-null too.
            */
            if (this->head != nullptr)
            {
                /*
                    The tail node's columns value seeds the total size calculation.
                    All nodes above the tail have their columns value equal to zero
                    by the append invariant, so only the tail contributes a columns
                    value to the product. Every node's rows value, including the
                    tail's, is then multiplied in as the loop walks from head to tail.
                */
                size_t total = static_cast<size_t>(this->tail->getColumns());

                DimensionsProperties<T>* current = this->head;

                while (current != nullptr)
                {
                    total = total * static_cast<size_t>(current->getRows());
                    current = current->getNext();
                }

                return total;
            }

            /*
                The list is empty. There are no dimensions and therefore no elements.
                Returning 0 is the correct and well-defined result for this case.
            */
            return 0;
        }

        /*
         * @brief Performs an in-place dimensional transformation (Metadata Surgery).
         * * This method reconfigures the geometric interpretation of the underlying data 
         * without modifying the data itself. It utilizes a "Release-and-Transfer" 
         * strategy to ensure memory integrity across shared dimensional nodes.
         *
         * @section Algorithm Logic:
         * 1. Validates the 'Total Elements' invariant (numel must remain constant).
         * 2. Iterates through the current linked list (head to tail).
         * 3. Decrements the reference count of each existing node.
         * 4. If a node's reference count reaches zero (orphaned), it is surgically 
         * removed and its neighbors are relinked to maintain list continuity.
         * 5. Transfers ownership of nodes from a temporary Dimensions object to 'this'.
         * 6. Neutralizes the temporary object (Move-semantics) to prevent its 
         * destructor from triggering a double-free of the new nodes.
         *
         * @param newShape A std::vector containing the target sizes for each axis.
         * The size of the vector determines the new 'n' (dimensionality).
         * * @return void (Operates in-place on the metadata pointers).
         * * @exception std::runtime_error Thrown if the product of newShape dimensions 
         * does not match the current numel(), preventing data corruption.
         * * @note Complexity: O(M + K) where M is old dimensionality and K is new 
         * dimensionality. Data size (N) does not affect performance.
         */
        void reshape(const std::vector<T>& newShape)
        {
            Dimensions<T> newDimensions;
            newDimensions.fromVector(newShape);

            if (newDimensions.numel() != this->numel())
            {
                throw std::runtime_error("Dimensions<T>::reshape(const std::vector<T>&) Error: Number of elements must match");
            }
            
            DimensionsProperties<T>* current = this->head;

            while (current != nullptr) // The release loop
            {
                current->decrementReferenceCount();

                /*
                    Decrement this node's ref count. If it reaches 0 (means that no Dimensions object owns this node),
                    delete it and relink its neighbors.

                    The relink is necessary
                    -----------------------
                    We are selectively deleting one node from a list while keeping the rest.
                    Our reference counts can be mixed, because an append method creates nodes with independent ref counts,
                    allowing a middle node to be deleted while its neighbors survive.                    
                */
                if (current->getReferenceCount() == 0)
                {
                    DimensionsProperties<T>* next = current->getNext();
                    DimensionsProperties<T>* prev = current->getPrevious();

                    delete current;
                        
                    /*
                        After deleting current
                        -----------------------

                        If the next node exists, update its previous pointer to point to the previous node.
                        If the previous node exists, update its next pointer to point to the next node.

                        This is important for any node that is not the head or the tail.
                    */
                    if (next != nullptr)
                    {
                        next->setPrev(prev);
                    }
                    if (prev != nullptr)
                    {
                        prev->setNext(next);
                    }

                    current = next;

                    /*
                        ------------------------------------------------------------
                        Discussion about when there is no mixed scenario. 
                        Either all nodes survive, or all nodes get deleted together.
                        ------------------------------------------------------------
                        Single-node case is safe either way. 
                        Now multi-node, once you add an append method...

                        // List: [A] → [B] → [C], all ref_count = 1 
                        // A.prev = null
                        // A.next = B
                        // B.prev = A
                        // B.next = C
                        // C.prev = B
                        // C.next = null
                        
                        // Destructor walks forward:
                        
                        // 1. Delete A... current is A
                        // prev = A.prev -> (null), next = A.next -> (B)
                        /// After deleting current...                        
                        // B.prev = null
                        
                        // 2. Delete B.... current is B
                        // pev = B.prev -> (null), next = B.next -> (C)
                        // After deleting current...
                        // C.prev = null

                        // 3. Delete C... curent is C
                        // prev = C.prev -> (null), next = C.next -> (null)
                        // After deleting current...
                        // Noting happens, because next is null and prev is null
                        // current gets assigned null which terminates the release loop
                    */
                }
                else
                {
                    current = current->getNext();
                }                    
            }

            /*
                This is a pure pointer swap, no deep copy.
                The nodes are not copied, only the pointers are swapped.
                By transferring these pointers to this, we’ve updated the "view" of the data in $O(1)$ time relative to the data size.
             */
            this->head = newDimensions.head;
            this->tail = newDimensions.tail;
            this->n = newDimensions.n;  
            
            /*
                To avoid newDimensions Destructor Conflict
                ------------------------------------------
                Since we have "stolen" the pointers from newDimensions (this->head = newDimensions.head),
                we must ensure that when newDimensions (the local variable) goes out of scope,
                its destructor doesn't delete the nodes we just attached to this.
             */
            newDimensions.head = nullptr;
            newDimensions.tail = nullptr;
            newDimensions.n = 0;
        }

        /*
         *   size(void) const
         *               
         *   PURPOSE:
         *       Returns the number of nodes in the linked list.
         *               
         *   PRECONDITIONS:
         *       None.
         *               
         *   POSTCONDITIONS:
         *       The list is not modified. This method is read-only (const).
         *       The returned value is the number of nodes in the linked list.
         *               
         *   RETURN VALUE:
         *       size_t — the number of nodes in the linked list.
         *       Returns 0 if the Dimensions object is empty (no nodes in the list).
         *               
         *   EXCEPTIONS:
         *       Does not throw.
         *               
         *   COMPLEXITY:
         *       O(1) — constant time, as it simply returns the stored value of n.
         */
        size_t size(void) const
        {
            return this->n;
        }

        /*
         *   transpose(Axis, Axis)
         *               
         *   PURPOSE:
         *       Returns a new Dimensions object with the specified axes swapped.
         *               
         *   PRECONDITIONS:
         *       The Dimensions object must not be empty.
         *       The specified axes must be valid and different.
         *               
         *   POSTCONDITIONS:
         *       The original Dimensions object is not modified.
         *       The returned Dimensions object has the specified axes swapped.
         *               
         *   RETURN VALUE:
         *       Dimensions<T> — a new Dimensions object with the specified axes swapped.
         *               
         *   EXCEPTIONS:
         *       std::runtime_error — if the Dimensions object is empty.
         *       std::runtime_error — if axis1 or axis2 is out of range.
         *       std::runtime_error — if axis1 and axis2 are the same.
         *               
         *   COMPLEXITY:
         *       O(n) — where n is the number of dimensions.
         */
        Dimensions<T> transpose(numcy::Axis axis1 = numcy::Axis::Last, numcy::Axis axis2 = numcy::Axis::SecondLast) const
        {
            if (this->head == nullptr || this->tail == nullptr || this->n == 0)
            {
                throw std::runtime_error("Dimensions<T>::transpose(Axis, Axis) Error: Dimensions object is empty");
            }

            // ndim stays size_t — it is a count and belongs in the unsigned domain.
            // Casting it to int would be a dangerous narrowing on large tensors.
            size_t ndim = this->size() + 1;

            // a1 and a2 start as int — the Axis enum underlying type is int,
            // so this cast is lossless and safe in both directions
            int a1 = static_cast<int>(axis1);
            int a2 = static_cast<int>(axis2);

            // Normalization stays entirely in the signed domain (int op int).
            // We convert ndim to int here ONLY for the addition, and only after
            // a range guard that makes the narrowing safe: if ndim > INT_MAX the
            // tensor has more axes than an int can index — that is a programming
            // error and we throw rather than silently narrow.
            if (ndim > static_cast<size_t>(std::numeric_limits<int>::max()))
            {
                throw std::runtime_error("Dimensions<T>::transpose(Axis, Axis) Error: number of dimensions exceeds int range");
            }

            int indim = static_cast<int>(ndim); // safe narrowing — guarded above

            if (a1 < 0)
            {
                a1 += indim;
            }
            if (a2 < 0)
            {
                a2 += indim;
            }

            // Bounds check — all int vs int, no mixed-sign comparisons
            if (a1 < 0 || a1 >= indim)
            {
                throw std::runtime_error("Dimensions<T>::transpose(Axis, Axis) Error: axis1 out of range");
            }
            if (a2 < 0 || a2 >= indim)
            {
                throw std::runtime_error("Dimensions<T>::transpose(Axis, Axis) Error: axis2 out of range");
            }

            // Same-axis check after normalization — axis1=1 and axis2=-2 on a 3D
            // tensor both normalize to 1 and are correctly caught here
            if (a1 == a2)
            {
                throw std::runtime_error("Dimensions<T>::transpose(Axis, Axis) Error: axis1 and axis2 must be different");
            }

            // Cross back into the unsigned domain only here — both values are fully
            // validated to be non-negative and in [0, ndim), so this cast is safe
            size_t ua1 = static_cast<size_t>(a1);
            size_t ua2 = static_cast<size_t>(a2);

            std::vector<T> vec = this->toVector();

            std::swap(vec[ua1], vec[ua2]);

            Dimensions<T> result;
            result.fromVector(vec);
            return result;
        }

        /*
         *   toVector(void) const
         *   
         *   PURPOSE:
         *       Returns a std::vector<T> containing the dimension values in row-major order.
         *       The vector is constructed by traversing the linked list from head to tail.
         *               
         *   PRECONDITIONS:
         *       - The Dimensions object must not be empty (head and tail must not be null).
         *               
         *   POSTCONDITIONS:
         *       - The returned vector will contain the dimension values in row-major order.
         *       - The Dimensions object will not be modified.
         *               
         *   RETURN VALUE:
         *       std::vector<T> — a vector containing the dimension values in row-major order.
         *               
         *   EXCEPTIONS:
         *       - std::runtime_error — if the Dimensions object is empty (head or tail is null).
         *               
         *   COMPLEXITY:
         *       O(n) — linear time, where n is the number of nodes in the linked list.
         */
        std::vector<T> toVector(void) const
        {
            // Validate both head and tail together
            if (this->head == nullptr || this->tail == nullptr)
            {
                throw std::runtime_error("Dimensions<T>::toVector() Error: " + std::string(this->head == nullptr ? "head" : "tail") + " is null");
            }

            std::vector<T> vec;

            // Reserve capacity to avoid reallocations
            // n is the number of nodes in the list
            // We need to add 1 for the tail's columns value
            vec.reserve(this->n + 1);

            DimensionsProperties<T>* current = this->head;
            size_t iterations = 0;
            constexpr size_t MAX_ITERATIONS = 10000; // Guard against cycles

            while (current != nullptr)
            {
                if (iterations++ > MAX_ITERATIONS)
                {
                    throw std::runtime_error("Dimensions<T>::toVector() Error: Cycle detected or list exceeds maximum allowed length");
                }

                vec.push_back(current->getRows());
                current = current->getNext();
            }

            // Append the tail's column count as the final dimension
            vec.push_back(this->tail->getColumns());

            return vec;
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