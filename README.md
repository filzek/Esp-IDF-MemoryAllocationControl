# ESP-IDF Memory Tracking and Debugging Utilities

This repository provides a set of functions and macros to **track**, **log**, and **debug** dynamic memory allocations in ESP-IDF projects. It helps identify:

- Memory leaks
- Double frees
- Out-of-bounds writes
- General memory mismanagement

By wrapping standard `heap_caps_*` allocation functions with additional logging and tracking, you gain detailed insights into where and how your memory is used.

---

## Key Features

1. **Allocation Tracking**  
   Each call to `malloc`, `calloc`, or `realloc` is replaced by a custom wrapper that tracks:
   - Source file, line number, and function name
   - Size, capabilities (caps), and address of the allocated block
   - Variable name (if available)
   - Allocation method (e.g., `heap_caps_malloc`, `heap_caps_calloc`, or `heap_caps_realloc`)

2. **Thread-Safety**  
   A `SemaphoreHandle_t` (`memorySemaphore`) is used to ensure thread-safe access to the allocation-tracking list. 

3. **Detailed Logging**  
   - **Failed Allocations**: Logs are generated when an allocation request fails, including the attempted size, method, and memory capability requested.
   - **Heap Info**: Prints detailed heap information for different memory capabilities, helping pinpoint memory issues.

4. **Free with Zeroing**  
   - `free_unregister_allocation` zeroes out the memory block before freeing, catching issues like use-after-free.
   - Automatically unregisters the allocation record, preventing stale pointers in the tracking table.

5. **Allocation List**  
   - A global array of `MemoryTraceInfo` structures tracks all live allocations.
   - The `list_allocations` function prints out all current allocations, along with their sizes, addresses, and other metadata.

---

## How It Works

### Tracking Structure

```c
typedef struct MemoryTraceInfo {
    char *file;
    int line;
    char *function;
    size_t size;
    char *method;   // e.g. "heap_caps_malloc"
    uint32_t caps;  // Memory capabilities
    int failure;
    void *address;  // Pointer to allocated memory
    char *var_name; // Variable name (if available)
} MemoryTraceInfo;
