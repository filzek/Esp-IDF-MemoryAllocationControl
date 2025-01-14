ESP-IDF Memory Tracking and Debugging Utilities
This repository provides a set of functions and macros to track, log, and debug dynamic memory allocations in ESP-IDF projects. It helps identify:

Memory leaks
Double frees
Out-of-bounds writes
General memory mismanagement
By wrapping standard heap_caps_* allocation functions with additional logging and tracking, you gain detailed insights into where and how your memory is used.

Key Features
Allocation Tracking
Each call to malloc, calloc, or realloc is replaced by a custom wrapper that tracks:

Source file, line number, and function name
Size, capabilities (caps), and address of the allocated block
Variable name (if available)
Allocation method (e.g., heap_caps_malloc, heap_caps_calloc, or heap_caps_realloc)
Thread-Safety
A SemaphoreHandle_t (memorySemaphore) is used to ensure thread-safe access to the allocation-tracking list.

Detailed Logging

Failed Allocations: Logs are generated when an allocation request fails, including the attempted size, method, and the memory capability requested.
Heap Info: The code can print detailed heap information (free memory in different capabilities) to help pinpoint memory issues.
Free with Zeroing

free_unregister_allocation zeroes out the memory block before freeing, helping catch issues like use-after-free.
Automatically unregisters the allocation record, preventing stale pointers in the tracking table.
Allocation List

A global array of MemoryTraceInfo structures tracks all live allocations.
The list_allocations function can be called to print out all current allocations, along with their sizes, addresses, and other metadata.
How It Works
Tracking Structure

c
Copiar código
typedef struct MemoryTraceInfo {
    char *file;
    int line;
    char *function;
    size_t size;
    char *method;   // e.g., "heap_caps_malloc", "heap_caps_calloc"
    uint32_t caps;  // Memory capabilities
    int failure;
    void *address;  // Pointer to allocated memory
    char *var_name; // Variable name (if available)
} MemoryTraceInfo;
This structure records all critical information about each allocation.

Wrapper Functions

c
Copiar código
void *traceable_heap_caps_malloc(size_t size, uint32_t caps, char *file, int line, char *function);
void *traceable_heap_caps_calloc(size_t n, size_t size, uint32_t caps, const char *file, int line, const char *function);
void *traceable_heap_caps_realloc(void *ptr, size_t size, uint32_t caps, char *file, int line, char *function);
These are drop-in replacements for heap_caps_malloc, heap_caps_calloc, and heap_caps_realloc. Each wrapper:

Invokes the actual ESP-IDF function (e.g., heap_caps_malloc).
Logs failures by calling log_memory_allocation if the allocation fails.
If successful, registers the allocation information in a global table using register_allocation.
Register and Unregister

register_allocation(MemoryTraceInfo info)
Inserts a new entry in the global tracking array.
unregister_allocation(void *ptr)
Removes an entry from the tracking array when the memory is freed.
Memory Log and Snapshot

c
Copiar código
void log_memory_allocation(MemoryTraceInfo info_requested);
Called when an allocation fails or if you want to capture a snapshot of the current memory state. It can be expanded or modified to write logs to flash, external files, or other logging sinks.

Free with Zeroing

c
Copiar código
void free_unregister_allocation(void **ptr);
Zeros out the memory block (if found in the allocation list) before freeing it.
Unregisters the allocation from the global table.
Sets the pointer to NULL to avoid dangling pointers.
Listing Current Allocations

c
Copiar código
void list_allocations();
Prints every tracked allocation’s:

Address
Caps
Size (in bytes)
Source file, line, and function
Variable name (if known)
Debug Macros
In your code, you can define macros that automatically pass __FILE__, __LINE__, and __FUNCTION__ to these wrappers. For instance:

c
Copiar código
#define TRACE_MALLOC(size, caps)  traceable_heap_caps_malloc((size), (caps), __FILE__, __LINE__, __FUNCTION__)
#define TRACE_CALLOC(n, size, caps) traceable_heap_caps_calloc((n), (size), (caps), __FILE__, __LINE__, __FUNCTION__)
#define TRACE_REALLOC(ptr, size, caps) traceable_heap_caps_realloc((ptr), (size), (caps), __FILE__, __LINE__, __FUNCTION__)
This way, you can replace all your heap_caps_* calls with these macros to log the allocation data transparently.

Usage
Include and Initialize
In your main application or a specific component, include the header that declares these tracking functions (e.g., memmap.h). Make sure to create and initialize the semaphore:

c
Copiar código
// Example in app_main() or initialization function
memorySemaphore = xSemaphoreCreateMutex();
Enable Debug Tracking
If you have a global flag like debug_track_allocation, set it to true to enable tracking:

c
Copiar código
extern bool debug_track_allocation;
debug_track_allocation = true; // Enable memory tracking
Use the Wrapper Functions
Replace your existing calls to heap_caps_malloc, heap_caps_calloc, and heap_caps_realloc with the new macros or direct function calls:

c
Copiar código
void *my_data = TRACE_MALLOC(100, MALLOC_CAP_8BIT);
Free Memory Correctly
Use free_unregister_allocation() instead of directly calling free() or heap_caps_free():

c
Copiar código
free_unregister_allocation((void **)&my_data);
This ensures proper zeroing, unregistering, and pointer nullification.

Check Allocations
Call list_allocations() wherever you want to see a summary of all current allocations.

Additional Heap Info
To print overall memory usage at any time, call:

c
Copiar código
print_memory_info("MyTag");
This shows detailed free-memory statistics for different capability regions.

Example
c
Copiar código
#include "memmap.h"

void app_main(void) {
    // Create the mutex for tracking
    memorySemaphore = xSemaphoreCreateMutex();

    // Enable debug tracking
    debug_track_allocation = true;

    // Allocate some memory
    char *buffer = TRACE_MALLOC(256, MALLOC_CAP_8BIT);
    if (!buffer) {
        printf("Failed to allocate buffer!\n");
        return;
    }

    // Use the buffer...
    snprintf(buffer, 256, "Hello from memory tracking!");

    // Print current allocations
    list_allocations();

    // Free the buffer
    free_unregister_allocation((void**)&buffer);

    // Print allocations again to confirm it is freed
    list_allocations();
}
Customization
Logging Method: In log_memory_allocation(), you can redirect output to a file, flash storage, or any logging system of your choice.
Memory Zeroing: If you do not want to zero out memory before freeing, you can remove or comment out the call to memset() inside free_unregister_allocation().
Thread Safety: Adjust semaphore logic based on your application’s concurrency needs.
Limitations
This module introduces additional overhead and is intended primarily for debugging. Consider disabling or removing it in production builds if performance or memory usage is critical.
The tracking array grows as allocations are performed and shrinks when allocations are freed. If your application has a high number of allocations, make sure you have enough heap space for this overhead.
Contributing
Contributions, bug reports, and feature requests are welcome! Feel free to open an issue or submit a pull request.

License
