# DimensionsOfArray Class Documentation

## Overview
`DimensionsOfArray` is a reference-counted tensor shape management class designed for ML operations. It manages multidimensional array dimensions with efficient memory sharing through reference counting and provides both deep copy and shallow copy semantics.

## Class Structure

### Core Data Structure
```cpp
struct DimensionsOfArrayProperties {
    size_type *ptr;           // Pointer to dimension sizes array
    size_type n;              // Number of dimensions (rank)
    unsigned int reference_count; // Reference counting for memory sharing
}
```

## Public Methods

### Constructors

#### `DimensionsOfArray()` - Default Constructor
- Allocates properties structure with custom allocator
- Initializes ptr=NULL, n=0, reference_count=0
- Throws `ala_exception` on allocation failure
- **Purpose**: Creates empty tensor shape for later initialization

#### `DimensionsOfArray(DimensionsOfArray& other)` - Deep Copy Constructor
- Creates completely independent copy with separate memory allocation
- Allocates new properties structure and dimension array
- Copies all dimension values from source
- Sets reference_count=1 for new independent object
- **Purpose**: When you need isolated tensor shape that can be modified independently

#### `DimensionsOfArray(size_type* p, size_type nn, size_type rc=1)` - Parameterized Constructor
- Takes external pointer to dimension data
- Does NOT copy dimension data, just stores pointer
- Caller retains ownership of dimension array memory
- **Purpose**: Wrapping existing dimension arrays with reference counting

### Destructor

#### `~DimensionsOfArray()`
- Decrements reference count atomically
- If reference count reaches 0, deallocates both dimension array and properties
- Sets properties pointer to NULL for safety
- **Purpose**: Automatic cleanup with reference counting

### Reference Counting Methods

#### `incrementReferenceCount()`
- Safely increments reference count if properties exist
- **Purpose**: Manual reference management when needed

#### `decrementReferenceCount()`
- Decrements reference count and performs cleanup if count reaches 0
- Deallocates memory and nullifies pointers
- **Purpose**: Manual early cleanup or explicit reference release

### Access Methods

#### `operator[](size_type index) const`
- Provides bounds-checked read-only access to dimension values
- Throws `ala_exception` with detailed error message on invalid access
- **Purpose**: Safe access to individual tensor dimensions (e.g., batch size, sequence length)

#### `size() const`
- Returns number of dimensions (tensor rank)
- Returns 0 if uninitialized
- **Purpose**: Query tensor dimensionality (1D, 2D, 3D, 4D, etc.)

#### `get_n() const`
- Simple alias for `size()` method
- **Purpose**: Alternative interface for getting dimension count

### Assignment Operator

#### `operator=(DimensionsOfArray& other)`
- Implements reference sharing (shallow copy)
- Decrements current reference, assigns new properties, increments new reference
- Handles self-assignment correctly
- Throws exception if either object is uninitialized
- **Purpose**: Efficient tensor shape sharing between multiple tensors

### Comparison Methods

#### `compare(const DimensionsOfArray& other, AXIS axis=AXIS_COLUMN) const`
- Compares tensor shapes for compatibility
- AXIS_COLUMN: Compares all dimensions except the last (useful for matrix operations)
- Returns true if shapes are compatible for the specified operation
- **Purpose**: Validate tensor compatibility for operations like concatenation, matrix multiplication

### Utility Methods

#### `getNumberOfInnerArrays() const`
- Calculates total number of "rows" in multidimensional tensor
- Multiplies all dimensions except the last one
- Follows NumCy convention where arrays have at least 2 dimensions
- **Purpose**: Determine iteration count for tensor processing, memory layout calculations

## Design Patterns

### Memory Management Strategy
- **Copy Constructor**: Deep copy for independence
- **Assignment Operator**: Reference sharing for efficiency
- **Destructor**: Reference counting for automatic cleanup

### Use Case Examples
```cpp
// Independent modification
DimensionsOfArray base_shape;           // [batch, seq, hidden]
DimensionsOfArray modified_shape(base_shape);  // Deep copy

// Efficient sharing  
DimensionsOfArray shared_shape;
shared_shape = base_shape;              // Reference counting
```

### Error Handling
- All methods throw `ala_exception` with descriptive messages
- Comprehensive bounds checking and null pointer validation
- Exception safety with cleanup in constructors

## ML/Tensor Context
This class is specifically designed for:
- Transformer model tensor shape management
- Efficient shape sharing across multiple tensors
- Matrix multiplication compatibility checking
- Memory layout calculations for tensor operations
- Low-level performance optimization in ML inference