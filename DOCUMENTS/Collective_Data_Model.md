# Collective\<T, E\> — Data Storage Model

*Numcy — Q@hackers.pk*

---

## Table of Contents

1. [Overview](#1-overview)
2. [The C Array Analogy](#2-the-c-array-analogy)
3. [How Collective\<T, E\> Mirrors a C Array](#3-how-collectivet-e-mirrors-a-c-array)
4. [The Dimensions\<E\> Linked List](#4-the-dimensionse-linked-list)
5. [Flat Index Arithmetic](#5-flat-index-arithmetic)
6. [Why Physical Transpose Touches Only the Last Two Axes](#6-why-physical-transpose-touches-only-the-last-two-axes)
7. [Ownership and Reference Counting](#7-ownership-and-reference-counting)
8. [Summary](#8-summary)

---

## 1. Overview

The `Collective<T, E>` class stores multi-dimensional tensor data in a **flat, contiguous heap buffer** that is laid out in **C row-major order** — exactly the same way the C language stores multi-dimensional arrays, with the single difference that C arrays have a fixed shape baked into the type at compile time, while `Collective` carries its shape dynamically in a linked list called `Dimensions<E>`.

The two key objects that make up every `Collective` instance are:

| Object | Role |
|---|---|
| `T* data` | Raw heap buffer holding all scalar values, allocated once, never shuffled unless a physical transpose is requested. |
| `Dimensions<E>` | Linked list of `DimensionsProperties` nodes. Encodes shape. Tells `Collective` how to interpret the flat index as an N-D coordinate. |

---

## 2. The C Array Analogy

In C, when you write:

```c
int arr[2][3][4];
```

the compiler allocates `2 × 3 × 4 = 24` integers in a single contiguous block of memory. The shape `[2][3][4]` is a compile-time constant — it lives in the type, not in any runtime variable.

Accessing `arr[s][r][c]` compiles down to a single pointer arithmetic expression:

```
*(arr  +  s*(3*4)  +  r*4  +  c)
```

which is identically:

```
offset = s * (rows * cols) + r * cols + c
```

This is row-major order: the **rightmost (innermost) index moves fastest** in memory. Incrementing `c` by 1 moves exactly one element forward. Incrementing `r` by 1 jumps over an entire row of `cols` elements.

---

## 3. How Collective\<T, E\> Mirrors a C Array

For a `Collective` with shape `[2][3][4]`, the heap buffer pointed to by `data` contains exactly the same 24 values in exactly the same order as the C array above.

| Property | C array `int arr[2][3][4]` | `Collective<double, size_t>` |
|---|---|---|
| Storage | Stack or static segment | Heap (`new T[numel]`) |
| Memory layout | Row-major, contiguous | Row-major, contiguous — identical |
| Shape | Fixed in the type at compile time | Dynamic — `Dimensions<E>` linked list |
| Index formula | Compiler generates it | Caller / method computes it manually |
| `ndim` | Determined by the type declaration | Determined by number of linked nodes + 1 |
| Element access | `arr[s][r][c]` | `data[s*(rows*cols) + r*cols + c]` |

The only runtime difference is **where the shape lives**. Everything else — the bit pattern in memory, the index arithmetic, the stride rules — is identical to a plain C array.

---

## 4. The Dimensions\<E\> Linked List

### 4.1 Node structure

Each `DimensionsProperties<T>` node stores two values:

- `rows` — the height of one 2D slice (or the count of outer slices for non-tail nodes).
- `columns` — non-zero **only on the tail node**; it is the innermost (fastest-changing) dimension.

The invariant enforced by `append()` is:

```
All nodes above the tail  →  columns == 0
Tail node                 →  columns != 0  (innermost dimension)
```

### 4.2 How `fromVector()` builds the list

`fromVector(vec)` calls `append(T(0), vec[i])` for every element except the last two, then calls `append(vec[n-1], vec[n-2])` for the tail. This means a shape vector of length `k` produces `k - 1` nodes, not `k` nodes.

### 4.3 Example: shape `[2, 4, 8]`

`fromVector({2, 4, 8})` produces **two nodes**:

| Node | `rows` | `columns` | Meaning |
|---|---|---|---|
| head (node 1) | 2 | 0 | 2 outer slices |
| tail (node 2) | 4 | 8 | each slice has 4 rows and 8 columns |

`toVector()` walks from `head` to `tail` collecting each node's `rows` value, then appends `tail->columns` at the end, reconstructing the shape vector `[2, 4, 8]`.

`numel()` computes:

```
total = tail->columns * product_of_all_rows
      = 8  *  2  *  4
      = 64
```

### 4.4 Example: shape `[2, 4, 8, 16]`

`fromVector({2, 4, 8, 16})` produces **three nodes**:

| Node | `rows` | `columns` | Meaning |
|---|---|---|---|
| head (node 1) | 2 | 0 | 2 outermost slices |
| node 2 | 4 | 0 | each outer slice has 4 sub-slices |
| tail (node 3) | 8 | 16 | each sub-slice has 8 rows and 16 columns |

`numel()` = `16 * 2 * 4 * 8 = 1024`.

### 4.5 General rule

A shape vector of length `k` → `k - 1` nodes → `numel = vec[k-1] * vec[0] * vec[1] * … * vec[k-2]`.

---

## 5. Flat Index Arithmetic

For shape vector `[d₀, d₁, …, d_{n-1}]`, the flat offset of element `[i₀, i₁, …, i_{n-1}]` is:

```
offset = i₀ * (d₁ * d₂ * … * d_{n-1})
       + i₁ * (d₂ * … * d_{n-1})
       + …
       + i_{n-2} * d_{n-1}
       + i_{n-1}
```

For the concrete shape `[2, 4, 8]` this reduces to:

```
offset = s * (4 * 8)  +  r * 8  +  c
       = s * 32       +  r * 8  +  c
```

This is word-for-word the formula a C compiler emits for `arr[s][r][c]` when `arr` is declared as `int arr[2][4][8]`.

---

## 6. Why Physical Transpose Touches Only the Last Two Axes

The innermost axis (`d_{n-1}`, i.e. `columns`) has stride 1 — its elements sit consecutively in memory. The second-innermost axis (`d_{n-2}`, i.e. `rows`) has stride `d_{n-1}`.

**When you swap Last ↔ SecondLast:**

The new innermost axis is what used to be `rows`, and the new stride-1 axis is now `columns`. Element `[s][r][c]` which lived at offset `r*cols + c` inside its slice must now live at `c*rows + r`. No shape metadata change can correct this — the physical byte positions are wrong for the new interpretation. A data copy is mandatory.

**When you swap any other pair of axes (e.g. axis 0 ↔ axis 1):**

The innermost axis is untouched; its stride-1 contiguity is preserved. The new shape correctly describes how to compute offsets into the unchanged buffer. Only the shape metadata (the `Dimensions` linked list) needs to change. The data bytes do not move.

This is the direct C-array analogy: reinterpreting `int arr[A][B][C]` as `int arr[B][A][C]` works correctly with the same flat buffer — no data movement needed. But reinterpreting `int arr[A][B][C]` as `int arr[A][C][B]` changes the innermost stride and forces a physical rewrite.

---

## 7. Ownership and Reference Counting

### 7.1 CollectiveProperties — the shared control block

Rather than copy the data buffer on every assignment, `Collective` uses a shared-ownership model. The `CollectiveProperties<T, E>` object acts as a control block that holds:

- The raw data pointer (`T* data`).
- The `Dimensions<E>` value member (shape, owned by value).
- A reference count (`size_t reference_count`).
- The `MemoryLocation` tag (`Host` or `Device`).

Multiple `Collective` handles can point to the same `CollectiveProperties` block. The block is destroyed (and the data buffer freed) only when the last handle decrements the reference count to zero. Deallocation uses `delete[]` for `MemoryLocation::Host` and `cudaFree` for `MemoryLocation::Device` (the latter only when `COMPILE_FOR_DEVICE` is defined).

### 7.2 Dimensions nodes — a second level of sharing

Inside `Dimensions<E>`, individual `DimensionsProperties` nodes carry their own reference count. A copy of a `Dimensions` object shares the head nodes with the original and only adds private nodes via subsequent `append()` calls. The destructor selectively deletes only the nodes whose reference count reaches zero, relinking surviving neighbours.

### 7.3 The default-constructed Collective

`Collective` has a default constructor that sets `properties = nullptr`. This is a valid empty state used inside `toDevice()` and `toHost()` for conditional initialization before a `#ifdef COMPILE_FOR_DEVICE` block. Any method that dereferences `properties` checks for `nullptr` first and throws `std::runtime_error` before accessing data.

### 7.4 Lifecycle of a transposed Collective

When `transpose(Last, SecondLast)` is called (physical transpose):

1. A new `T[]` buffer is allocated (`new T[numel]`).
2. Data is physically reordered into that buffer.
3. A new `Dimensions` object is created with the swapped shape.
4. A new `CollectiveProperties` is constructed around the new buffer and shape.
5. A new `Collective` handle is returned — `refcount = 1`.
6. The original `Collective` and its buffer are unchanged.

For non-innermost axis swaps, only steps 3–5 occur. The new buffer is a flat copy of the original with a different shape description attached; no physical reordering is needed.

> **Note:** Physical data reordering for the `Last ↔ SecondLast` case is currently stubbed in `Collective::transpose()` and is planned for implementation via `Numcy::transpose`. Metadata-only transposition for all other axis pairs is fully operational.

---

## 8. Summary

| Concept | In `Collective<T, E>` |
|---|---|
| Flat contiguous buffer | `T* data` in `CollectiveProperties` |
| Shape | `Dimensions<E>` linked list (dynamic) |
| Node count for shape of length k | `k - 1` nodes |
| Row-major layout | Identical to C multi-dimensional arrays |
| Index formula | `offset = i0*(d1*...*dn-1) + ... + in-1` |
| Innermost axis | `tail->columns` in `Dimensions` (stride = 1) |
| Second-innermost axis | `tail->rows` in `Dimensions` (stride = `tail->columns`) |
| Physical transpose needed | Only when swapping `Last ↔ SecondLast` axes |
| Metadata-only transpose | All other axis pair swaps |
| Ownership model | Reference-counted `CollectiveProperties` block |
| Empty Collective | Default constructor — `properties = nullptr` |
| CUDA support | `MemoryLocation::Device` — same row-major layout on GPU |
