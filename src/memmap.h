#ifndef RUN_ONCE_HEADER
#define RUN_ONCE_HEADER 1
void *traceable_heap_caps_malloc(            size_t size, uint32_t caps, char *file, int line, char *function);
void *traceable_heap_caps_calloc(size_t n  , size_t size, uint32_t caps, const char *file, int line, const char *function);
//void *traceable_heap_caps_calloc(size_t n  , size_t size, uint32_t caps, char *file, int line, char *function);
void *traceable_heap_caps_realloc(void *ptr, size_t size, uint32_t caps, char *file, int line, char *function);
#endif

#ifndef TRACEABLE_HEAP_CAPS_MALLOC
#define TRACEABLE_HEAP_CAPS_MALLOC(size, caps) traceable_heap_caps_malloc(size, caps, __FILE__, __LINE__, __FUNCTION__)
#endif

#ifndef TRACEABLE_HEAP_CAPS_CALLOC
#define TRACEABLE_HEAP_CAPS_CALLOC(n, size, caps) traceable_heap_caps_calloc(n, size, caps, __FILE__, __LINE__, __FUNCTION__)
#endif

#ifndef TRACEABLE_HEAP_CAPS_REALLOC
#define TRACEABLE_HEAP_CAPS_REALLOC(ptr, size, caps) traceable_heap_caps_realloc(ptr, size, caps, __FILE__, __LINE__, __FUNCTION__)
#endif
