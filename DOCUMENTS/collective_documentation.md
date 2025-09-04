# Collective Class Documentation

## Overview

The `Collective` class is a template-based container that represents a collection of array elements of type `E` along with their dimensional shape. It combines data storage (via a pointer to array elements) with structural information (via a `Dimensions` object) to create a complete tensor-like data structure. The class uses reference counting for memory management and shared ownership semantics.

## Template Parameters

- `E` - Element type (defaults to `double`)
  - Can be any type suitable for array storage
  - Common usage: `Collective<double>`, `Collective<int>`, `Collective<float>`

## Core Structures

### CollectiveProperties<E>

The internal properties structure that holds the actual data and metadata:

**Private Properties:**
- `ptr` - Pointer to the array of elements of type E
- `shape` - Pointer to a DIMENSIONS object describing the tensor structure  
- `reference_count` - Reference counting for shared ownership management

**Access Control:**
- All properties are private with `Collective<E>` as a friend class
- Ensures encapsulation while allowing controlled access

**Constructors:**
- **Default Constructor**: Initializes with NULL pointer, empty shape (0,0), and default reference count
- **Data Constructor**: Takes an element pointer and dimensions object, sets up proper shape sharing

## Class Properties

### Public Members
- `properties` - Pointer to CollectiveProperties<E> structure containing data and metadata

## Methods

### Constructors

#### Default Constructor
```cpp
Collective(void)
```
Creates an empty Collective object with no data allocation. Initializes the properties structure with NULL data pointer, empty dimensions (0,0), and default reference count. Propagates `ala_exception` from CollectiveProperties constructor if memory allocation fails.

#### Data Constructor
```cpp
Collective(E* v, DIMENSIONS& like) throw (ala_exception)
```
Creates a Collective object from an existing array pointer and dimensions specification. Handles multiple scenarios:

**Use Cases:**
- **Valid Data + Valid Shape**: `v != NULL` and `like.getN() > 0` - Creates fully functional collective
- **NULL Data + Valid Shape**: `v == NULL` but `like.getN() > 0` - Creates shape-only placeholder (useful for deferred allocation)
- **Invalid Shape**: `like.getN() == 0` - Falls back to default empty initialization

**Special Features:**
- Supports placeholder creation with defined shape but no data allocation
- Enables deferred memory allocation patterns
- Properly manages dimension reference counting

### Memory Management Methods

#### incrementReferenceCount()
```cpp
void incrementReferenceCount(void)
```
Increments reference counts for both the properties structure and the associated shape dimensions. Called when creating new references to shared collective structures. Includes null pointer checking for safe operation on uninitialized objects.

#### decrementReferenceCount()
```cpp
void decrementReferenceCount(void)
```
Decrements reference counts and performs cleanup when counts reach zero. Features comprehensive cleanup logic:

**Cleanup Process:**
- Decrements shape dimension reference count
- Decrements properties reference count with underflow protection
- When properties reference count reaches 0:
  - Deallocates element array using `cc_tokenizer::allocator<E>()`
  - Deallocates properties structure using `cc_tokenizer::allocator<char>()`
  - Maintains proper cleanup order to prevent memory leaks

### Accessor Methods

#### getShape()
```cpp
DIMENSIONS getShape(void) const throw (ala_exception)
```
Returns a copy of the dimensions object describing the collective's structure. Includes null pointer validation and returns an empty dimensions object (0,0) for uninitialized collectives. This method provides safe access to shape information without exposing internal pointers.

#### getReferenceCount()
```cpp
cc_tokenizer::string_character_traits<char>::size_type getReferenceCount(void) const
```
Returns the current reference count of the properties structure. Returns 0 for uninitialized objects (properties == NULL). Useful for debugging shared ownership scenarios and understanding object lifecycle.

### Destructor
```cpp
~Collective()
```
Handles cleanup by delegating to the reference counting system. Simply calls `decrementReferenceCount()` which automatically handles deallocation when reference counts reach zero. This approach ensures consistent cleanup behavior and proper handling of shared ownership.

### Operators

#### Assignment Operator
```cpp
Collective<E>& operator=(const Collective<E>& other) throw (ala_exception)
```
Implements **shared ownership semantics** similar to the Dimensions class assignment pattern. 

**Behavior:**
- Performs self-assignment checking at the object level
- Releases current references via `decrementReferenceCount()`
- Adopts the source object's properties pointer (shallow copy)
- Increments reference counts to reflect shared ownership
- Multiple objects now share the same underlying data and shape

**Thread Safety:** Not thread-safe - requires external synchronization for multi-threaded usage.

#### Index Operator
```cpp
E& operator[](cc_tokenizer::string_character_traits<char>::size_type index) throw (ala_exception)
```
Provides direct access to individual elements in the underlying array.

**Validation:**
- Checks if properties are initialized (throws exception if NULL)
- Validates that data pointer is non-NULL (throws exception for empty arrays)
- Performs bounds checking against shape dimensions (throws exception for out-of-range access)

**Return:** Reference to the element at the specified index, enabling both read and write operations.

## Design Patterns & Characteristics

### Reference Counting
Manual reference counting system manages both the properties structure and the associated dimensions object, ensuring coordinated cleanup of related resources.

### Shared Ownership
Assignment operations create shared ownership relationships where multiple Collective objects can reference the same underlying data and shape information.

### Template-Based Design
Generic template allows the same container structure to work with different element types while maintaining type safety.

### Exception Safety
Comprehensive validation in critical methods with detailed error messages. Follows the same fatal error philosophy as the Dimensions class for critical memory operations.

### Null Pointer Safety
Most methods include null pointer checking to handle uninitialized or corrupted objects gracefully.

## Key Features

### Deferred Allocation Support
The constructor supports creating shape-only objects (NULL data pointer with valid dimensions), enabling patterns where structure is defined before data allocation.

### Integrated Shape Management
Tight integration with the Dimensions class provides comprehensive multi-dimensional array support with proper reference counting across both data and shape.

### Type Safety
Template-based design ensures compile-time type checking while maintaining generic container capabilities.

## Thread Safety
The class is **not thread-safe**. All reference counting operations and data access require external synchronization in multi-threaded environments.

## Usage Examples

```cpp
// Create empty collective
Collective<double> empty_collective;

// Create with data and shape  
double* data = new double[12];
DIMENSIONS shape(3, 4, NULL, NULL);  // 3x4 matrix
Collective<double> matrix(data, shape);

// Create shape-only placeholder
Collective<double> placeholder(NULL, shape);

// Shared ownership via assignment
Collective<double> shared_matrix;
shared_matrix = matrix;  // Now shares same data
```

## Typedefs
- `CollectiveProperties<E>` - Internal properties structure template
- Template instantiations create type-specific versions (e.g., `CollectiveProperties<double>`)

## Dependencies
- Requires the Dimensions class for shape management
- Uses cc_tokenizer allocator system for memory management
- Depends on ala_exception for error handling