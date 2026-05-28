# Collective — Numcy Tensor Container
### Design, Architecture, and Usage Guide

**Project:** Numcy — A C++ Tensor Library  
**Author:** Q@hackers.pk  
**Files:** `Collective.hh`, `CollectiveProperties.hh`

---

## Table of Contents

1. [What is Collective](#1-what-is-collective)
2. [Why Two Classes](#2-why-two-classes)
3. [Template Parameters](#3-template-parameters)
4. [Memory Layout](#4-memory-layout)
5. [Construction](#5-construction)
6. [Copying and Sharing](#6-copying-and-sharing)
7. [Assignment](#7-assignment)
8. [Destruction](#8-destruction)
9. [Element Access — operator[]](#9-element-access--operator)
10. [Shape Access — getShape()](#10-shape-access--getshape)
11. [Raw Data Access — getData()](#11-raw-data-access--getdata)
12. [Memory Location — getMemoryLocation()](#12-memory-location--getmemorylocation)
13. [Device Transfer — toDevice() and toHost()](#13-device-transfer--todevice-and-tohost)
14. [Transpose](#14-transpose)
15. [Full Usage Examples](#15-full-usage-examples)
16. [Design Decisions and Rationale](#16-design-decisions-and-rationale)
17. [Invariants](#17-invariants)
18. [What Collective Does Not Do](#18-what-collective-does-not-do)

---

## 1. What is Collective

`Collective<T, E>` is the primary tensor container in the Numcy library. It holds a block of typed data (`T*`) together with a shape description (`Dimensions<E>`), and manages the lifetime of both through reference counting.

A `Collective` object represents a tensor — a multi-dimensional array of elements of type `T`, whose shape is described by a linked list of `DimensionsProperties<E>` nodes accessible through the `Dimensions<E>` object.

Every tensor operation in `Numcy` takes `Collective` objects as arguments and returns `Collective` objects as results.

---

## 2. Why Two Classes

`Collective` is split into two classes:

```
Collective<T, E>                  — the handle the user holds
CollectiveProperties<T, E>        — the actual data and shape, heap allocated
```

This separation enables **shared ownership without copying data**. Multiple `Collective` handles can point to the same `CollectiveProperties` object. The data block and shape are allocated once and shared across all handles that refer to the same tensor. The `reference_count` inside `CollectiveProperties` tracks how many `Collective` handles currently share it — when the last handle is destroyed, `CollectiveProperties` is deleted and the data is freed.

This is the same two-level design used by PyTorch (`Tensor` / `TensorImpl`) and NumPy (`ndarray` / buffer), but implemented here entirely from scratch in C++ without `shared_ptr` — which is necessary for correctness at the CUDA boundary where `shared_ptr` cannot manage device memory.

---

## 3. Template Parameters

```cpp
template <typename T = double, typename E = size_t>
class Collective
```

| Parameter | Default | Purpose |
|---|---|---|
| `T` | `double` | Element type of the data array. Can be `float`, `double`, or any numeric type. For CUDA use, typically `float` or `double`. |
| `E` | `size_t` | Index and dimension type. Must be unsigned to avoid signed/unsigned comparison warnings under `-Wconversion`. Must match the type used by `Dimensions<E>` and its `numel()` return type. |

### Typical instantiations

```cpp
Collective<double>            // double data, size_t indices (default)
Collective<float>             // float data, size_t indices
Collective<double, size_t>    // explicit — same as first
```

---

## 4. Memory Layout

```
Stack / heap (user code)
    └──► Collective<T, E>
              └──► CollectiveProperties<T, E>*  properties
                        ├──► T* data              ← flat array on heap (or device)
                        │         [e0, e1, e2, e3, ..., eN]
                        │         row-major layout
                        ├──► size_t reference_count
                        ├──► MemoryLocation memory_location  ← Host or Device
                        └──► Dimensions<E>         ← value member inside CollectiveProperties
                                  ├──► head*
                                  └──► tail*
                                        └──► DimensionsProperties<E> nodes (linked list)
                                                  each node: rows, columns, next*, prev*, refcount
```

The data is stored as a **flat, row-major array**. For a 2D tensor of shape `(rows, columns)`, element at position `(i, j)` is at flat index `i * columns + j`.

`MemoryLocation` tracks whether `data` lives on the CPU heap (`Host`) or on CUDA device memory (`Device`). This governs which deallocation path is taken in the destructor (`delete[]` vs `cudaFree`), and determines whether `toDevice()` / `toHost()` need to perform a transfer or can return `*this` directly.

---

## 5. Construction

### Default constructor

```cpp
Collective(void)
```

Constructs a `Collective` with `properties` set to `nullptr`. This represents an empty, uninitialized handle. It is safe to assign to later. It is **not** safe to call `operator[]`, `getShape()`, `getData()`, or any other method that dereferences `properties` until a valid tensor is assigned.

```cpp
Collective<T, E> c;   // properties == nullptr — valid but empty
```

This constructor exists to support local variable declaration before conditional initialization, such as inside `toDevice()` and `toHost()`.

### From a Dimensions object (host allocation)

```cpp
Collective(const Dimensions<E>& d, MemoryLocation mem_loc = MemoryLocation::Host)
```

Allocates a new `CollectiveProperties` on the heap, which in turn allocates the flat data array with `new T[dimensions.numel()]`. The reference count starts at 1. The default `MemoryLocation` is `Host`.

```cpp
// Create a 2D tensor of shape 128 x 64 on the host
Dimensions<size_t> d(64, 128);      // columns=64, rows=128
Collective<double> tensor(d);       // allocates 128*64 doubles on host

// Create a scalar (1x1)
Dimensions<size_t> d(1, 1);
Collective<double> scalar(d);
```

### From a raw pointer (device or host)

```cpp
Collective(T* ptr, const Dimensions<E>& d, MemoryLocation mem_loc = MemoryLocation::Device)
```

Wraps an externally allocated data pointer in a new `CollectiveProperties`. The default `MemoryLocation` is `Device`, reflecting the primary use case of wrapping a `cudaMalloc`-allocated pointer. The `Collective` takes ownership — when the reference count reaches zero, the appropriate deallocation (`delete[]` or `cudaFree`) is called based on `mem_loc`.

```cpp
// Wrap a CUDA device pointer
T* d_ptr = nullptr;
cudaMalloc(&d_ptr, numel * sizeof(T));
Dimensions<size_t> d(cols, rows);
Collective<T, E> device_tensor(d_ptr, d, MemoryLocation::Device);
```

This constructor is also used internally by `toDevice()` and `toHost()` to construct the result collective after a `cudaMemcpy`.

---

## 6. Copying and Sharing

### Copy constructor

```cpp
Collective(const Collective<T, E>& other)
```

Copying a `Collective` does **not** copy the data. It shares the same `CollectiveProperties` and increments the reference count. Both the original and the copy point to the same memory block.

```cpp
Dimensions<size_t> d(64, 128);
Collective<double> a(d);      // refcount = 1
Collective<double> b = a;     // refcount = 2 — no data copied
Collective<double> c = a;     // refcount = 3 — no data copied
```

All three — `a`, `b`, `c` — share the same 8192 doubles. Modifying `a[0]` is visible through `b[0]` and `c[0]`.

If `other.properties` is `nullptr` (i.e., `other` was default-constructed and never assigned), `properties` is set to `nullptr` and no increment is performed.

This is the intended behaviour for tensor operations in a deep learning library. When `Numcy::matmul()` returns a `Collective`, the result is returned by value — the copy constructor fires, refcount increments, no data is duplicated.

### Destructor chain when sharing

```
c goes out of scope → refcount 3 → 2
b goes out of scope → refcount 2 → 1
a goes out of scope → refcount 1 → 0 → CollectiveProperties deleted → data freed
```

---

## 7. Assignment

```cpp
Collective<T, E>& operator=(const Collective<T, E>& other)
```

Assignment releases the current `CollectiveProperties` (decrementing its refcount and deleting it if it reaches zero), then acquires the new one (sharing the pointer and incrementing its refcount).

```cpp
Dimensions<size_t> d1(64, 128);
Dimensions<size_t> d2(32, 256);

Collective<double> a(d1);   // a → PropertiesA, refcount=1
Collective<double> b(d2);   // b → PropertiesB, refcount=1

b = a;
// PropertiesB refcount → 0 → deleted → data freed
// PropertiesA refcount → 2
// b now shares a's data
```

Self-assignment is guarded:

```cpp
a = a;  // safe — detected and skipped via (this != &other) check
```

Both `this->properties` and `other.properties` are null-guarded independently — assigning from or to a default-constructed `Collective` (where `properties == nullptr`) is safe.

---

## 8. Destruction

```cpp
~Collective()
```

Decrements the reference count of `properties`. If it reaches zero, `delete properties` is called, which triggers the full destructor chain. After decrement (regardless of whether deletion occurs), `this->properties` is set to `nullptr`.

```
~Collective()
    └──► if (this->properties != nullptr)
              ├──► this->properties->decrementReferenceCount()
              ├──► if (getReferenceCount() == 0)
              │         └──► delete properties
              │                    └──► ~CollectiveProperties()
              │                              ├──► if Host:   delete[] data
              │                              ├──► if Device: cudaFree(data)  [only if COMPILE_FOR_DEVICE]
              │                              └──► ~Dimensions<E>()    (automatic, value member)
              │                                        └──► release loop over DimensionsProperties nodes
              │                                                  └──► delete nodes where refcount → 0
              └──► this->properties = nullptr
```

No manual cleanup is required from user code.

---

## 9. Element Access — operator[]

Two versions are provided — one for read/write access, one for read-only access on `const` objects.

```cpp
T&       operator[](E index);        // non-const — read and write
const T& operator[](E index) const;  // const     — read only
```

Both check:
1. That `properties` is not `nullptr`
2. That `index` is within bounds (`index < numel()`)

```cpp
Dimensions<size_t> d(4, 3);    // 3 rows, 4 columns → 12 elements
Collective<double> t(d);

// Write
t[0] = 1.0;
t[5] = 3.14;
t[11] = 9.9;

// Read
double val = t[5];   // 3.14

// Out of bounds
t[12] = 0.0;   // throws std::runtime_error
```

### 2D indexing using flat index

Since the data is row-major, element at `(row, col)` maps to flat index `row * num_columns + col`:

```cpp
Dimensions<size_t> d(4, 3);   // 3 rows, 4 columns
Collective<double> m(d);

size_t rows = 3;
size_t cols = 4;

for (size_t i = 0; i < rows; i++)
{
    for (size_t j = 0; j < cols; j++)
    {
        m[i * cols + j] = static_cast<double>(i * cols + j);
    }
}

// Element at row=1, col=2
double val = m[1 * 4 + 2];   // index 6
```

---

## 10. Shape Access — getShape()

```cpp
const Dimensions<E>& getShape(void) const
```

Returns a `const` reference to the `Dimensions<E>` object inside `CollectiveProperties`. No copy is made. The reference is valid as long as the `Collective` object is alive. Throws `std::runtime_error` if `properties` is `nullptr`.

```cpp
Dimensions<size_t> d(64, 128);
Collective<double> t(d);

const Dimensions<size_t>& shape = t.getShape();

size_t total   = shape.numel();               // 128 * 64 = 8192
size_t rows    = shape.getNumberOfRows();     // 128
size_t columns = shape.getNumberOfColumns();  // 64
size_t n_nodes = shape.size();                // number of DimensionsProperties nodes
```

---

## 11. Raw Data Access — getData()

```cpp
T* getData(void) const
```

Returns the raw `T*` pointer to the underlying flat array. This pointer may be a host (`new`-allocated) or device (`cudaMalloc`-allocated) pointer — callers should check `getMemoryLocation()` before dereferencing it on the CPU.

The primary use of `getData()` is to pass the pointer to CUDA kernels in Numcy's operation implementations.

```cpp
T* ptr = tensor.getData();
// Use ptr in a CUDA kernel launch:
// some_kernel<<<blocks, threads>>>(ptr, numel);
```

Throws `std::runtime_error` if `properties` is `nullptr`.

---

## 12. Memory Location — getMemoryLocation()

```cpp
MemoryLocation getMemoryLocation(void) const
```

Returns the `MemoryLocation` enum value stored in `CollectiveProperties` — either `MemoryLocation::Host` or `MemoryLocation::Device`. This tells callers whether the pointer returned by `getData()` lives in CPU memory or CUDA device memory.

Throws `std::runtime_error` if `properties` is `nullptr`.

```cpp
if (tensor.getMemoryLocation() == MemoryLocation::Host)
{
    // Safe to access tensor[i] from CPU
}
else
{
    // Pointer is on GPU — pass to CUDA kernel only
}
```

---

## 13. Device Transfer — toDevice() and toHost()

### toDevice()

```cpp
Collective<T, E> toDevice(void)
```

Transfers the tensor from host memory to CUDA device memory. Returns a new `Collective` backed by a `cudaMalloc`-allocated buffer with `MemoryLocation::Device`. If the tensor is already on the device, returns `*this` (via the copy constructor — refcount incremented, no transfer performed).

The transfer uses `cudaMemcpy` with `cudaMemcpyHostToDevice`. This is handled by the CUDA runtime via DMA, bypassing GPU compute cores entirely — it is faster and simpler than a kernel for a raw copy.

Only compiled when `COMPILE_FOR_DEVICE` is defined. In CPU-only builds, `toDevice()` returns a default-constructed (empty) `Collective`.

```cpp
Collective<float> host_tensor(d);
// ... fill host_tensor ...

Collective<float> device_tensor = host_tensor.toDevice();
// device_tensor.getMemoryLocation() == MemoryLocation::Device
```

### toHost()

```cpp
Collective<T, E> toHost(void)
```

Transfers the tensor from CUDA device memory back to host memory. Returns a new `Collective` backed by a `new`-allocated buffer with `MemoryLocation::Host`. If the tensor is already on the host, returns `*this` (via the copy constructor — refcount incremented, no transfer performed).

The transfer uses `cudaMemcpy` with `cudaMemcpyDeviceToHost`.

Only compiled when `COMPILE_FOR_DEVICE` is defined. In CPU-only builds, `toHost()` returns a default-constructed (empty) `Collective`.

```cpp
Collective<float> device_tensor = host_tensor.toDevice();
// ... run kernels on device_tensor ...

Collective<float> result = device_tensor.toHost();
// result.getMemoryLocation() == MemoryLocation::Host
// result[i] is safe to access from CPU
```

Both methods throw `std::runtime_error` if `properties` or `getData()` returns `nullptr`.

---

## 14. Transpose

```cpp
Collective<T, E> transpose(numcy::Axis axis1 = numcy::Axis::Last,
                            numcy::Axis axis2 = numcy::Axis::SecondLast)
```

Returns a new `Collective` with its shape transposed along the two specified axes. The default swaps the last two axes, which corresponds to a standard matrix transpose.

The transposed `Collective` shares the same underlying `data` pointer as the original — the raw memory is not physically rearranged unless both axes are `Last` and `SecondLast`, in which case a physical transpose may be performed (currently stubbed, to be implemented via `Numcy::transpose`).

Shape transposition is delegated to `Dimensions<E>::transpose(axis1, axis2)`.

> **Note:** This method is under active development. Physical data rearrangement for the default axis pair is not yet implemented. Refer to `collective_data_model.docx` for the full design.

```cpp
Dimensions<size_t> d(64, 128);        // 128 rows, 64 columns
Collective<double> a(d);

Collective<double> at = a.transpose(); // shape becomes 64 rows, 128 columns
```

---

## 15. Full Usage Examples

### Example 1 — Allocate and fill a matrix

```cpp
// 3 rows, 4 columns matrix of doubles
Dimensions<size_t> d(4, 3);
Collective<double> m(d);

size_t rows = m.getShape().getNumberOfRows();
size_t cols = m.getShape().getNumberOfColumns();

for (size_t i = 0; i < rows; i++)
{
    for (size_t j = 0; j < cols; j++)
    {
        m[i * cols + j] = static_cast<double>(i * cols + j);
    }
}
```

### Example 2 — Shared tensors

```cpp
Dimensions<size_t> d(64, 128);
Collective<double> weights(d);

// Fill weights
for (size_t i = 0; i < weights.getShape().numel(); i++)
{
    weights[i] = 0.01 * static_cast<double>(i);
}

// Share weights — no data copied
Collective<double> shared_weights = weights;

// Modification through one handle is visible through the other
shared_weights[0] = 999.0;
// weights[0] is now also 999.0
```

### Example 3 — Passing to Numcy functions

All `Numcy` static methods take `Collective` by value or `const` reference and return `Collective` by value. The copy constructor handles sharing transparently:

```cpp
Dimensions<size_t> da(64, 128);
Dimensions<size_t> db(32, 64);

Collective<double> a(da);   // 128 x 64
Collective<double> b(db);   // 64  x 32

// matmul returns a new Collective — result shape is 128 x 32
Collective<double> c = Numcy::matmul(a, b);
```

### Example 4 — Assignment releases old data

```cpp
Dimensions<size_t> d1(10, 10);
Dimensions<size_t> d2(20, 20);

Collective<double> x(d1);   // 100 doubles allocated
Collective<double> y(d2);   // 400 doubles allocated

x = y;
// x's original 100-double block is freed
// x now shares y's 400-double block
// y's refcount = 2
```

### Example 5 — const Collective in Numcy methods

When a `Numcy` method takes a `const Collective<T,E>&`, the `const` version of `operator[]` and `getShape()` are called automatically:

```cpp
template <typename T, typename E>
static Collective<T, E> Numcy::sum(const Collective<T, E>& a)
{
    T total = T(0);

    for (E i = 0; i < a.getShape().numel(); i++)
    {
        total += a[i];   // calls const operator[]
    }

    // ... return result
}
```

### Example 6 — Host to device round-trip

```cpp
Dimensions<size_t> d(64, 128);
Collective<float> host_tensor(d);

// Fill on host
for (size_t i = 0; i < host_tensor.getShape().numel(); i++)
{
    host_tensor[i] = static_cast<float>(i);
}

// Upload to GPU
Collective<float> device_tensor = host_tensor.toDevice();

// ... pass device_tensor.getData() to CUDA kernels ...

// Download result back to host
Collective<float> result = device_tensor.toHost();

// result[i] is now accessible from CPU
```

### Example 7 — Default-constructed Collective used for conditional initialization

```cpp
Collective<T, E> device_collective;  // properties == nullptr

#ifdef COMPILE_FOR_DEVICE
    // ... cudaMalloc, cudaMemcpy ...
    device_collective = Collective<T, E>(ptr, this->getShape(), MemoryLocation::Device);
#endif

return device_collective;
```

---

## 16. Design Decisions and Rationale

### Why manual reference counting instead of shared_ptr

`std::shared_ptr` cannot manage CUDA device memory. When `data` is a `cudaMalloc`-allocated pointer, the deleter must be `cudaFree`, not `delete[]`. Manual reference counting gives full control over what happens at refcount zero — the destructor calls `delete[]` for `MemoryLocation::Host` and `cudaFree` for `MemoryLocation::Device` (when `COMPILE_FOR_DEVICE` is defined), without changing anything else in the architecture.

### Why there is a default constructor

The old design had no default constructor. The current design adds one (`properties = nullptr`) to support conditional initialization patterns that arise naturally in `toDevice()` and `toHost()` — both methods need to declare a local `Collective` variable before a `#ifdef COMPILE_FOR_DEVICE` block that conditionally constructs the real value. Without a default constructor, such a pattern requires awkward restructuring.

The default-constructed state (`properties == nullptr`) is explicitly safe: all methods that dereference `properties` check for `nullptr` and throw before doing so.

### Why CollectiveProperties is not assignable

`CollectiveProperties` has `operator=` explicitly deleted. Assignment semantics for `CollectiveProperties` would require either a deep copy (which defeats the purpose of sharing) or careful refcount management (which is already handled by `Collective`'s `operator=`). Since `Collective` is the intended handle class, `CollectiveProperties` assignment is never needed and is disabled to prevent accidental misuse.

### Why getShape() returns const reference

`getShape()` returns `const Dimensions<E>&` — a reference to the `Dimensions` value member inside `CollectiveProperties`. This avoids a copy of the `Dimensions` object (which would walk the entire `DimensionsProperties` linked list and increment every node's refcount, then decrement them again when the temporary is destroyed). Since callers only need to read the shape, a `const` reference is sufficient and significantly cheaper.

### Why data is row-major

Row-major layout matches C array semantics, NumPy default layout, and CUDA memory access patterns for row-wise operations. CUDA coalesced memory access requires consecutive threads to access consecutive memory addresses — row-major layout achieves this naturally for row-parallel operations.

### Why toDevice() and toHost() use cudaMemcpy instead of a kernel

`cudaMemcpy` with `cudaMemcpyHostToDevice` / `cudaMemcpyDeviceToHost` uses DMA internally, bypassing the GPU's compute cores entirely. It is faster than a kernel for a raw copy because a kernel would need to launch threads, schedule warps, and go through the GPU pipeline just to read and write memory. A kernel would only be warranted if a computation needed to happen *during* the transfer (e.g., type promotion from `float16` on host to `float32` on device).

---

## 17. Invariants

The following invariants must hold at all times:

| Invariant | Description |
|---|---|
| `properties` is valid | `properties` is either `nullptr` or points to a live `CollectiveProperties` with `reference_count >= 1` |
| refcount matches holders | `reference_count` inside `CollectiveProperties` equals the number of `Collective` objects currently pointing to it |
| data freed exactly once | `data` is freed if and only if `reference_count` reaches zero |
| correct deallocation path | `delete[]` is used for `MemoryLocation::Host`; `cudaFree` is used for `MemoryLocation::Device` |
| shape consistent with data | `dimensions.numel()` equals the number of elements allocated in `data[]` |
| bounds always checked | `operator[]` always verifies `index < numel()` before accessing `data` |
| nullptr methods throw | Any public method that dereferences `properties` checks for `nullptr` first and throws `std::runtime_error` |

---

## 18. What Collective Does Not Do

`Collective` is intentionally minimal. It does not:

- Perform arithmetic — that is `Numcy`'s responsibility
- Manage strides — currently assumes contiguous, row-major layout
- Support slicing that shares data — planned for future with offset and stride fields in `CollectiveProperties`
- Provide move semantics — move constructor and move assignment are not yet implemented; copy semantics with reference counting serve the same purpose at low cost
- Validate shape consistency with data on construction from external pointer — the caller is responsible for passing a shape whose `numel()` matches the allocation size of `ptr`
- Perform physical data rearrangement in `transpose()` — the current implementation shares the data pointer and only reorders the shape description; physical rearrangement is planned

---

*This document covers `Collective.hh` and `CollectiveProperties.hh` as of the current development revision. Tensor operations are documented separately in `Numcy.md`.*
