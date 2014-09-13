#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "sle_rv.h"

double rv_horiz_dist_ratio(const sle_t *sle)
{
  double dist = FLT_MAX;
  double c = 1;
  sle_pt_t *pt_curr = sle->pt_start;

  while(pt_curr != NULL){
    dist = fmin(dist, cabs(pt_curr->z - c));
    pt_curr = pt_curr->next;
  }

  return dist;
}

double rv_num_steps_greater_than_eps(const sle_t *sle)
{
  double eps = 0.01, count = 0;
  sle_pt_t *pt_curr = sle->pt_start->next;

  while(pt_curr != NULL){
    if(cabs(pt_curr->z - pt_curr->prev->z) > eps){
      ++count;
    }
    pt_curr = pt_curr->next;
  }
  return count;
}

void rv_append_value(const sle_t *sle, const rand_var rv, FILE *fp)
{
  double rv_val= (*rv)(sle);
  fprintf(fp, "%f\n", rv_val);
}



