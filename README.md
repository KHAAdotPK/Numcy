# Numcy — Documentation Index

**Project:** Numcy — A C++ Tensor Library  
**Author:** Q@hackers.pk

This file is the entry point for all documentation in the Numcy library. Each document below covers a specific class, concept, or architectural decision. They are ordered from the lowest level (individual node) up to the highest level (full tensor container), followed by cross-cutting architecture documents.

---

## Core Class Documentation

### [`Dimensions.md`](./DOCUMENTS/Dimensions.md)

**File:** `Numcy/Dimensions.hh`

Documents the `Dimensions<T>` class — the doubly-linked list that encodes tensor shape.

| Topic | Section |
|---|---|
| The core invariant (`columns = 0` on all but tail) | Overview |
| Why each instance keeps its own `n` | §Why Each Instance Keeps Its Own `n` |
| Default, two-parameter, pointer, and copy constructors | §Constructors |
| Destructor and selective node deletion | §Destructor |
| Assignment operator | §Operator Overloading |
| `append()` — building shapes one slice at a time | §append() |
| `fromVector()` — building from a `std::vector` | §fromVector() |
| `getNumberOfColumns()` and `getNumberOfRows()` | §getNumberOfColumns(), §getNumberOfRows() |
| `numel()` — total element count | §numel() |
| `reshape()` — in-place metadata surgery | §reshape() |
| `size()` — node count | §size() |
| `transpose()` — axis-swapped shape copy | §transpose() |
| `toVector()` — reconstruct shape vector | §toVector() |
| `size()` vs `numel()` distinction | §size() vs numel() |
| `n` maintenance table | §n Maintenance |
| Full usage examples (8 examples) | §Complete Usage Examples |

---

### [`Axis.md`](./DOCUMENTS/Axis.md)

**File:** `Numcy/axis.hh`

Documents the `numcy::Axis` scoped enum — the named constants used wherever an axis index is required.

| Topic | Section |
|---|---|
| Standard axis mapping (Rows, Columns, Slices) | §Overview |
| Full enum definition and value table | §The Axis Enum |
| Why `enum class` with underlying type `int` | §Why a Scoped Enum with Underlying Type `int` |
| Mapping onto the `Dimensions` linked list | §How Axis Fits with the Dimensions Linked List |
| Negative index normalization (`Last`, `SecondLast`) | §Negative Index Normalization |
| Usage examples | §Usage |
| Invariants | §Invariants |

---

### [`CollectiveProperties.md`](./DOCUMENTS/Collective.md) *(documented inside Collective.md)*

`CollectiveProperties<T, E>` is the shared control block holding `T* data`, `Dimensions<E>`, `reference_count`, and `MemoryLocation`. It has no standalone `.md` file — it is fully covered in `Collective.md`.

---

### [`Collective.md`](./DOCUMENTS/Collective.md)

**Files:** `Numcy/Collective.hh`, `Numcy/CollectiveProperties.hh`

Documents the primary tensor container `Collective<T, E>` and its backing store `CollectiveProperties<T, E>`.

| Topic | Section |
|---|---|
| What `Collective` is | §1 |
| Why two classes (`Collective` + `CollectiveProperties`) | §2 |
| Template parameters `T` and `E` | §3 |
| Memory layout diagram | §4 |
| Default constructor (`properties = nullptr`) | §5 |
| Constructor from `Dimensions` (host allocation) | §5 |
| Constructor from raw pointer (device/host) | §5 |
| Copy constructor — shared ownership, no data copy | §6 |
| Copy assignment operator | §7 |
| Destructor and `properties = nullptr` | §8 |
| `operator[]` — bounds-checked element access | §9 |
| `getShape()` — const reference to shape | §10 |
| `getData()` — raw pointer for CUDA kernels | §11 |
| `getMemoryLocation()` — Host or Device | §12 |
| `toDevice()` and `toHost()` — CUDA transfers | §13 |
| `transpose()` — axis-swapped collective | §14 |
| Full usage examples (7 examples) | §15 |
| Design decisions and rationale | §16 |
| Invariants | §17 |
| What `Collective` does not do | §18 |

---

## Architecture Documents

### [`Three_Level_Reference_Counting.md`](./DOCUMENTS/Three_Level_Reference_Counting.md)

Describes the full three-level reference counting architecture that connects `Collective`, `CollectiveProperties`, `Dimensions`, and `DimensionsProperties` into a single coherent memory management system.

| Topic | Section |
|---|---|
| Motivation — why shared ownership matters for deep learning | §1 |
| Architecture overview diagram and level table | §2 |
| Level 1 — `Collective` ↔ `CollectiveProperties` | §3 |
| Level 2 — `CollectiveProperties` ↔ `Dimensions` (value member) | §4 |
| Level 3 — `Dimensions` ↔ `DimensionsProperties` nodes | §5 |
| All special members of `Collective` with code | §6 |
| All special members of `Dimensions` with code | §7 |
| Full destructor chain end-to-end | §8 |
| `reshape()` and the reference counting architecture | §9 |
| Why `n` lives in `Dimensions`, not `DimensionsProperties` | §10 |
| Thread safety note | §11 |
| Invariants summary table | §12 |

---

### [`collective_data_model.md`](./DOCUMENTS/Collective_Data_Model.md)

Describes how tensor data is physically stored in memory — the C row-major layout, the index arithmetic, and the relationship between the `Dimensions` linked list and the flat buffer.

| Topic | Section |
|---|---|
| Flat contiguous buffer and the two key objects | §1 |
| The C array analogy — how `arr[s][r][c]` maps to a flat offset | §2 |
| Side-by-side comparison: C array vs `Collective` | §3 |
| `Dimensions` node structure and the `columns = 0` invariant | §4.1 |
| How `fromVector()` builds nodes (shape length `k` → `k-1` nodes) | §4.2 |
| Worked example: shape `[2, 4, 8]` — 2 nodes, `numel = 64` | §4.3 |
| Worked example: shape `[2, 4, 8, 16]` — 3 nodes, `numel = 1024` | §4.4 |
| General rule: shape vector length `k` → `k-1` nodes | §4.5 |
| N-D flat index formula | §5 |
| Why physical transpose only affects `Last ↔ SecondLast` | §6 |
| Ownership model and `MemoryLocation` | §7.1–7.2 |
| Default-constructed `Collective` (`properties = nullptr`) | §7.3 |
| Lifecycle of a transposed `Collective` | §7.4 |
| Summary table | §8 |

---

## Reading Order

If you are new to the codebase, read in this order:

1. `DOCUMENTS/Collective_Data_Model.md` — understand the memory model first
2. `DOCUMENTS/Axis.md` — understand how axes are named
3. `DOCUMENTS/Dimensions.md` — understand how shape is represented
4. `DOCUMENTS/Collective.md` — understand the full tensor container
5. `DOCUMENTS/Three_Level_Reference_Counting.md` — understand the ownership and lifetime model

---

*All documents were written against the current development revision of the Numcy source files.*
