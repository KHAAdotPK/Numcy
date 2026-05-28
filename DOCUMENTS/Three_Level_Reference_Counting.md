# Reference Counting in Numcy
### Memory Architecture of `Collective`, `CollectiveProperties`, `Dimensions`, and `DimensionsProperties`

**Project:** Numcy — A C++ Tensor Library  
**Author:** Q@hackers.pk  
**Scope:** This document describes the three-level reference counting architecture used to manage shared memory across tensor objects without dynamic memory reallocation.

---

## Table of Contents

1. [Motivation](#1-motivation)
2. [Architecture Overview](#2-architecture-overview)
3. [Level 1 — Collective and CollectiveProperties](#3-level-1--collective-and-collectiveproperties)
4. [Level 2 — CollectiveProperties and Dimensions](#4-level-2--collectiveproperties-and-dimensions)
5. [Level 3 — Dimensions and DimensionsProperties](#5-level-3--dimensions-and-dimensionsproperties)
6. [The Special Members of Collective](#6-the-special-members-of-collective)
7. [The Special Members of Dimensions](#7-the-special-members-of-dimensions)
8. [Destructor Chain — End to End](#8-destructor-chain--end-to-end)
9. [reshape() and the Reference Counting Architecture](#9-reshape-and-the-reference-counting-architecture)
10. [Why n Lives in Dimensions, Not DimensionsProperties](#10-why-n-lives-in-dimensions-not-dimensionsproperties)
11. [Thread Safety Note](#11-thread-safety-note)
12. [Invariants Summary](#12-invariants-summary)

---

## 1. Motivation

In a tensor library used for deep learning workloads — particularly Transformer and BERT architectures — the same tensor shape and the same underlying data buffer are frequently shared across multiple tensor objects. For example:

- Two tensors of the same shape used in an attention head share the same shape metadata.
- A tensor slice shares the same raw data allocation as its parent.
- Copying a tensor for use in a different part of the computation graph should not trigger a new `malloc` or `cudaMalloc`.

The goal of the reference counting architecture in Numcy is to make sharing **safe**, **transparent**, and **allocation-free** — while keeping full manual control over when memory is actually freed. This is especially important at the CUDA boundary, where `cudaMalloc` is expensive and `shared_ptr` cannot manage device memory.

---

## 2. Architecture Overview

The ownership hierarchy is three levels deep:

```
Collective A ──┐
               ├──► CollectiveProperties  (refcount = N)
Collective B ──┘         │
                         ├──► T* data[]               ← heap or device allocation
                         ├──► MemoryLocation           ← Host or Device
                         └──► Dimensions              ← value member
                                   ├──► head*
                                   └──► tail*
                                         └──► DimensionsProperties nodes
                                                   (each node has its own refcount)
```

Each level has its own reference counting strategy:

| Level | Owner Class | Owned Object | Ref Count Location |
|---|---|---|---|
| 1 | `Collective` | `CollectiveProperties*` | inside `CollectiveProperties` |
| 2 | `CollectiveProperties` | `Dimensions` (value) | N/A — value member |
| 3 | `Dimensions` | `DimensionsProperties*` nodes | inside each `DimensionsProperties` node |

---

## 3. Level 1 — Collective and CollectiveProperties

`Collective<T, E>` holds a single pointer to a `CollectiveProperties<T, E>` object on the heap. Multiple `Collective` instances can share the same `CollectiveProperties` object. The `reference_count` inside `CollectiveProperties` tracks how many `Collective` objects currently point to it.

### Rules

- When a `Collective` is **default-constructed**, `properties` is set to `nullptr`. No `CollectiveProperties` is allocated. This is a valid empty state.
- When a `Collective` is **constructed from a `Dimensions` object**, a new `CollectiveProperties` is allocated with `reference_count = 1`.
- When a `Collective` is **constructed from a raw pointer**, a new `CollectiveProperties` wrapping the external pointer is allocated with `reference_count = 1`. The `MemoryLocation` argument (default `Device`) determines which deallocation path is used later.
- When a `Collective` is **copy-constructed**, the `properties` pointer is shared and `reference_count` is incremented. Null-guarded — if `other.properties` is `nullptr`, no increment is performed.
- When a `Collective` is **copy-assigned**, the old `properties` reference count is decremented (and deleted if it reaches zero), then the new `properties` pointer is shared and its count is incremented. Both sides are null-guarded independently.
- When a `Collective` is **destroyed**, the `properties` reference count is decremented. If it reaches zero, `delete properties` is called, which triggers `~CollectiveProperties()`. `this->properties` is then set to `nullptr`.

### Example

```cpp
Dimensions<size_t> d(512, 64);

Collective<double> a(d);   // CollectiveProperties refcount = 1
Collective<double> b = a;  // CollectiveProperties refcount = 2
Collective<double> c = a;  // CollectiveProperties refcount = 3

// b goes out of scope → refcount = 2
// c goes out of scope → refcount = 1
// a goes out of scope → refcount = 0 → delete CollectiveProperties → data[] freed
```

---

## 4. Level 2 — CollectiveProperties and Dimensions

`CollectiveProperties<T, E>` holds a `Dimensions<E>` object as a **value member** — not a pointer. This means:

- The `Dimensions` object lives inside `CollectiveProperties` on the heap.
- When `CollectiveProperties` is constructed, its `Dimensions` member is copy-constructed from the argument, which triggers the `Dimensions` copy constructor — which in turn increments the reference count of every `DimensionsProperties` node it shares.
- When `~CollectiveProperties()` runs, the `Dimensions` value member destructor is called automatically by C++, which in turn decrements the reference count of every `DimensionsProperties` node.

No explicit management of `Dimensions` is needed inside `CollectiveProperties`. C++ value member semantics handle it correctly.

```
~CollectiveProperties()
    ├──► if Host:   delete[] data      (manual — pointer member)
    ├──► if Device: cudaFree(data)     (manual — only when COMPILE_FOR_DEVICE is defined)
    └──► ~Dimensions<E>()              (automatic — value member)
              └──► decrements each DimensionsProperties node's refcount
                        └──► deletes nodes where refcount reaches 0
                                  └──► relinks neighbors
```

---

## 5. Level 3 — Dimensions and DimensionsProperties

`Dimensions<T>` is a doubly-linked list of `DimensionsProperties<T>` nodes. Each node represents one 2D slice of a higher-dimensional tensor — storing a `rows` value and a `columns` value.

Multiple `Dimensions` objects can share the same underlying nodes. Each `DimensionsProperties` node has its own `reference_count` tracking how many `Dimensions` objects currently include that node in their list.

### Why per-node reference counting

Because different `Dimensions` objects can share **different subsets** of the same list. When `append()` adds a new node to one `Dimensions` object, that node is private to it — its `refcount = 1`. Nodes copied from another `Dimensions` object have higher refcounts. This means a single destructor pass may delete some nodes but not others, and must relink the remaining nodes to keep the list consistent.

### Mixed refcount scenario

```
Dimensions X: [A(2)] ──► [B(1)] ──► [C(2)],  n = 3
Dimensions Y: [A(2)] ──────────────► [C(2)],  n = 2

(B was appended only by X, never shared with Y)
```

When X is destroyed:
```
A: refcount 2 → 1  — survives, Y still holds it
B: refcount 1 → 0  — deleted, neighbors relinked: A.next = C, C.prev = A
C: refcount 2 → 1  — survives, Y still holds it

Y remains valid:  [A(1)] ──► [C(1)],  n = 2
```

### Relinking after selective deletion

When a node is deleted because its refcount reaches zero, the destructor relinks its neighbors:

```cpp
DimensionsProperties<T>* next = current->getNext();
DimensionsProperties<T>* prev = current->getPrevious();

delete current;

if (next != nullptr) { next->setPrev(prev); }
if (prev != nullptr) { prev->setNext(next); }
```

This preserves list integrity for all surviving `Dimensions` objects that share adjacent nodes.

---

## 6. The Special Members of Collective

### Default Constructor

```cpp
Collective(void) : properties(nullptr)
{
}
```

Constructs an empty `Collective` with `properties == nullptr`. No allocation occurs. This state is safe to assign to later but must not be used to call `operator[]`, `getShape()`, `getData()`, or any method that dereferences `properties`.

This constructor exists to support conditional initialization patterns such as those inside `toDevice()` and `toHost()`, where a local `Collective` variable must be declared before a `#ifdef COMPILE_FOR_DEVICE` block.

### Constructor from Dimensions (host allocation)

```cpp
Collective(const Dimensions<E>& d, MemoryLocation mem_loc = MemoryLocation::Host)
    : properties(nullptr)
{
    properties = new CollectiveProperties<T, E>(d, mem_loc);
    // properties->reference_count = 1
}
```

Allocates a new `CollectiveProperties` which in turn allocates the data array with `new T[d.numel()]`. Default `MemoryLocation` is `Host`.

### Constructor from raw pointer (device or host)

```cpp
Collective(T* ptr, const Dimensions<E>& d, MemoryLocation mem_loc = MemoryLocation::Device)
    : properties(nullptr)
{
    properties = new CollectiveProperties<T, E>(ptr, d, mem_loc);
    // properties->reference_count = 1
}
```

Wraps an externally allocated data pointer. Default `MemoryLocation` is `Device`, reflecting the primary use case of wrapping a `cudaMalloc`-allocated pointer. The `Collective` takes ownership — the appropriate deallocation (`delete[]` or `cudaFree`) is called when the refcount reaches zero.

### Copy Constructor

```cpp
Collective(const Collective<T, E>& other) : properties(other.properties)
{
    if (this->properties != nullptr)
    {
        this->properties->incrementReferenceCount();
    }
}
```

Shares `properties` pointer. Increments refcount. Null-guarded — if `other` was default-constructed, no increment is performed.

### Copy Assignment Operator

```cpp
Collective<T, E>& operator=(const Collective<T, E>& other)
{
    if (this != &other)                           // self-assignment guard
    {
        if (this->properties != nullptr)          // null guard
        {
            this->properties->decrementReferenceCount();

            if (this->properties->getReferenceCount() == 0)
            {
                delete this->properties;          // triggers ~CollectiveProperties()
            }
        }

        this->properties = other.properties;

        if (this->properties != nullptr)          // null guard
        {
            this->properties->incrementReferenceCount();
        }
    }

    return *this;
}
```

Releases old `properties`, acquires new one. Both sides null-guarded independently. Safe when assigning from or to a default-constructed `Collective`.

### Destructor

```cpp
~Collective()
{
    if (this->properties != nullptr)
    {
        this->properties->decrementReferenceCount();

        if (this->properties->getReferenceCount() == 0)
        {
            delete this->properties;   // triggers ~CollectiveProperties()
        }

        this->properties = nullptr;    // always nulled, whether or not deleted
    }
}
```

Decrements refcount. Deletes `CollectiveProperties` if it reaches zero. Always nulls `this->properties` afterward.

---

## 7. The Special Members of Dimensions

### Default Constructor

```cpp
Dimensions(void) : head(nullptr), tail(nullptr), n(0)
{
}
```

Creates an empty list. No nodes allocated.

### Copy Constructor

```cpp
Dimensions(const Dimensions<T>& other) : head(other.head), tail(other.tail), n(other.n)
{
    DimensionsProperties<T>* current = this->head;

    while (current != nullptr)
    {
        current->incrementReferenceCount();
        current = current->getNext();
    }
}
```

Shares all nodes. Increments every node's refcount. Copies `n` directly — each `Dimensions` object tracks its own length independently.

### Copy Assignment Operator

Releases the old list (decrement each node, delete if refcount reaches zero, relink neighbors), then acquires the new list (share pointer, increment each node's refcount, copy `n`). Self-assignment is checked first. `this->tail` is nulled before the release loop to avoid a dangling pointer if the tail node is deleted during release.

### Destructor

```cpp
~Dimensions(void)
{
    if (this->head != nullptr)
    {
        DimensionsProperties<T>* current = this->head;

        while (current != nullptr)
        {
            current->decrementReferenceCount();

            if (current->getReferenceCount() == 0)
            {
                DimensionsProperties<T>* next = current->getNext();
                DimensionsProperties<T>* prev = current->getPrevious();

                delete current;

                if (next != nullptr) { next->setPrev(prev); }
                if (prev != nullptr) { prev->setNext(next); }

                current = next;
            }
            else
            {
                current = current->getNext();
            }
        }

        this->head = nullptr;
        this->tail = nullptr;
        this->n    = 0;
    }
}
```

Walks the list forward. Decrements each node. Deletes and relinks selectively. Never touches a surviving node's neighbors incorrectly. Resets `head`, `tail`, and `n` to null/zero after the loop.

---

## 8. Destructor Chain — End to End

When the last `Collective` holding a `CollectiveProperties` is destroyed, the full destructor chain fires automatically:

```
~Collective()
    └──► properties->decrementReferenceCount()
              └──► refcount reaches 0
                        └──► delete properties
                                   └──► ~CollectiveProperties()
                                              ├──► if Host:   delete[] data
                                              ├──► if Device: cudaFree(data)
                                              │         (only when COMPILE_FOR_DEVICE defined)
                                              └──► ~Dimensions<E>()    (automatic)
                                                        └──► release loop over nodes
                                                                  └──► per-node refcount check
                                                                            └──► delete node if 0
                                                                                      └──► relink neighbors
    └──► this->properties = nullptr
```

No manual cleanup is required at any level beyond what is shown. C++ value member semantics handle the `Dimensions` destructor call automatically.

---

## 9. reshape() and the Reference Counting Architecture

`Dimensions::reshape()` interacts directly with the reference counting system and is worth understanding separately. It performs an in-place metadata replacement using a manual move-semantic pattern.

**Steps:**

1. Builds `newDimensions` from the new shape via `fromVector()`. All new nodes start with `refcount = 1`.
2. Validates `newDimensions.numel() == this->numel()`. Throws if not — no nodes are touched yet.
3. Runs the **release loop** over the existing nodes — the same loop as the destructor. Each existing node's refcount is decremented, and nodes that reach zero are deleted and relinked.
4. **Pointer steal:** Assigns `this->head`, `this->tail`, and `this->n` from `newDimensions`.
5. **Destructor neutralization:** Sets `newDimensions.head`, `newDimensions.tail`, and `newDimensions.n` to `nullptr`/`0`. When `newDimensions` goes out of scope, its destructor finds `head == nullptr` and does nothing — preventing a double-free of the nodes now owned by `this`.

This pattern is safe with respect to the reference counting system because:
- The old nodes are released exactly the same way as in the destructor.
- The new nodes' refcounts are never artificially incremented — `this` is the sole owner (refcount = 1 for each new node) after the steal.
- No other `Dimensions` object that shared old nodes is affected — surviving shared nodes were already relinked during the release loop.

---

## 10. Why `n` Lives in `Dimensions`, Not `DimensionsProperties`

Each `Dimensions` object maintains its own node count `n` independently. This is necessary because different `Dimensions` instances can share the same head node but have different tail nodes — each one seeing a different length of the same underlying list.

### Concrete example

```cpp
Dimensions<size_t> d1(10, 20);
// d1: head──►[A], tail──►[A], n = 1

Dimensions<size_t> d2(d1);
// d2: head──►[A], tail──►[A], n = 1  (shared with d1, A.refcount = 2)

d2.append(30, 40);
// d2: head──►[A]──►[B], tail──►[B], n = 2
// B is private to d2. A.refcount = 2, B.refcount = 1.

d2.append(50, 60);
// d2: head──►[A]──►[B]──►[C], tail──►[C], n = 3

Dimensions<size_t> d3(d2);
// d3: head──►[A]──►[B]──►[C], tail──►[C], n = 3
// A.refcount = 3, B.refcount = 2, C.refcount = 2.

d3.append(70, 80);
// d3: head──►[A]──►[B]──►[C]──►[D], tail──►[D], n = 4
// D is private to d3. D.refcount = 1.
```

After all of the above:

| Instance | `n` | Visible nodes |
|---|---|---|
| `d1` | 1 | `[A]` |
| `d2` | 3 | `[A]──►[B]──►[C]` |
| `d3` | 4 | `[A]──►[B]──►[C]──►[D]` |

Each `n` is correct and independent.

### Why `n` cannot live in `DimensionsProperties`

If `n` were stored inside a `DimensionsProperties` node, all `Dimensions` objects sharing that node would read and write the same counter. It would be impossible to track the length of any individual `Dimensions` object's list. The counter would reflect the total number of appends across all sharing instances, not the length of any one instance's list.

**Conclusion:** `n` belongs in `Dimensions`, not in `DimensionsProperties`.

---

## 11. Thread Safety Note

The reference counting in this implementation is **not thread-safe**. The `incrementReferenceCount()` and `decrementReferenceCount()` methods perform non-atomic increments and decrements on `size_t` values. Concurrent access to shared `CollectiveProperties` or `DimensionsProperties` objects from multiple threads without external synchronization will produce data races.

For the current use case — single-threaded CPU training and CUDA kernel dispatch from a single host thread — this is not a concern. If multi-threaded host access is needed in the future, the reference count fields should be changed to `std::atomic<size_t>` and the increment/decrement methods updated accordingly.

---

## 12. Invariants Summary

The following invariants must hold at all times for the system to be correct:

| Invariant | Description |
|---|---|
| **Collective** | `properties` is either `nullptr` or points to a valid `CollectiveProperties` whose `reference_count >= 1` |
| **Collective** | After destruction, `this->properties` is always set to `nullptr` |
| **CollectiveProperties** | `reference_count` equals the number of `Collective` objects currently holding a pointer to this object |
| **CollectiveProperties** | `data` is freed if and only if `reference_count` reaches zero — via `delete[]` for `Host`, via `cudaFree` for `Device` (when `COMPILE_FOR_DEVICE` is defined) |
| **Dimensions** | `head` and `tail` are either both `nullptr` or both non-null |
| **Dimensions** | `n` equals the number of nodes reachable by walking forward from `head` |
| **DimensionsProperties** | `reference_count` equals the number of `Dimensions` objects whose list includes this node |
| **DimensionsProperties** | A node is deleted if and only if its `reference_count` reaches zero |
| **DimensionsProperties** | After a node is deleted, its neighbors are relinked to preserve list continuity |
| **append() invariant** | In a multi-node list, all nodes except the tail have `columns = 0`; only the tail carries a non-zero `columns` value |
| **reshape() invariant** | After a reshape, `newDimensions.head/tail/n` are explicitly nulled to prevent double-free of transferred nodes |

---

*This document was written against the source files: `DimensionsProperties.hh`, `Dimensions.hh`, `CollectiveProperties.hh`, and `Collective.hh` as of the current development revision.*
