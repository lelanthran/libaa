
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "aa.h"

struct bucket_t {
   size_t page_size;
   void *free_ptr;
   size_t free_bytes;
   void *base;
};

static struct bucket_t *bucket_new (struct bucket_t *dst, size_t page_size)
{
   memset (dst, 0, sizeof *dst);
   if (!(dst->base = calloc (1, page_size)))
      return NULL;

   dst->page_size = page_size;
   dst->free_ptr = dst->base;
   dst->free_bytes = page_size;

   return dst;
}

static void bucket_del (struct bucket_t *dst)
{
   free (dst->base);
   memset (dst, 0, sizeof *dst);
}


static void *bucket_alloc (struct bucket_t *bucket, size_t nbytes)
{
   if (bucket->free_bytes < nbytes)
      return NULL;

   void *ret = bucket->free_ptr;
   bucket->free_ptr += nbytes;
   bucket->free_bytes -= nbytes;
   return ret;
}



/* ******************************************************** */


struct aa_t {
   size_t nbuckets;
   size_t nallocs;
   size_t nbytes;
   size_t nvallocs;
   struct bucket_t *buckets;
};

size_t aa_metric_nbuckets (aa_t *arena)
{
   return arena->nbuckets;
}

size_t aa_metric_nallocs (aa_t *arena)
{
   return arena->nallocs;
}

size_t aa_metric_nvallocs (aa_t *arena)
{
   return arena->nvallocs;
}

size_t aa_metric_nbytes (aa_t *arena)
{
   return arena->nbytes;
}


aa_t *aa_init (size_t page_size)
{
   bool error = true;
   aa_t *ret = NULL;

   if (!(ret = calloc (1, sizeof *ret)))
      goto cleanup;

   if (!(ret->buckets = calloc (1, sizeof *ret->buckets)))
      goto cleanup;

   if (!(bucket_new (&ret->buckets[0], page_size)))
      goto cleanup;

   ret->nbuckets = 1;
   ret->nallocs = 3;
   ret->nbytes = sizeof *ret->buckets + page_size + sizeof *ret;

   error = false;

cleanup:
   if (error) {
      aa_destroy (&ret);
   }
   return ret;
}

void aa_destroy (aa_t **dst)
{
   if (!dst || !*dst)
      return;

   for (size_t i=0; i<(*dst)->nbuckets; i++) {
      bucket_del (&(*dst)->buckets[i]);
   }

   free ((*dst)->buckets);
   free (*dst);
   *dst = NULL;
}

static struct bucket_t *aa_add_bucket (aa_t *arena, size_t page_size)
{
   if (!arena)
      return NULL;

   size_t nlen = arena->nbuckets + 1;
   struct bucket_t *tmp = realloc (arena->buckets, (sizeof *arena->buckets) * nlen);
   if (!tmp)
      return NULL;
   arena->buckets = tmp;
   arena->nallocs++;
   arena->nbytes += sizeof *arena->buckets;

   struct bucket_t *bucket = &arena->buckets[arena->nbuckets];
   if (!(bucket_new (bucket, page_size)))
      return NULL;

   arena->nbuckets = nlen;
   return bucket;
}


void *aa_alloc (aa_t *arena, size_t nbytes)
{
   void *ret = NULL;
   if (!arena)
      return NULL;

   struct bucket_t *bucket = NULL;

   for (size_t i=0; i<arena->nbuckets; i++) {
      if (nbytes < arena->buckets[i].free_bytes) {
         bucket = &arena->buckets[i];
         break;
      }
   }

   if (!bucket) {
      if (!(bucket = aa_add_bucket (arena, nbytes * 4)))
         goto cleanup;
   }

   if (!(ret = bucket_alloc (bucket, nbytes)))
      goto cleanup;

   arena->nbytes += nbytes;
   arena->nvallocs++;
cleanup:
   return ret;
}

