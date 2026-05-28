/*
 * Numcy/MemoryLocation.hh
 * Q@hackers.pk
 */

#ifndef NUMCY_MEMORY_LOCATION_HH
#define NUMCY_MEMORY_LOCATION_HH

/*
 * Indicates where a data pointer lives.
 * CollectiveProperties uses this to decide whether to call
 * delete[] (Host) or cudaFree (Device) in its destructor.
 */
enum class MemoryLocation
{
    Host,    // CPU heap — allocated with new[], freed with delete[]
    Device,  // GPU memory — allocated with cudaMalloc, freed with cudaFree
    None     // No memory allocated, used for uninitialized collectives
};

#endif