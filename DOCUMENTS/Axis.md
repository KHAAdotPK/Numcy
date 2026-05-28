# Axis — Numcy Axis Enum

**File:** `Numcy/axis.hh`  
**Project:** Numcy — A C++ Tensor Library  
**Author:** Q@hackers.pk

---

## Overview

In NumPy, the number of axes a tensor has is exactly equal to its **ndim** (number of dimensions). While we often talk about "Rows" and "Columns," NumPy is designed for N-dimensional arrays, meaning there is no theoretical limit to the number of axes other than available memory.

Standard axis mapping:

- **Axis 0:** First dimension. In a 2D array these are the **Rows** (moving vertically).
- **Axis 1:** Second dimension. In a 2D array these are the **Columns** (moving horizontally).
- **Axis 2:** Third dimension. In a 3D array this represents **Slices** (Depth/Pages).
- **Axis N:** The N-th dimension.

---

## The `Axis` Enum

`numcy::Axis` is a scoped enum (`enum class`) with underlying type `int`, defined inside the `numcy` namespace.

```cpp
namespace numcy {

    enum class Axis : int {
        Rows        = 0,  // Vertical (NumPy Axis 0)
        Columns     = 1,  // Horizontal (NumPy Axis 1)
        Slices      = 2,  // Third Dimension (NumPy Axis 2) Depth/Pages
        Last        = -1, // Innermost/last dimension (tail->columns)
        SecondLast  = -2  // Second-to-last dimension (tail->rows)
    };
}
```

| Enumerator | Value | NumPy Equivalent | Numcy Component | Description |
|---|---|---|---|---|
| `Rows` | `0` | Axis 0 | `tail->rows` | Vertical extent of the innermost 2D slice |
| `Columns` | `1` | Axis 1 | `tail->columns` | Horizontal extent of the innermost 2D slice |
| `Slices` | `2` | Axis 2 | `tail->prev->rows` | Third dimension — depth / pages |
| `Last` | `-1` | Axis -1 | `tail->columns` | Innermost / last dimension |
| `SecondLast` | `-2` | Axis -2 | `tail->rows` | Second-to-last dimension |

---

## Why a Scoped Enum with Underlying Type `int`

**`enum class` (scoped)** prevents enumerators from leaking into the enclosing scope and eliminates implicit conversion to integer. This avoids silent bugs where an integer constant is accidentally passed where an axis is expected.

**Underlying type `int`** is required because negative values (`Last = -1`, `SecondLast = -2`) must be representable. A plain `enum` with no underlying type would have implementation-defined behaviour for negative values under strict compilation flags. Specifying `: int` makes the representation well-defined and portable.

This also means `static_cast<int>(axis)` is safe and explicit, satisfying `-Wold-style-cast` and `-Wconversion` when converting an `Axis` value to an integer for arithmetic (e.g., negative index normalization in `Dimensions::transpose()`).

---

## How `Axis` Fits with the `Dimensions` Linked List

Numcy represents tensor shape as a doubly linked list of `DimensionsProperties` nodes. The `Axis` enum maps onto this structure as follows:

- **The tail node** holds the innermost 2D slice.
  - `tail->columns` — horizontal extent → `Axis::Columns` (1) / `Axis::Last` (-1)
  - `tail->rows` — vertical extent → `Axis::Rows` (0) / `Axis::SecondLast` (-2)
- **Nodes above the tail** (`tail->prev` up to `head`) each contribute one additional outer dimension → `Axis::Slices` (2) through Axis N.

`Dimensions::toVector()` is the bridge between this linked-list structure and positional axis indexing. It traverses from `head` to `tail`, collecting each node's `rows` value, then appends `tail->columns` as the last element. The resulting `std::vector<T>` gives O(1) lookup for any axis index:

```
vec[0] = head->rows        → outermost dimension
...
vec[n-2] = tail->rows      → second-to-last dimension  (Axis::SecondLast / -2)
vec[n-1] = tail->columns   → innermost dimension       (Axis::Last / -1)
```

**Example — 3D tensor with shape `{4, 5, 6}`:**

```
vec[0] = 4   →  Slices (outermost)
vec[1] = 5   →  Rows
vec[2] = 6   →  Columns (innermost)
```

---

## Negative Index Normalization

`Axis::Last` (`-1`) and `Axis::SecondLast` (`-2`) follow Python-style negative indexing. Code that accepts an `Axis` value (such as `Dimensions::transpose()`) normalizes negative values by adding `ndim` before any bounds check:

```cpp
int a = static_cast<int>(axis);
if (a < 0) { a += ndim; }
// a is now a non-negative index in [0, ndim)
```

This means `Last` and `Columns` refer to the same physical dimension on any tensor, and `SecondLast` and `Rows` refer to the same physical dimension on a 2D tensor.

---

## Usage

```cpp
#include "Numcy/axis.hh"

// Passing to Dimensions::transpose()
Dimensions<size_t> d;
d.fromVector({128, 64});

// Swap last two axes (default matrix transpose)
Dimensions<size_t> dt = d.transpose(numcy::Axis::Last, numcy::Axis::SecondLast);

// Equivalent using positive names on a 2D tensor
Dimensions<size_t> dt2 = d.transpose(numcy::Axis::Columns, numcy::Axis::Rows);
```

---

## Invariants

- `Axis::Last` always resolves to `tail->columns` regardless of tensor rank.
- `Axis::SecondLast` always resolves to `tail->rows` regardless of tensor rank.
- `Axis::Rows`, `Axis::Columns`, and `Axis::Slices` are fixed positive indices and are interpreted positionally from the outermost dimension inward via `toVector()`.
- It is a programming error to pass `axis1 == axis2` (after normalization) to any method that requires two distinct axes. `Dimensions::transpose()` detects and throws on this condition.
