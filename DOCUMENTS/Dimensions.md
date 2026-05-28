# Dimensions\<T\>

**File:** `Numcy/Dimensions.hh`  
**Project:** Numcy — A C++ Tensor Library  
**Author:** Q@hackers.pk

---

## Overview

`Dimensions<T>` is a template class that represents the shape of a multi-dimensional tensor. Internally it is a **doubly linked list** of `DimensionsProperties<T>` nodes, where each node carries a `rows` and `columns` value.

The default template parameter is `size_t`, but the class works with any numeric type including `int64_t` for CUDA API compatibility.

---

## The Core Invariant

This is the most important concept in the class. Every method depends on it:

- **All intermediate nodes** (every node except the tail) have `columns = 0`.
- **The tail node** always has a non-zero `columns` value.
- **Every node** (including intermediate nodes) always has a non-zero `rows` value.

This invariant is established and enforced exclusively by `append()`. Callers do not need to manage it.

### Example — shape `{2, 4, 8}` stored internally:

```
[rows=2, cols=0]  →  [rows=4, cols=8]
     head                   tail
```

### Example — shape `{2, 4, 8, 16}` stored internally:

```
[rows=2, cols=0]  →  [rows=4, cols=0]  →  [rows=8, cols=16]
     head                                        tail
```

---

## Template Parameter

```cpp
template <typename T = size_t>
class Dimensions;
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `T` | `size_t` | The numeric type used for dimension values. Use `size_t` for general use, `int64_t` for CUDA APIs. |

---

## Why Each `Dimensions` Instance Keeps Its Own `n`

Each `Dimensions` object maintains its own node count (`n`) independently, because different instances can share the same head node but have different tail nodes — each one seeing a different length of the same underlying list.

Consider:

```cpp
Dimensions<T> d1(10, 20);
// d1: head──►[A], tail──►[A], n=1

Dimensions<T> d2(d1);
// d2: head──►[A], tail──►[A], n=1 — shares node A with d1
// A.refcount = 2

d2.append(30, 40);
// d2: head──►[A]──►[B], tail──►[B], n=2
// A.refcount=2, B.refcount=1

d2.append(50, 60);
// d2: head──►[A]──►[B]──►[C], tail──►[C], n=3

Dimensions<T> d3(d2);
// d3: head──►[A]──►[B]──►[C], tail──►[C], n=3
// A.refcount=3, B.refcount=2, C.refcount=2

d3.append(70, 80);
// d3: head──►[A]──►[B]──►[C]──►[D], tail──►[D], n=4
```

After all of the above: `d1.n = 1`, `d2.n = 3`, `d3.n = 4` — each is correct and independent.

`n` cannot live inside `DimensionsProperties` because all `Dimensions` objects sharing a node would increment and decrement the same counter, making it impossible to track the length of any individual object's list independently. `n` belongs in `Dimensions`, not in `DimensionsProperties`.

---

## Constructors

### Default Constructor

```cpp
Dimensions(void)
```

Creates an empty `Dimensions` object. `head` and `tail` are `nullptr`, `n` is `0`.

```cpp
Dimensions<> d;  // empty, n = 0
```

---

### Two-Parameter Constructor

```cpp
Dimensions(T columns, T rows)
```

Creates a single-node list representing a 2D matrix. Internally delegates to `append()` so allocation errors are caught and reported with context.

```cpp
Dimensions<size_t> d(8, 4);  // 4 rows, 8 columns — a single 2D slice
```

---

### Pointer Constructor

```cpp
Dimensions(DimensionsProperties<T>* h, DimensionsProperties<T>* t)
```

Constructs a `Dimensions` object from an existing linked list by taking shared ownership. Walks the list from `h` forward, counting nodes into `n` and incrementing the reference count of every node encountered.

> **Note:** Use with care. The caller must ensure `h` and `t` are consistent — both null or both non-null, with `t` reachable from `h`.

---

### Copy Constructor

```cpp
Dimensions(const Dimensions<T>& other)
```

Shares ownership of the underlying nodes with `other`. Copies `head`, `tail`, and `n` from `other`, then walks the list and increments the reference count of every node. No deep copy is performed — nodes are shared.

```cpp
Dimensions<> a(8, 4);
Dimensions<> b(a);  // b shares nodes with a — a's node refcount becomes 2
```

---

## Destructor

```cpp
~Dimensions(void)
```

Walks the list from `head` forward. For each node, decrements its reference count. If the count reaches zero, the node is deleted and its neighbors are relinked (`next->setPrev(prev)`, `prev->setNext(next)`). After the release loop, `head`, `tail`, and `n` are all set to `nullptr` / `0`.

This selective deletion allows mixed-ownership lists — a middle node can be freed while its neighbors (owned by other `Dimensions` objects) survive intact.

---

## Operator Overloading

### Assignment Operator

```cpp
Dimensions<T>& operator=(const Dimensions<T>& rhs)
```

Self-assignment is checked first and returns `*this` immediately. Otherwise:

1. Nulls out `this->tail` before the release loop to prevent a dangling pointer if the tail node is deleted.
2. Runs the release loop over the current nodes — decrementing reference counts and deleting/relinking any that reach zero.
3. Assigns `head`, `tail`, and `n` from `rhs`.
4. Walks the new list and increments the reference count of every node.

```cpp
Dimensions<> a(8, 4);
Dimensions<> b;
b = a;  // b now shares a's nodes; node refcount = 2
```

---

## Public Methods

### `append(T c, T r)`

```cpp
void append(T c, T r)
```

Appends a new 2D slice node to the end of the dimensions list. This is the **primary method for building tensor shapes**. An `assert` at entry checks that `head` and `tail` are consistent (both null or both non-null) before any pointer work.

**Parameters:**
- `c` — columns value of the new node
- `r` — rows value of the new node

**Behavior:**
- If the list is empty, the new node becomes both `head` and `tail`.
- If the current tail has a non-zero `columns` value (which would violate the invariant if a further append followed), the method first zeroes out the tail's `columns`, then recursively calls `append(T(0), column)` to insert an intermediate boundary node that carries the old `columns` as its `rows`. The recursion is **exactly one level deep** — the recursive call always passes `T(0)` as `c`, so the boundary condition cannot fire again.
- `n` is incremented once per call (the recursive intermediate node call handles its own `n++`).

**Exceptions:** `std::runtime_error` on `std::bad_alloc` or any other allocation failure, with the original message preserved in the chain.

**Complexity:** O(1)

```cpp
Dimensions<> d;
d.append(T(0), 2);  // intermediate node: rows=2, cols=0
d.append(8, 4);     // tail node: rows=4, cols=8
// shape: {2, 4, 8}
```

---

### `fromVector(const std::vector<T>& vec)`

```cpp
void fromVector(const std::vector<T>& vec)
```

Populates the `Dimensions` object from a `std::vector<T>`. All validation is performed before any node allocation.

**Vector layout:** `{dim_0, dim_1, ..., dim_{n-2}, columns}`
The last element becomes the tail's `columns` value. All preceding elements become `rows` values of intermediate and tail nodes.

**Preconditions:**
- Vector size must be ≥ 2.
- No element may be zero (checked with index reported in the error message).
- The object may already be populated — calling `fromVector()` on a non-empty object appends to the existing list. This is intentional and the caller's responsibility to manage.

**Implementation detail:** Iterates `vec[0]` through `vec[n-3]` calling `append(T(0), vec[i])` for intermediate nodes, then calls `append(vec[n-1], vec[n-2])` for the tail node.

**Exceptions:** `std::runtime_error` for size < 2, any zero value (with index reported), or any allocation failure from `append()`.

**Complexity:** O(n)

```cpp
Dimensions<> d;
d.fromVector({2, 4, 8});
// Internally: [rows=2, cols=0] → [rows=4, cols=8]
```

---

### `getNumberOfColumns(void) const`

```cpp
T getNumberOfColumns(void) const
```

Returns the `columns` value of the tail node — the width of the tensor's last 2D slice. By the invariant, this is the only node with a non-zero `columns` value, and it is always the final column dimension of the tensor.

Returns `T(0)` if the list is empty (`tail == nullptr`). Does not throw.

**Complexity:** O(1) — direct pointer access to tail.

```cpp
Dimensions<size_t> d(64, 128);
size_t cols = d.getNumberOfColumns();  // 64
```

---

### `getNumberOfRows(void) const`

```cpp
T getNumberOfRows(void) const
```

Returns the **total logical number of rows** in the tensor — the product of every node's `rows` field across the entire list. For a 2D matrix this is simply the number of rows. For an nD tensor this is the product of all dimensions except the final column count.

Returns `T(0)` if the list is empty. Uses `size_t` internally for intermediate products to prevent overflow, then casts back to `T` for the return value (satisfying `-Wconversion`). Includes a `MAX_ITERATIONS` guard against cycles.

**Complexity:** O(n)

```cpp
Dimensions<size_t> d;
d.fromVector({2, 4, 8});
size_t rows = d.getNumberOfRows();  // 2 * 4 = 8
```

---

### `numel(void) const`

```cpp
size_t numel(void) const
```

Returns the **total number of elements** in the tensor — the product of all dimension values. Equivalent to NumPy's `ndarray.size` or PyTorch's `tensor.numel()`.

An `assert` at entry enforces the head/tail consistency invariant: `(head != nullptr) == (tail != nullptr)`. The program terminates immediately if this is violated, as it indicates memory corruption.

Computed as:
```
total = tail->getColumns() × node_1->getRows() × node_2->getRows() × ... × tail->getRows()
```

All intermediate arithmetic is done in `size_t` via `static_cast` to avoid signed/unsigned conversion warnings.

Returns `0` if the list is empty.

**Complexity:** O(n)

```cpp
Dimensions<> d;
d.fromVector({2, 4, 8});
size_t total = d.numel();  // 2 * 4 * 8 = 64
```

---

### `reshape(const std::vector<T>& newShape)`

```cpp
void reshape(const std::vector<T>& newShape)
```

Performs an in-place dimensional transformation of the metadata — a **zero-copy** operation on the tensor data. Only the linked-list nodes defining the shape are touched.

**Algorithm (Release-and-Transfer):**

1. Builds `newDimensions` from `newShape` via `fromVector()`.
2. Validates that `newDimensions.numel() == this->numel()`. Throws `std::runtime_error` if not.
3. Runs the release loop over the existing nodes, decrementing reference counts and deleting/relinking any that reach zero.
4. Transfers ownership: assigns `this->head`, `this->tail`, and `this->n` from `newDimensions`.
5. Neutralizes `newDimensions` by setting its `head`, `tail`, and `n` to `nullptr`/`0` — preventing its destructor from double-freeing the nodes now owned by `this`. This is an explicit manual move-semantic pattern.

**Complexity:**
- Time: O(M + K), where M is the old dimensionality and K is the new dimensionality.
- Space: O(K) for the new `DimensionsProperties` nodes.
- Data impact: O(1) — the actual tensor data on host or device is never touched.

```cpp
Dimensions<size_t> d;
d.fromVector({2, 5, 10}); // numel = 100

d.reshape({10, 10});      // numel = 100 — succeeds

try {
    d.reshape({3, 33});   // numel = 99 — throws
} catch (const std::runtime_error& e) {
    // "Dimensions<T>::reshape(...) Error: Number of elements must match"
}
```

---

### `size(void) const`

```cpp
size_t size(void) const
```

Returns the **number of nodes** in the linked list — the value of `n`. This is the node count, not the total element count.

Returns `0` if the list is empty. Does not throw.

**Complexity:** O(1)

```cpp
Dimensions<> d;
d.fromVector({2, 4, 8});
size_t nodes = d.size();  // 2 (two nodes in the list)
```

---

### `transpose(numcy::Axis axis1, numcy::Axis axis2) const`

```cpp
Dimensions<T> transpose(numcy::Axis axis1 = numcy::Axis::Last,
                         numcy::Axis axis2 = numcy::Axis::SecondLast) const
```

Returns a **new** `Dimensions` object with the two specified axes swapped. The original object is not modified. The default arguments swap the last two axes — the standard matrix transpose.

**Algorithm:**

1. Checks that the object is non-empty; throws if not.
2. Computes `ndim = this->size() + 1` (node count plus the final column dimension).
3. Guards against `ndim > INT_MAX` to make the signed narrowing cast safe.
4. Resolves negative axis indices (`a += ndim`) to their positive equivalents.
5. Bounds-checks both axes after normalization; throws if either is out of range.
6. Checks that `axis1 != axis2` after normalization (e.g., `axis1=1` and `axis2=-2` on a 3D tensor both normalize to `1` and are correctly caught here).
7. Calls `this->toVector()`, swaps `vec[ua1]` and `vec[ua2]`, builds a new `Dimensions` via `fromVector(vec)`, and returns it.

**Axes:** Uses the `numcy::Axis` enum. Negative values follow Python-style negative indexing — `Axis::Last` is `-1` (last dimension), `Axis::SecondLast` is `-2`.

**Exceptions:** `std::runtime_error` if the object is empty, if either axis is out of range after normalization, if the two axes are the same, or if `ndim` exceeds `INT_MAX`.

**Complexity:** O(n)

```cpp
Dimensions<size_t> d;
d.fromVector({128, 64});      // 128 rows, 64 columns

Dimensions<size_t> dt = d.transpose();
// dt: 64 rows, 128 columns (Last and SecondLast swapped)
```

---

### `toVector(void) const`

```cpp
std::vector<T> toVector(void) const
```

Reconstructs the original shape vector from the linked list. Walks from `head` to `tail`, collecting each node's `rows` value, then appends the tail's `columns` as the final element.

The returned vector contains `n + 1` elements, where `n` is the node count returned by `size()`.

**Preconditions:** The list must not be empty (`head` and `tail` must not be `nullptr`).

**Exceptions:** `std::runtime_error` if `head` or `tail` is null (with which pointer is null reported), or if the list exceeds 10,000 nodes (cycle guard).

**Complexity:** O(n). Capacity is pre-reserved to `n + 1` to avoid reallocations.

```cpp
Dimensions<> d;
d.fromVector({2, 4, 8});
std::vector<size_t> shape = d.toVector();
// shape = {2, 4, 8}
```

---

## `size()` vs `numel()` — Key Distinction

| Method | Returns | Example for shape `{2, 4, 8}` |
|--------|---------|-------------------------------|
| `size()` | Number of nodes in the list | `2` |
| `numel()` | Total number of tensor elements | `64` |

---

## `n` Maintenance — How Node Count Is Tracked

`n` is incremented exclusively inside `append()` — once per call. No other method touches `n` directly (except resetting to `0` in the destructor and `operator=`, and the pointer-steal in `reshape()`).

| Scenario | `n` change |
|----------|------------|
| Normal `append()` — no intermediate node needed | `+1` |
| `append()` with automatic intermediate boundary node | `+2` (recursive call `+1`, outer call `+1`) |
| `fromVector()` | Delegates entirely to `append()` |
| Destructor | Reset to `0` |
| `operator=` release side | Reset to `0`, then assigned `rhs.n` |
| `reshape()` pointer steal | Assigned from `newDimensions.n`, then `newDimensions.n` set to `0` |

---

## Complete Usage Examples

### Example 1 — 2D Matrix (single node)

```cpp
#include "Numcy/Dimensions.hh"

int main()
{
    // 4 rows, 8 columns
    Dimensions<size_t> d(8, 4);

    std::cout << "nodes : " << d.size()  << "\n";  // 1
    std::cout << "numel : " << d.numel() << "\n";  // 32

    auto shape = d.toVector();
    // shape = {4, 8}

    return 0;
}
```

---

### Example 2 — 3D Tensor via `append()`

```cpp
#include "Numcy/Dimensions.hh"

int main()
{
    Dimensions<size_t> d;
    d.append(size_t(0), 2);  // intermediate node: rows=2, cols=0
    d.append(8, 4);          // tail node: rows=4, cols=8

    // Internal list: [rows=2, cols=0] → [rows=4, cols=8]

    std::cout << "nodes : " << d.size()  << "\n";  // 2
    std::cout << "numel : " << d.numel() << "\n";  // 64

    auto shape = d.toVector();
    // shape = {2, 4, 8}

    return 0;
}
```

---

### Example 3 — nD Tensor via `fromVector()`

```cpp
#include "Numcy/Dimensions.hh"
#include <vector>

int main()
{
    Dimensions<size_t> d;
    d.fromVector({2, 4, 8, 16});

    // Internal list:
    // [rows=2, cols=0] → [rows=4, cols=0] → [rows=8, cols=16]

    std::cout << "nodes : " << d.size()  << "\n";  // 3
    std::cout << "numel : " << d.numel() << "\n";  // 1024

    auto shape = d.toVector();
    // shape = {2, 4, 8, 16}

    return 0;
}
```

---

### Example 4 — Copy and Assignment

```cpp
#include "Numcy/Dimensions.hh"

int main()
{
    Dimensions<size_t> a;
    a.fromVector({2, 4, 8});

    // Copy constructor — shares nodes with a
    Dimensions<size_t> b(a);

    // Assignment operator — b2 releases its nodes, shares a's
    Dimensions<size_t> b2;
    b2 = a;

    std::cout << b.numel()  << "\n";  // 64
    std::cout << b2.numel() << "\n";  // 64

    return 0;
}
```

---

### Example 5 — CUDA-compatible `int64_t`

```cpp
#include "Numcy/Dimensions.hh"
#include <cstdint>

int main()
{
    Dimensions<int64_t> d;
    d.fromVector({32, 512, 768});  // batch=32, seq_len=512, hidden=768

    std::cout << "numel : " << d.numel() << "\n";  // 12,582,912

    auto shape = d.toVector();
    // shape = {32, 512, 768}
    // Pass shape.data() directly to CUDA APIs expecting int64_t*

    return 0;
}
```

---

### Example 6 — reshape()

```cpp
#include "Numcy/Dimensions.hh"

int main()
{
    Dimensions<size_t> d;
    d.fromVector({2, 5, 10});  // numel = 100

    d.reshape({10, 10});       // numel = 100 — succeeds
    auto shape = d.toVector(); // shape = {10, 10}

    try
    {
        d.reshape({3, 33});    // numel = 99 — throws
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << "\n";
        // Dimensions<T>::reshape(...) Error: Number of elements must match
    }

    return 0;
}
```

---

### Example 7 — transpose()

```cpp
#include "Numcy/Dimensions.hh"

int main()
{
    Dimensions<size_t> d;
    d.fromVector({128, 64});  // 128 rows, 64 columns

    // Default: swap Last and SecondLast axes
    Dimensions<size_t> dt = d.transpose();
    auto shape = dt.toVector();  // shape = {64, 128}

    // Explicit axes on a 3D tensor
    Dimensions<size_t> d3;
    d3.fromVector({2, 4, 8});  // ndim = 3

    Dimensions<size_t> dt3 = d3.transpose(numcy::Axis::Last, numcy::Axis::SecondLast);
    auto shape3 = dt3.toVector();  // shape = {2, 8, 4}

    return 0;
}
```

---

### Example 8 — Exception Handling

```cpp
#include "Numcy/Dimensions.hh"
#include <stdexcept>
#include <iostream>

int main()
{
    // Too few elements
    try
    {
        Dimensions<size_t> d;
        d.fromVector({8});  // throws — needs at least 2 elements
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << "\n";
        // Dimensions<T>::fromVector(std::vector<T>) Error: vector size must be at least 2
    }

    // Zero dimension value
    try
    {
        Dimensions<size_t> d;
        d.fromVector({2, 0, 8});  // throws — zero at index 1
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << "\n";
        // Dimensions<T>::fromVector(std::vector<T>) Error: zero value at index 1
    }

    // toVector on empty object
    try
    {
        Dimensions<size_t> d;
        auto shape = d.toVector();  // throws — head is null
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << "\n";
        // Dimensions<T>::toVector() Error: head is null
    }

    // transpose on empty object
    try
    {
        Dimensions<size_t> d;
        auto dt = d.transpose();  // throws — empty
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << "\n";
        // Dimensions<T>::transpose(Axis, Axis) Error: Dimensions object is empty
    }

    // transpose with same axis
    try
    {
        Dimensions<size_t> d;
        d.fromVector({4, 8});
        // axis1=Last(-1) and axis2=SecondLast(-2) both normalize to same index on 2D
        auto dt = d.transpose(numcy::Axis::Last, numcy::Axis::Last);  // throws
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << "\n";
        // Dimensions<T>::transpose(Axis, Axis) Error: axis1 and axis2 must be different
    }

    return 0;
}
```

---

## Compilation

This class is designed to compile cleanly under strict warning flags:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wconversion \
    -Wsign-conversion -Wshadow -Wnon-virtual-dtor -Wold-style-cast \
    -Wcast-align -Wunused -Woverloaded-virtual -Wnull-dereference \
    -Wdouble-promotion -Wformat=2 -Wmisleading-indentation \
    -Wduplicated-cond -Wduplicated-branches -Wlogical-op \
    -Wuseless-cast -Weffc++ -O2 -fsanitize=address,undefined \
    main.cpp -o main
```

---

## Dependencies

| Header | Purpose |
|--------|---------|
| `<cassert>` | `assert()` for internal consistency checks in `append()` and `numel()` |
| `<string>` | `std::to_string()` in error messages |
| `<vector>` | `std::vector<T>` for `fromVector()`, `toVector()`, `reshape()`, and `transpose()` |
| `<limits>` | `std::numeric_limits<int>::max()` for the `INT_MAX` guard in `transpose()` |
| `DimensionsProperties.hh` | The linked list node type |

---

## Notes

- `Dimensions<T>` uses **shared ownership** via reference counting — copies share nodes rather than deep-copying them.
- The `append()` invariant (intermediate nodes have `columns = 0`, tail has non-zero `columns`) is the contract that `numel()`, `toVector()`, `fromVector()`, `getNumberOfColumns()`, and `getNumberOfRows()` all depend on.
- `fromVector()` does not reset the object before populating it — calling it on an already-populated object extends the list. This is intentional.
- `numel()` returns `0` for an empty list, which is the correct well-defined result. It uses `assert` rather than `throw` for the head/tail consistency check, terminating the program immediately on corruption.
- `reshape()` uses explicit manual move-semantics (nulling the temporary's pointers) rather than `std::move`, to remain compatible with the custom reference-counting design.
- `transpose()` works entirely in the signed `int` domain for axis arithmetic, crossing back into `size_t` only after full validation — this avoids every class of signed/unsigned comparison warning under strict `-Wconversion -Wsign-conversion`.
