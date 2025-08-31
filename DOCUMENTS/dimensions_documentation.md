# Dimensions Class Documentation

## Overview

The `Dimensions` class implements a tensor/multi-dimensional array representation using a doubly-linked list structure. Each node in the linked list represents a 2D slice within a higher-dimensional tensor, enabling efficient representation and manipulation of multi-dimensional data structures.

## Core Structures

### DimensionsProperties

The internal structure that forms the linked list nodes:

**Private Properties:**
- `columns` - Width of a 2D slice (meaningful only in the last node)
- `rows` - Height of 2D slice / number of slices in current dimension  
- `next` - Pointer to next slice/dimension in tensor
- `prev` - Pointer to previous slice/dimension in tensor
- `reference_count` - Reference counting for shared ownership management

**Access Control:**
- All properties are private with `Dimensions` as a friend class
- Ensures encapsulation while allowing controlled access

## Class Properties

### Private Members
- `properties` - Pointer to the head of the DimensionsProperties linked list

### Public Interface
The class provides a comprehensive public API for tensor dimension management.

## Methods

### Constructors

#### Default Constructor
```cpp
Dimensions() throw (ala_exception)
```
Creates a single linked list node representing one 2D tensor slice with zero dimensions. Initializes all fields to default values and sets reference count to 1. Throws `ala_exception` on memory allocation failures, which are considered fatal system errors requiring immediate process termination.

#### Array Constructor  
```cpp
Dimensions(DIMENSIONSOFARRAY& dimensionsOfArray)
```
Constructs a complete linked list from an array of dimension sizes. The first element becomes the root node's rows, intermediate elements become subsequent nodes' rows, and the final element becomes the last node's columns. Validates that at least 2 dimensions are provided and throws `ala_exception` for invalid input or allocation failures.

#### Custom Constructor
```cpp
Dimensions(cc_tokenizer::string_character_traits<char>::size_type c, 
           cc_tokenizer::string_character_traits<char>::size_type r, 
           DIMENSIONSPROPERTIES_PTR n, 
           DIMENSIONSPROPERTIES_PTR p, 
           cc_tokenizer::string_character_traits<char>::size_type rc = 1)
```
Provides fine-grained control for creating individual nodes with explicit linkage. Allows direct specification of columns, rows, next/previous pointers, and reference count. **Critical responsibility**: Caller must manually maintain bidirectional linkage integrity by updating neighboring nodes' reciprocal pointers. Intended for advanced use cases like manual linked list construction, node insertion, or custom tensor building operations. The constructor trusts the caller completely and performs no validation of pointer relationships.

#### Copy Constructor
```cpp
Dimensions(const Dimensions& ref)
```
Creates a **deep, independent copy** of another Dimensions object using the internal `copy()` method. The resulting object is completely separate from the original and can be modified independently. This contrasts with the assignment operator's shallow copy behavior.

### Destructor
```cpp
~Dimensions()
```
Handles cleanup by delegating to the reference counting system. Calls `decrementReferenceCount()` which automatically deallocates nodes when their reference count reaches zero. This approach reuses existing logic and handles shared ownership scenarios properly.

### Memory Management Methods

#### incrementReferenceCount()
```cpp
void incrementReferenceCount(void)
```
Traverses the entire linked list and increments the reference count of each node by 1. Used when creating new references to shared dimensional structures. **Not thread-safe** - requires external synchronization for multi-threaded usage.

#### decrementReferenceCount()
```cpp
void decrementReferenceCount(void)
```
Decrements reference counts and immediately deallocates nodes reaching zero references. Maintains linked list structural integrity during node removal, handling edge cases like head node removal and complete list deletion. Features underflow protection and is **not thread-safe**.

### Utility Methods

#### copy()
```cpp
DIMENSIONSPROPERTIES_PTR copy(void) const throw (ala_exception)
```
Creates a deep copy of the entire linked list structure with separate memory allocation for each node. Extracts the dimensions array and reconstructs an identical but independent linked list. Returns a pointer to the root of the new copy.

#### getN()
```cpp
cc_tokenizer::string_character_traits<char>::size_type getN(void) const
```
Calculates the total number of elements in the multi-dimensional array by multiplying all dimensions together (rows of each node × final columns). Returns 0 for empty structures. **Thread-safe for concurrent reads**.

#### getNumberOfColumns()
```cpp
cc_tokenizer::string_character_traits<char>::size_type getNumberOfColumns(void) const
```
Returns the number of columns in the innermost dimension by traversing to the final node and returning its columns value. This represents the size of the innermost array dimension. Returns 0 for empty or malformed structures.

#### getNumberOfRows()
```cpp
cc_tokenizer::string_character_traits<char>::size_type getNumberOfRows(void) throw (ala_exception)
```
Calculates total number of rows by multiplying all dimensions except the last one (which represents columns). Uses `getDimensionsOfArray()` internally and relies on automatic cleanup via the DIMENSIONSOFARRAY destructor.

#### getNumberOfLinks()
```cpp
cc_tokenizer::string_character_traits<char>::size_type getNumberOfLinks(void) const
```
Counts the total number of nodes in the linked list structure. Returns 0 for empty structures. The total dimensions equal getNumberOfLinks() + 1 (accounting for the final columns value). **Thread-safe for concurrent reads**.

#### getDimensionsOfArray()
```cpp
DIMENSIONSOFARRAY getDimensionsOfArray(void) const throw (ala_exception)
```
Extracts dimensional information into a flat array structure. Each node's rows becomes one dimension, and the final node's columns becomes the last dimension. Allocates memory for the output array - **caller is responsible for deallocation**.

#### getReferenceCounts()
```cpp
cc_tokenizer::string_character_traits<char>::size_type* getReferenceCounts(void) const throw (ala_exception)
```
Returns an array containing the reference count of each node in the linked list. Allocates memory for the output array - **caller must deallocate**. Useful for debugging shared ownership scenarios.

### Operators

#### Assignment Operator
```cpp
Dimensions& operator=(const Dimensions& other)
```
Implements **intentional shared ownership semantics** (shallow copy). Releases current references, shares the source object's linked list, and increments reference counts. This creates a fundamentally different behavior from the copy constructor's deep copy approach.

#### Assignment Operator (Properties Pointer)
```cpp
Dimensions& operator=(DIMENSIONSPROPERTIES_PTR other)
```
Direct assignment from a DimensionsProperties pointer, enabling low-level control over the object's linked list reference. Performs self-assignment checking at the pointer level, releases current references via `decrementReferenceCount()`, then adopts the new pointer and increments its reference counts. This operator allows direct integration with externally managed DimensionsProperties structures and provides a bridge between raw pointer manipulation and the class's reference counting system. **Warning**: No validation is performed on the input pointer - caller must ensure it points to a valid, properly constructed linked list structure.

#### Index Operator
```cpp
cc_tokenizer::string_character_traits<char>::size_type operator[](cc_tokenizer::string_character_traits<char>::size_type index) const
```
Provides access to root node properties using predefined constants:
- `NUMCY_DIMENSIONS_SHAPE_ROWS` (0) - Returns rows value
- `NUMCY_DIMENSIONS_SHAPE_COLUMNS` (1) - Returns columns value

#### Equality Operator
```cpp
bool operator==(const Dimensions& other) const
```
Performs deep structural comparison with performance optimizations. Uses early exit strategies (total element count, link count) before detailed node-by-node comparison. **Thread-safe for concurrent reads** but not thread-safe if either object is being modified.

## Design Patterns & Characteristics

### Mixed Copy Semantics
- **Copy Constructor**: Deep copy (independent objects)
- **Assignment Operator**: Shallow copy (shared ownership)

### Reference Counting
Manual reference counting system for shared ownership management with automatic cleanup when references reach zero.

### Error Handling Philosophy
Critical errors (memory allocation, length violations) are considered fatal and require immediate process termination with no cleanup performed.

### Thread Safety
The class is **not thread-safe**. All methods accessing or modifying reference counts require external synchronization in multi-threaded environments.

## Typedefs

- `DIMENSIONS` - Alias for the Dimensions struct
- `DIMENSIONS_PTR` - Pointer to Dimensions object  
- `CONST_DIMENSIONS_PTR` - Pointer to const Dimensions object
- `DIMENSIONSPROPERTIES` - Alias for DimensionsProperties struct
- `DIMENSIONSPROPERTIES_PTR` - Pointer to DimensionsProperties object