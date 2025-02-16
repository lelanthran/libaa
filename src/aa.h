/* Simple arena allocator */

#ifndef H_AA
#define H_AA

typedef struct aa_t aa_t;

#ifdef __cplusplus
extern "C" {
#endif

   // Create an arena allocator with specified page_size. The caller should use the
   // guideline of (avg_alloc_size * expected_allocs) where:
   //    avg_alloc_size = expected average of the allocations made, in bytes
   //    expected_allocs = expected number of allocations to be made
   aa_t *aa_init (size_t page_size);

   // Free all memory allocated by this arena
   void aa_destroy (aa_t **dst);

   // Allocate some memory; equivalent to `malloc()`
   void *aa_alloc (aa_t *arena, size_t nbytes);

   // No `realloc()` for now. Since this is an arena the caller should just
   // allocate a new block and forget about the existing block.

   // Number of buckets created
   size_t aa_metric_nbuckets (aa_t *arena);

   // Number of real allocations made
   size_t aa_metric_nallocs (aa_t *arena);

   // Number of allocations caller made
   size_t aa_metric_nvallocs (aa_t *arena);

   // Number of bytes used
   size_t aa_metric_nbytes (aa_t *arena);

#ifdef __cplusplus
};
#endif


#endif


