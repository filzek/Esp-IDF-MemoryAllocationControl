// Macros for ease of use
#include "memmap.h"

SemaphoreHandle_t memorySemaphore    						= NULL;

typedef struct MemoryTraceInfo {
    char *file;
    int line;
    char *function;
    size_t size;
    char *method; // malloc, calloc, realloc
    uint32_t caps;
    int failure;
    void *address; // Pointer to allocated memory
    char *var_name; // Name of the variable
} MemoryTraceInfo;


// Function to log memory allocation
char * get_memory_capability_name(uint32_t caps) {
    if (caps & MALLOC_CAP_EXEC) {
        return (char *)"MALLOC_CAP_EXEC ";
    }
    if (caps & MALLOC_CAP_32BIT) {
        return (char *)"MALLOC_CAP_32BIT ";
    }
    if (caps & MALLOC_CAP_8BIT) {
        return (char *)"MALLOC_CAP_8BIT ";
    }
    if (caps & MALLOC_CAP_DMA) {
        return (char *)"MALLOC_CAP_DMA ";
    }
    if (caps & MALLOC_CAP_PID2) {
        return (char *)"MALLOC_CAP_PID2 ";
    }
    if (caps & MALLOC_CAP_PID3) {
        return (char *)"MALLOC_CAP_PID3 ";
    }
    if (caps & MALLOC_CAP_PID4) {
        return (char *)"MALLOC_CAP_PID4 ";
    }
    if (caps & MALLOC_CAP_PID5) {
        return (char *)"MALLOC_CAP_PID5 ";
    }
    if (caps & MALLOC_CAP_PID6) {
        return (char *)"MALLOC_CAP_PID6 ";
    }
    if (caps & MALLOC_CAP_PID7) {
        return (char *)"MALLOC_CAP_PID7 ";
    }
    if (caps & MALLOC_CAP_SPIRAM) {
        return (char *)"MALLOC_CAP_SPIRAM ";
    }
    if (caps & MALLOC_CAP_INTERNAL) {
        return (char *)"MALLOC_CAP_INTERNAL ";
    }
    if (caps & MALLOC_CAP_DEFAULT) {
        return (char *)"MALLOC_CAP_DEFAULT ";
    }
  return (char *)"unknow";
}

// Function to capture a memory snapshot (Implement as needed)
void log_memory_allocation(MemoryTraceInfo info_requested) {

// Implementation depends on how you want to capture and log the memory snapshot
printf("Capturing memory snapshot for region: %s\n"
       "Failed to allocate %zu bytes for %s in %s at %s:%d\n", get_memory_capability_name(info_requested.caps), info_requested.size, info_requested.method, info_requested.function, info_requested.file, info_requested.line);

// Print heap information for the specified memory capability
heap_caps_print_heap_info(info_requested.caps);

printf("Done\n\n\n\n");

multi_heap_info_t info;
heap_caps_get_info(&info, info_requested.caps);
}

// Global array to store tracked allocations
MemoryTraceInfo *memory_track_allocation = NULL;
size_t memory_track_allocation_t = 0;

// Function to register a new allocation
void register_allocation(MemoryTraceInfo info) {
    if (debug_track_allocation){
      if (xSemaphoreTake(memorySemaphore, portMAX_DELAY) == pdTRUE) {
        // Increase the list size by 1 to hold the new allocation
        MemoryTraceInfo *new_allocation = realloc(memory_track_allocation, (memory_track_allocation_t + 1) * sizeof(MemoryTraceInfo));
        if (new_allocation == NULL) {
          printf("Failed to allocate memory for tracking allocations\n");
          wait(5000);
          xSemaphoreGive(memorySemaphore);
          return;
        }

        memory_track_allocation = new_allocation;
        memory_track_allocation[memory_track_allocation_t] = info; // Store the new allocation info
        memory_track_allocation_t++; // Update the count of tracked allocations
        xSemaphoreGive(memorySemaphore);
      }
    }
}

// Function to unregister an allocation (free memory tracking entry)
void unregister_allocation(void *ptr) {
    if (ptr == NULL || !debug_track_allocation) {
        return; // No need to process NULL pointers or if tracking is disabled
    }

    if (xSemaphoreTake(memorySemaphore, portMAX_DELAY) == pdTRUE) {
      for (size_t i = 0; i < memory_track_allocation_t; i++) {
        if (memory_track_allocation[i].address == ptr) {
          // Shift the remaining elements to fill the gap
          for (size_t j = i; j < memory_track_allocation_t - 1; j++) {
            memory_track_allocation[j] = memory_track_allocation[j + 1];
          }
          memory_track_allocation_t--;

          // Shrink the array size using realloc
          if (memory_track_allocation_t > 0) {
            MemoryTraceInfo *new_allocation = realloc(memory_track_allocation, memory_track_allocation_t * sizeof(MemoryTraceInfo));
            if (new_allocation != NULL) {
              memory_track_allocation = new_allocation;
            }
            // If realloc fails, keep the old allocation to avoid memory leak
          } else {
            // Free the memory if tracking array is empty
            free(memory_track_allocation);
            memory_track_allocation = NULL;
          }
          xSemaphoreGive(memorySemaphore);
          return;
        }
      }
      xSemaphoreGive(memorySemaphore);
    }

}

void free_unregister_allocation(void **ptr) {
    if (ptr != NULL && *ptr != NULL) {
        if (debug_track_allocation) {
            // First, try to zero out the memory if we can determine its size
            if (xSemaphoreTake(memorySemaphore, portMAX_DELAY) == pdTRUE) {
                for (size_t i = 0; i < memory_track_allocation_t; i++) {
                    if (memory_track_allocation[i].address == *ptr) {
                        // Overwrite the block with zeros before freeing
                        memset(*ptr, 0, memory_track_allocation[i].size);
                        break;
                    }
                }
                xSemaphoreGive(memorySemaphore);
            }
            // Now unregister it from our tracking list
            unregister_allocation(*ptr);
        }
         // Actually free the pointer
        heap_caps_free(*ptr);        // Free the actual memory
        *ptr = NULL;                 // Set the caller's pointer to NULL
    }
}

// Function to list all current allocations
void list_allocations() {
  if (debug_track_allocation){
    if (xSemaphoreTake(memorySemaphore, portMAX_DELAY) == pdTRUE) {
      for (size_t i = 0; i < memory_track_allocation_t; i++) {
        MemoryTraceInfo *info = &memory_track_allocation[i];
        printf("Memory allocated at %p, caps 0x%x, size %zu bytes, in %s, line %d, function %s, variable %s\n",
          info->address, info->caps, info->size, info->file, info->line, info->function, info->var_name );
      }
      xSemaphoreGive(memorySemaphore);
    }
  }
}


// Wrapper functions
void *traceable_heap_caps_malloc(size_t size, uint32_t caps, char *file, int line, char *function) {
    void *ptr = heap_caps_malloc(size, caps);
    int failure = (ptr) ? 0 : 1;
		if (failure){
				// printf("traceable_heap_caps_malloc size [%d]\n", size);
				log_memory_allocation((MemoryTraceInfo){file, line, function, size, (char *)"heap_caps_malloc", caps, failure});
		}
    if (debug_track_allocation){
      if (ptr) {
        // printf("[%d] ENTREI MALLOC\n",__LINE__ );
        register_allocation((MemoryTraceInfo){file, line, function, size, "heap_caps_malloc", caps, 0, ptr, (char *)"unknow"});
        // printf("[%d] SAI MALLOC\n",__LINE__ );
      }
    }
    return ptr;
}

void *traceable_heap_caps_calloc(size_t n, size_t size, uint32_t caps, const char *file, int line, const char *function) {
    void *ptr = heap_caps_calloc(n, size, caps);
		int failure = (ptr) ? 0 : 1;
		if (failure){
			// printf("traceable_heap_caps_calloc objects [%d] size [%d]\n",n, size);
    	log_memory_allocation((MemoryTraceInfo){file, line, function, n*size, (char *)"heap_caps_calloc", caps, failure});
		}
    if (debug_track_allocation){
      if (ptr) {
        // printf("[%d] ENTREI CALLOC \n",__LINE__ );
        register_allocation((MemoryTraceInfo){file, line, function, n*size, "heap_caps_calloc", caps, 0, ptr, (char *)"unknow"});
        // printf("[%d] SAI CALLOC\n",__LINE__ );
      }
    }
    return ptr;
}

void *traceable_heap_caps_realloc(void *ptr, size_t size, uint32_t caps, char *file, int line, char *function) {
    void *new_ptr = heap_caps_realloc(ptr, size, caps);
    int failure = (new_ptr == NULL) ? 1 : 0;  // Check the newly allocated pointer
    if (failure){
			    log_memory_allocation((MemoryTraceInfo){file, line, function, size, (char *)"heap_caps_realloc", caps, failure});
		}

    if (debug_track_allocation){
      // If the reallocation was successful and the pointer changed, update tracking
      if (new_ptr != ptr) {
        // printf("[%d] ENTREI realloc - unregsiter\n",__LINE__ );
        unregister_allocation(ptr); // Remove old pointer from tracking
        // printf("[%d] ENTREI sai - unregsiter\n",__LINE__ );
        // printf("[%d] ENTREI realloc - register\n",__LINE__ );
        register_allocation((MemoryTraceInfo){file, line, function, size, "heap_caps_realloc", caps, 0, new_ptr, "unknown"});
        // printf("[%d] sai realloc - register\n",__LINE__ );
      }
    }
    return new_ptr;
}

void print_memory_info(const char* tag) {
    printf("%s - Total free heap: %d bytes\n", tag, heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
    printf("%s - Free internal memory (DRAM): %d bytes\n", tag, heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    printf("%s - Free IRAM memory (executable): %d bytes\n", tag, heap_caps_get_free_size(MALLOC_CAP_EXEC));
    printf("%s - Free DMA-capable memory: %d bytes\n", tag, heap_caps_get_free_size(MALLOC_CAP_DMA));
    printf("%s - Free 8-bit accessible memory: %d bytes\n", tag, heap_caps_get_free_size(MALLOC_CAP_8BIT));
    printf("%s - Free 32-bit accessible memory: %d bytes\n", tag, heap_caps_get_free_size(MALLOC_CAP_32BIT));
    printf("%s - Free SPIRAM memory: %d bytes\n", tag, heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    printf("%s - Free IRAM 8-bit accessible memory: %d bytes\n", tag, heap_caps_get_free_size(MALLOC_CAP_IRAM_8BIT));
    printf("%s - Free retention memory (RTC fast): %d bytes\n", tag, heap_caps_get_free_size(MALLOC_CAP_RETENTION));
}
