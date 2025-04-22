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
``` 

# ESP-IDF Memory Tracking and Debugging Utilities

## Stores all critical information about each allocation:

```c
typedef struct MemoryTraceInfo {
    char *file;
    int line;
    char *function;
    size_t size;
    char *method;   // e.g., "heap_caps_malloc"
    uint32_t caps;  // Memory capabilities
    int failure;
    void *address;  // Pointer to allocated memory
    char *var_name; // Variable name (if available)
} MemoryTraceInfo;
``` 

This structure tracks details such as file name, line number, function name, allocated size, memory capabilities, allocation method, failure status, pointer address, and variable name (if available).

##Wrapper Functions
These wrappers replace the standard heap_caps_* functions for logging and tracking. They invoke the real ESP-IDF memory functions and register allocations if successful.

```c
void *traceable_heap_caps_malloc(size_t size, uint32_t caps, char *file, int line, char *function);
void *traceable_heap_caps_calloc(size_t n, size_t size, uint32_t caps, const char *file, int line, const char *function);
void *traceable_heap_caps_realloc(void *ptr, size_t size, uint32_t caps, char *file, int line, char *function);
``` 

#What They Do
Allocate memory using ESP-IDF’s heap_caps_* functions.
Log failures by calling log_memory_allocation if the allocation fails.
Register successful allocations by calling register_allocation.

Register and Unregister
register_allocation(MemoryTraceInfo info)
Adds a new entry to the global tracking array for successful allocations.

*unregister_allocation(void ptr)
Removes the entry corresponding to ptr from the global tracking array when memory is freed.

#Logging

```c
void log_memory_allocation(MemoryTraceInfo info_requested);
``` 

Whenever an allocation fails (or if you want to capture a snapshot), this function logs details of the failure. It can be customized to write logs to flash, files, or other sinks.

#Free with Zeroing

```c
void free_unregister_allocation(void **ptr);
``` 

Zeros out the memory block (if found) before freeing.
Unregisters it from the global tracking array.
Sets the caller’s pointer to NULL to avoid dangling pointers.

#Listing Current Allocations

```c
void list_allocations();
``` 

Prints out all tracked allocations, including:

Pointer address
Memory capabilities (caps)
Allocation size
Source file, line, and function
Variable name (if available)

#Debug Macros

To automatically include file name, line number, and function in the tracking data, define macros like:

```c
#define TRACE_MALLOC(size, caps) \
    traceable_heap_caps_malloc((size), (caps), __FILE__, __LINE__, __FUNCTION__)

#define TRACE_CALLOC(n, size, caps) \
    traceable_heap_caps_calloc((n), (size), (caps), __FILE__, __LINE__, __FUNCTION__)

#define TRACE_REALLOC(ptr, size, caps) \
    traceable_heap_caps_realloc((ptr), (size), (caps), __FILE__, __LINE__, __FUNCTION__)
``` 

Replace direct calls to heap_caps_* with these macros in your code to enable automatic tracking.


#Example Usage

```c
#include "memmap.h"

void app_main(void) {
    // Create a mutex for thread-safe tracking
    memorySemaphore = xSemaphoreCreateMutex();

    // Enable memory allocation debug tracking
    debug_track_allocation = true;

    // Allocate memory with tracking
    char *buffer = TRACE_MALLOC(256, MALLOC_CAP_8BIT);
    if (!buffer) {
        printf("Failed to allocate buffer!\n");
        return;
    }

    // Use the buffer as needed
    snprintf(buffer, 256, "Hello from memory tracking!");

    // Print current allocations
    list_allocations();

    // Free the buffer and unregister
    free_unregister_allocation((void **)&buffer);

    // Confirm it's removed from tracking
    list_allocations();
}
``` 


#Additional Notes
##Heap Information:
You can call print_memory_info("YourTag"); at any time to see a breakdown of free memory for different capabilities.

##Performance Considerations:
This system adds overhead and is best used during development to detect memory issues. Disable or remove it for production if performance is critical.

##Semaphore Usage:
A global SemaphoreHandle_t memorySemaphore is used to protect the tracking array from concurrent modifications.


<a href="https://scan.coverity.com/projects/esp-idf-memory-tracking-and-debugging-utilities">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/31614/badge.svg"/>
</a>
