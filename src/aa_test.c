
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aa.h"

// In this example program we expect to make around 100
// allocations average 32 bytes per allocation.
//
// Overshooting the average results in additional buckets
// are created while undershooting results in memory wastage.
//
// Either way, no memory errors are going to be generated.
#define AVG_ALLOC       ((size_t)(32))
#define PAGE_SIZE       ((size_t)(AVG_ALLOC * 100))

// Number of tests we are going to run
#define NALLOCS         ((size_t)64)

int main(void)
{
   int ret = EXIT_FAILURE;

   aa_t *arena = NULL;
   if (!(arena = aa_init (PAGE_SIZE))) {
      printf ("Failed to initialise arena allocator\n");
      goto cleanup;
   }

   // Deterministic random
   srand (0);

   struct alloc_t {
      size_t nbytes;
      void *ptr;
   };

   static struct alloc_t allocs[NALLOCS];

   for (size_t i=0; i<sizeof allocs/sizeof allocs[0]; i++) {
      allocs[i].nbytes = rand() % AVG_ALLOC;
      allocs[i].ptr = aa_alloc (arena, allocs[i].nbytes);
      memset (allocs[i].ptr, 0x5a, allocs[i].nbytes);
   }

   printf ("nbuckets:   %zu\n", aa_metric_nbuckets (arena));
   printf ("nallocs:    %zu\n", aa_metric_nallocs (arena));
   printf ("nvallocs:   %zu\n", aa_metric_nvallocs (arena));
   printf ("nbytes:     %zu\n", aa_metric_nbytes (arena));

   ret = EXIT_SUCCESS;
cleanup:
   aa_destroy (&arena);
   return ret;
}

