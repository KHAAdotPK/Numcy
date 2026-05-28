# Numcy — CUDA Development Journal
### From CPU Tensor Library to GPU-Accelerated Implementation

**Project:** Numcy — A C++ Tensor Library with CUDA Support  
**Author:** Q@hackers.pk  
**Hardware:** Google Colab — Tesla T4 GPU (15360 MiB), CUDA 13.0, Driver 580.82.07  

---

## Table of Contents

1. [Project Background](#1-project-background)
2. [Architecture Overview](#2-architecture-overview)
3. [Phase 1 — Foundation: Reference Counting Architecture](#3-phase-1--foundation-reference-counting-architecture)
4. [Phase 2 — CUDA Infrastructure](#4-phase-2--cuda-infrastructure)
5. [Phase 3 — Build System](#5-phase-3--build-system)
6. [Phase 4 — First CUDA Kernel — randn](#6-phase-4--first-cuda-kernel--randn)
7. [Error Log — All Errors Encountered and Remedies](#7-error-log--all-errors-encountered-and-remedies)
8. [Current Status](#8-current-status)
9. [Next Steps](#9-next-steps)

---

## 1. Project Background

Numcy is a C++ numerical computing library built from scratch, implementing tensor operations needed for deep learning models. The CPU implementation is complete and has been used to successfully train:

- Skip-gram
- CBOW
- Transformer
- BERT

The current phase is rewriting all tensor operations to use CUDA kernels for GPU acceleration, targeting the device API (not the host API) for maximum learning and control.

The CPU `Numcy` class already implements: `matmul`, `matmul_backward`, `dot`, `mean`, `variance`, `exp`, `sqrt`, `pow`, `max`, `cos`, `enorm`, `enorm_distance`, `dropout`, `concatenate`, `softmax`, `layer_norm`, `LinAlg::norm`, `Spatial::Distance::cosine`, `Random::randn`, `Random::randn_xavier`, `Random::randn_word2vec`, `Random::binomial`, `Random::randint`, `Random::shuffle`, `arange`.

---

## 2. Architecture Overview

Numcy uses a three-level reference counted architecture:

```
Collective<T, E>
    └──► CollectiveProperties<T, E>*   (refcount — managed by Collective)
              ├──► T* data             (heap or device memory)
              └──► Dimensions<E>       (value member)
                        └──► DimensionsProperties<E>* nodes
                                  (linked list, each node has its own refcount)
```

### File Structure

```
Numcy/
├── MemoryLocation.hh          — enum: Host | Device
├── DimensionsProperties.hh    — linked list node for shape
├── Dimensions.hh              — shape container, manages node list
├── CollectiveProperties.hh    — data + shape + refcount
├── Collective.hh              — user-facing tensor handle
├── kernels.hh                 — CUDA __global__ kernels
├── Numcy.hh                   — static tensor operations
└── header.hh                  — master include, CUDA guards
```

### Include Chain

```
MemoryLocation.hh
    └──► DimensionsProperties.hh
              └──► Dimensions.hh
                        └──► CollectiveProperties.hh
                                  └──► Collective.hh
                                            └──► kernels.hh
                                                      └──► Numcy.hh
                                                                └──► header.hh
```

---

## 3. Phase 1 — Foundation: Reference Counting Architecture

### 3.1 Collective and CollectiveProperties

`CollectiveProperties` holds `T* data` and `Dimensions<E>` as a value member. Multiple `Collective` objects share the same `CollectiveProperties` via reference counting.

**Key design decisions made:**

- `operator=` on `CollectiveProperties` is `= delete` — assignment semantics are handled entirely by `Collective`
- `getDimensions()` returns `const Dimensions<E>&` — avoids copying the entire linked list
- `getShape()` in `Collective` also returns `const Dimensions<E>&` for the same reason
- Both `const` and non-`const` versions of `operator[]` implemented — required for `const Collective&` parameters in `Numcy` methods

**Destructor chain:**
```
~Collective()
    └──► decrementReferenceCount()
              └──► refcount == 0 → delete properties
                        └──► ~CollectiveProperties()
                                   ├──► delete[] data  (Host) or cudaFree(data) (Device)
                                   └──► ~Dimensions<E>()  (automatic — value member)
                                             └──► release loop over DimensionsProperties nodes
```

### 3.2 MemoryLocation Flag

Added `MemoryLocation.hh` with:

```cpp
enum class MemoryLocation { Host, Device };
```

`CollectiveProperties` destructor uses this to decide between `delete[]` and `cudaFree`:

```cpp
~CollectiveProperties()
{
    if (this->data != nullptr)
    {
        if (this->memory_location == MemoryLocation::Host)
            delete[] this->data;
        else
            cudaFree(this->data);   // Device path
        this->data = nullptr;
    }
}
```

### 3.3 New Constructors Added

```cpp
// Takes ownership of externally allocated pointer (cudaMalloc or new[])
Collective(T* ptr, Dimensions<E>& d, MemoryLocation loc)

// Allocates its own host memory internally
Collective(Dimensions<E>& d)
```

---

## 4. Phase 2 — CUDA Infrastructure

### 4.1 header.hh — Conditional CUDA Includes

```cpp
#ifdef COMPILE_FOR_DEVICE
    #include <cuda_runtime.h>
    #include <curand.h>
    #include <curand_kernel.h>    // curandState, curand_init, curand_normal_double
#endif
```

`COMPILE_FOR_DEVICE` is defined by the build script via `-DCOMPILE_FOR_DEVICE`. CPU builds never see CUDA headers.

### 4.2 kernels.hh — Device Kernels

```cpp
#ifdef COMPILE_FOR_DEVICE

__global__ void setup_curand_kernel(curandState* states, unsigned long seed, size_t n)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n)
        curand_init(seed, idx, 0, &states[idx]);
}

__global__ void randn_kernel(curandState* states, double* data, size_t n)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n)
        data[idx] = curand_normal_double(&states[idx]);
}

#endif
```

### 4.3 Thread Index Pattern

Every CUDA kernel uses this pattern:

```
Grid
├──► Block 0  [ thread0 ... thread255 ]
├──► Block 1  [ thread0 ... thread255 ]
...
└──► Block N  [ thread0 ... thread255 ]

idx = blockIdx.x * blockDim.x + threadIdx.x
```

`if (idx < n)` guards against threads beyond the array boundary when grid size is not an exact multiple of block size.

### 4.4 CUDA Context Initialization

Must be done at program start before any `cudaMalloc` call:

```cpp
cudaFree(0);            // Forces CUDA context initialization
cudaSetDevice(0);       // Explicitly select device 0
```

---

## 5. Phase 3 — Build System

### 5.1 Two Build Scripts

`build.sh` — CPU build, full strict flags including `-Wpedantic` and sanitizers:
```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion
    -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused
    -Woverloaded-virtual -Wnull-dereference -Wdouble-promotion -Wformat=2
    -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches
    -Wlogical-op -Wuseless-cast -Weffc++ -O2
    -fsanitize=address,undefined -fstack-protector-strong
```

`gpu-build.sh` — GPU build, same flags minus incompatible ones:
```bash
nvcc -arch=sm_75 main.cu \
    -Xcompiler "-std=c++17 -Wall ... (no -Wpedantic, no -fsanitize)" \
    -isystem /usr/local/cuda/include \
    --expt-relaxed-constexpr \
    -DCOMPILE_FOR_DEVICE \
    -lcurand \
    -o skipy.out
```

**Flags removed for GPU build and why:**

| Flag | Reason Removed |
|---|---|
| `-Wpedantic` | CUDA's internal code generation produces GCC-style `#line` directives that are not strictly ISO C++ compliant — fires on NVIDIA's own intermediate files |
| `-fsanitize=address` | AddressSanitizer intercepts memory allocation calls and conflicts with `cudaMalloc` — causes `out of memory` errors on valid allocations |
| `-fsanitize=undefined` | Same conflict with CUDA runtime memory management |
| `-Wold-style-cast` | CUDA system headers contain C-style casts that trigger this |

**Flags added for GPU build:**

| Flag | Purpose |
|---|---|
| `-isystem /usr/local/cuda/include` | Marks CUDA headers as system headers — suppresses warnings from NVIDIA's own code |
| `--expt-relaxed-constexpr` | Suppresses `constexpr operator&&` and `operator\|\|` errors from `nv/target` |
| `-lcurand` | Links the cuRAND library |
| `-DCOMPILE_FOR_DEVICE` | Enables CUDA code paths in all `#ifdef` guards |

### 5.2 Source File Must Be .cu

CUDA kernels (`__global__`, `blockIdx`, `threadIdx`) must be in a `.cu` file. When `nvcc` compiles `.cpp` files it hands them to the host compiler (`cc1plus`) which does not understand CUDA device code.

```
main.cpp  →  nvcc → cc1plus  →  blockIdx not declared ERROR
main.cu   →  nvcc directly   →  compiles correctly
```

---

## 6. Phase 4 — First CUDA Kernel — randn

### 6.1 Implementation Pattern

```cpp
// Step 1 — allocate device memory for data
cudaError_t err = cudaMalloc(&data, numel * sizeof(T));
if (err != cudaSuccess)
    throw std::runtime_error("cudaMalloc failed: " + std::string(cudaGetErrorString(err)));

// Step 2 — allocate device memory for cuRAND states
curandState* states = nullptr;
err = cudaMalloc(&states, sizeof(curandState) * numel);
if (err != cudaSuccess)
{
    cudaFree(data);
    throw std::runtime_error("cudaMalloc for states failed: " + std::string(cudaGetErrorString(err)));
}

// Step 3 — configure grid
size_t threads_per_block = 256;
size_t blocks = (numel + threads_per_block - 1) / threads_per_block;

// Step 4 — initialize cuRAND states
setup_curand_kernel<<<static_cast<unsigned int>(blocks),
                      static_cast<unsigned int>(threads_per_block)>>>(states, seed, numel);

// Step 5 — generate random numbers
randn_kernel<<<static_cast<unsigned int>(blocks),
               static_cast<unsigned int>(threads_per_block)>>>(states, data, numel);

// Step 6 — free states (no longer needed)
cudaFree(states);

// Step 7 — wrap device pointer in Collective
Collective<T, E> result(data, d, MemoryLocation::Device);
return result;
```

### 6.2 Reading Device Data Back to Host

Device memory cannot be read directly from CPU code. `operator[]` on a `Collective` backed by device memory will segfault. Must use `cudaMemcpy`:

```cpp
// copyToHost() method in Collective
std::vector<T> copyToHost(void) const
{
    size_t n = this->getShape().numel();
    std::vector<T> host_data(n);

#ifdef COMPILE_FOR_DEVICE
    cudaMemcpy(host_data.data(),
               this->properties->getData(),
               n * sizeof(T),
               cudaMemcpyDeviceToHost);
#else
    for (size_t i = 0; i < n; i++)
        host_data[i] = this->properties->getData()[i];
#endif

    return host_data;
}
```

---

## 7. Error Log — All Errors Encountered and Remedies

### Error 1 — `curandState` was not declared in this scope
**Cause:** `curand_kernel.h` was not included. `curand.h` provides the host API only. `curandState` lives in `curand_kernel.h`.  
**Fix:** Add `#include <curand_kernel.h>` inside the `COMPILE_FOR_DEVICE` guard in `header.hh`.

---

### Error 2 — style of line directive is a GCC extension [-Werror]
**Cause:** `-Wpedantic` passed via `-Xcompiler` to host compiler. CUDA's internal code generation produces GCC-style `#line` directives not compliant with ISO C++.  
**Fix:** Remove `-Wpedantic` from `gpu-build.sh`. Keep it in `build.sh` for CPU builds.

---

### Error 3 — `blockIdx` was not declared in this scope
**Cause:** Kernel code in `kernels.hh` being compiled by host compiler (`cc1plus`) instead of `nvcc`. This happened because `main.cpp` was passed to `nvcc` — `.cpp` files are handed to the host compiler which does not understand device intrinsics.  
**Fix:** Rename `main.cpp` to `main.cu`. `nvcc` compiles `.cu` files entirely itself, handling both device and host code correctly.

---

### Error 4 — sign-conversion errors in grid/block calculation
**Cause:** `threads_per_block` and `blocks` declared as `int` but `numel` is `size_t` (unsigned). Mixed signed/unsigned arithmetic under `-Wconversion -Wsign-conversion`.  
**Fix:** Declare both as `size_t`. Cast to `unsigned int` only at the kernel launch boundary:
```cpp
size_t threads_per_block = 256;
size_t blocks = (numel + threads_per_block - 1) / threads_per_block;
setup_kernel<<<static_cast<unsigned int>(blocks),
               static_cast<unsigned int>(threads_per_block)>>>(...);
```

---

### Error 5 — Warnings from CUDA system headers (curand_kernel.h, curand_poisson.h, nv/target)
**Cause:** NVIDIA's own headers violate `-Wconversion`, `-Wsign-conversion`, `-Wdouble-promotion`, `-Weffc++`. With `-Werror` these become compilation errors.  
**Fix:** Add `-isystem /usr/local/cuda/include` and `--expt-relaxed-constexpr` to `gpu-build.sh`. `-isystem` tells the compiler to suppress warnings from system headers.

---

### Error 6 — `cudaMalloc` returns `out of memory` with empty GPU
**Cause:** `-fsanitize=address` (AddressSanitizer) intercepts low-level memory allocation calls and conflicts with `cudaMalloc`, causing it to fail even when the GPU has free memory.  
**Fix:** Remove `-fsanitize=address` and `-fsanitize=undefined` from `gpu-build.sh`. Keep them in `build.sh` for CPU builds only.

---

### Error 7 — `No CUDA device available: out of memory`
**Cause:** Same as Error 6 — AddressSanitizer conflicting with CUDA runtime at the `cudaGetDeviceCount` call itself.  
**Fix:** Same — remove sanitizer flags from GPU build script.

---

### Error 8 — Segmentation fault after successful cudaMalloc
**Cause:** `operator[]` on a `Collective` backed by device memory dereferences a GPU pointer from CPU code. CPU cannot directly read device memory — this is a fundamental CUDA rule.  
**Fix:** Add `copyToHost()` method to `Collective` that uses `cudaMemcpy(cudaMemcpyDeviceToHost)` to bring data to a host buffer before reading.

---

### Error 9 — `MemoryLocation` not declared (CollectiveProperties)
**Cause:** `CollectiveProperties.hh` used `MemoryLocation` type without including `MemoryLocation.hh`.  
**Fix:** Add `#include "./MemoryLocation.hh"` to `CollectiveProperties.hh`.

---

### Error 10 — binding reference discards qualifiers
```
error: binding reference of type 'Dimensions<long unsigned int>&' 
       to 'const Dimensions<long unsigned int>' discards qualifiers
```
**Cause:** `getDimensions()` was declared `const` but returned `Dimensions<E>&` (non-const reference). Inside a `const` method, all members are treated as `const` — returning a non-const reference to a const member discards the const qualifier.  
**Fix:** Change return type to `const Dimensions<E>&`:
```cpp
const Dimensions<E>& getDimensions(void) const { return this->dimensions; }
```

---

## 8. Milestone — First Successful End-to-End CUDA Execution

**Date:** March 26, 2026  
**Hardware:** Tesla T4, CUDA 13.0, Driver 580.82.07

The `randn` kernel ran successfully end to end on real GPU hardware. 30,000 normally distributed random numbers were generated on the T4, copied back to host memory, and printed correctly.

### What the full execution path covered

```
main.cu
    └──► cudaFree(0)                          — CUDA context initialized
    └──► cudaSetDevice(0)                     — T4 selected
    └──► Numcy::randn(d)
              └──► cudaMalloc(&data, ...)     — device memory allocated
              └──► cudaMalloc(&states, ...)   — cuRAND states allocated
              └──► setup_curand_kernel<<<>>> — each thread initialized its curandState
              └──► randn_kernel<<<>>>        — each thread wrote one N(0,1) value
              └──► cudaFree(states)           — states freed
              └──► cudaMemcpy(DeviceToHost)   — data copied back to host
              └──► cudaFree(data)             — device data freed
              └──► Collective<double>(host_data, d, MemoryLocation::Host)
    └──► printed 30,000 values correctly
```

### Sample output (first few values)

```
-0.0674731 0.0121126 0.45911 -0.602107 -0.514806 0.630338 -0.714463
-0.663915 0.58946 -0.882945 -0.154319 0.435689 0.982069 0.031532 1.23034
-1.11286 1.47752 0.416914 0.426032 0.258961 ...
```

Values are normally distributed around 0 with standard deviation ~1 — correct behavior for `curand_normal_double`.

### Current implementation note

The current `randn` implementation copies data back to host after generation (`cudaMemcpyDeviceToHost`) and wraps the host pointer in a `Collective<T, E>` with `MemoryLocation::Host`. This is intentional for the first working implementation — it allows `operator[]` to work directly without a `copyToHost()` call. In a future iteration, data will remain on device and explicit `cudaMemcpy` will only happen when the user explicitly requests it.

---

## 9. Current Status

| Component | Status |
|---|---|
| `DimensionsProperties.hh` | Complete and verified |
| `Dimensions.hh` | Complete and verified |
| `CollectiveProperties.hh` | Complete and verified |
| `Collective.hh` | Complete |
| `MemoryLocation.hh` | Complete |
| `kernels.hh` | `setup_curand_kernel` and `randn_kernel` complete and verified |
| `Numcy.hh` — `randn` | **Complete and verified on Tesla T4** — 30,000 values generated correctly |
| Build system | Two scripts — `build.sh` (CPU) and `gpu-build.sh` (GPU) |
| Hardware | Tesla T4, 15360 MiB, CUDA 13.0, Driver 580.82.07 |

---

## 10. Next Steps

In order of priority:

1. **Keep data on device** — refactor `randn` to return a `Collective` with `MemoryLocation::Device`, only copying to host when explicitly requested
2. **`zeros` / `fill` kernel** — simplest kernel, pure write, no cuRAND dependency — good second kernel to establish the pattern
3. **Elementwise kernels** — `exp`, `add`, `multiply` — one thread per element pattern
4. **`matmul` kernel** — most complex, shared memory tiling, coalesced access
5. **Skip-gram forward pass on GPU** — first real model workload on CUDA

---

*This journal covers development from initial architecture design through first successful end-to-end CUDA kernel execution on Tesla T4, March 26, 2026.*
