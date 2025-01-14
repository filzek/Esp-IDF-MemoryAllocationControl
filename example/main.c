#include "memmap.h"

void app_main(void) {
    // Create a mutex for thread-safe tracking
    memorySemaphore = xSemaphoreCreateMutex();

    // Enable memory allocation debug tracking
    debug_track_allocation = true;

    // Allocate memory with tracking
  
    char *buffer = TRACEABLE_HEAP_CAPS_CALLOC(256, sizeof(char),MALLOC_CAP_8BIT);
    if (!buffer) {
        printf("Failed to allocate buffer!\n");
        return;
    }

    // Use the buffer as needed
    snprintf(buffer, 256, "Hello from memory tracking!");

    char *debug_string = TRACEABLE_HEAP_CAPS_CALLOC(512, sizeof(char),MALLOC_CAP_SPIRAM);
    if (!debug_string) {
          printf("Failed to allocate debug_string!\n");
          return;
    }
  

    // Print current allocations
    list_allocations();

    // Free the buffer and unregister
    free_unregister_allocation((void **)&buffer);

    // Confirm it's removed from tracking
    list_allocations();
  
    // Free the debug_string and unregister
    free_unregister_allocation((void **)&debug_string);

    // Confirm it's removed from tracking
    list_allocations();
  
}
