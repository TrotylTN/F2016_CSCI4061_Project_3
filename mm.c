#include <stdio.h>

#include "mm.h"
#include <string.h>

/* Return usec */
double comp_time(struct timeval time_s, struct timeval time_e) {

  double elap = 0.0;

  if (time_e.tv_sec > time_s.tv_sec) {
    elap += (time_e.tv_sec - time_s.tv_sec - 1) * 1000000.0;
    elap += time_e.tv_usec + (1000000 - time_s.tv_usec);
  }
  else {
    elap = time_e.tv_usec - time_s.tv_usec;
  }
  return elap;
}

int mm_init(mm_t *mm, int hm, int sz) {
  int i;
  if (mm == NULL) { // bad address
    errno = EFAULT;
    return -1;
  }
  mm->unit_size = sz;
  mm->tot = hm;
  mm->used_cnt = 0;
  mm->avail_pos = 0;
  mm->del_pos = 0;
  mm->pool = NULL;
  mm->used_list = NULL;

  mm->pool = malloc(hm * sz);
  if (mm->pool == NULL) { // init memory pool error
    // errno is set
    return -1;
  }
  mm->used_list = (char *) malloc(hm * sizeof(char));
  if (mm->used_list == NULL) { // init used list error
    //errno is set
    free(mm->pool);
    return -1;
  }
  memset(mm->used_list, 0, hm * sizeof(char));
  return 0;
}

void *mm_get(mm_t *mm) {
  int i;
  if (mm == NULL) { // bad address
    errno = EFAULT;
    return NULL;
  }
  if (mm->used_cnt >= mm->tot) { // all memory are used
    errno = ENOMEM;
    return NULL;
  }
  for (i = mm->avail_pos; i < mm->tot; i++) {
    if (mm->used_list[i] == 0) {
      mm->used_list[i] = 1;
      mm->used_cnt++;
      mm->avail_pos = i + 1;
      return mm->pool + (i * mm->unit_size);
    }
  }
  for (i = 0; i < mm->avail_pos; i++) {
    if (mm->used_list[i] == 0) {
      mm->used_list[i] = 1;
      mm->used_cnt++;
      mm->avail_pos = i + 1;
      return mm->pool + (i * mm->unit_size);
    }
  }
  // it should have a non-used memory but we cannot find it
  // Please try again :)
  errno = EAGAIN;
  return NULL;
}

void mm_put(mm_t *mm, void *chunk) {
  int i;
  if (mm == NULL) { // bad address
    errno = EFAULT;
    return;
  }
  if (chunk == NULL) { // bad address
    errno = EFAULT;
    return;
  }
  for (i = mm->del_pos; i < mm->tot; i++) {
    if (mm->pool + (i * mm->unit_size) == chunk) {
      mm->used_list[i] = 0;
      mm->used_cnt--;
      mm->del_pos = i + 1;
      return;
    }
  }
  for (i = 0; i < mm->del_pos; i++) {
    if (mm->pool + (i * mm->unit_size) == chunk) {
      mm->used_list[i] = 0;
      mm->used_cnt--;
      mm->del_pos = i + 1;
      return;
    }
  }
  return;
}

void mm_release(mm_t *mm) {
  free(mm->pool);
  free(mm->used_list);
  mm->unit_size = 0;
  mm->tot = 0;
  mm->used_cnt = 0;
  mm->avail_pos = 0;
  mm->del_pos = 0;
  return;
}

#define TESTNM 1000000

void timer_ours() {
  struct timeval time_s, time_e;
  int i;
  /* start timer */
  gettimeofday (&time_s, NULL);
  mm_t mm;
  mm_init(&mm, TESTNM, 1);

  void* test[TESTNM];
  for (i = 0; i < TESTNM; i++) {
    test[i] = mm_get(&mm);
  }
  for (i = 0; i < TESTNM; i++) {
    mm_put(&mm, test[i]);
  }

  mm_release(&mm);
  gettimeofday(&time_e, NULL);

  fprintf(stderr, "Our Memory Manager Time taken = %f msec\n",
          comp_time(time_s, time_e) / 1000.0);
}

void timer_origin() {
  struct timeval time_s, time_e;
  int i;
  /* start timer */
  gettimeofday (&time_s, NULL);
  void* test[TESTNM];
  for (i = 0; i < TESTNM; i++)
    test[i] = malloc(1);
  for (i = 0; i < TESTNM; i++)
    free(test[i]);

  gettimeofday(&time_e, NULL);

  fprintf(stderr, "Native Malloc Time taken = %f msec\n",
          comp_time(time_s, time_e) / 1000.0);
}

// int main() {
//   timer_ours();
//   timer_origin();
// }
