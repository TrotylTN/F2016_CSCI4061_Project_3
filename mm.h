#ifndef __MM_H
#define __MM_H

#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define CHUNK_SIZE 64
#define NUM_CHUNKS 1000000

typedef struct {
  int unit_size;
  int tot;
  int used_cnt;    // count of used memory blocks
  int avail_pos;   // the position start to find available memory
  void* pool;      // memory pool
  char* used_list; // the list indicate which memory is used
} mm_t;

double comp_time(struct timeval time_s, struct timeval time_e);
int mm_init(mm_t *mm, int num_chunks, int chunk_size);
void *mm_get(mm_t *mm);
void mm_put(mm_t *mm, void *chunk);
void mm_release(mm_t *mm);

#endif
