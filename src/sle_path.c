#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "sle_path.h"

static void sle_alloc(sle_t *sle);
static void sle_gen_driving_fn(sle_t *sle, int rand_seed);
static void sle_set_starting_z(complex *z, sle_type_t type);
static void sle_compute_pt(sle_t *sle, sle_pt_t *pt);
static void sle_pt_adapt(sle_t *sle, sle_pt_t *pt_start, sle_pt_t *pt_end);
static void sle_trim_path(sle_t *sle);
static void sle_set_sample_times(sle_t *sle);

/* Atomic conformal maps */
static complex sle_cmap_slit_plane(double t, double u, complex z);
static complex sle_cmap_slit_disc(double t, double u, complex z);

void sle_init(sle_t *sle, sle_param_t sle_param)
{
  sle->param = sle_param;
  sle_alloc(sle);
}

void sle_free(sle_t *sle)
{
  sle_pt_t *curr = sle->pt_start, *next = NULL;
  while(curr != NULL){
    next = curr->next;
    free(curr);
    curr = next;
  }
}

/* Allocate and create a deep copy of sle_src */
sle_t* sle_deep_copy(sle_t *sle_src)
{
  sle_t *sle_dst = malloc(sizeof(sle_t));
  sle_pt_t *pt_curr_dst, *pt_curr_src;
  sle_dst->param = sle_src->param;

  if(!sle_src->pt_start) return sle_dst;

  sle_dst->pt_start = (sle_pt_t*) malloc(sizeof(sle_pt_t));
  pt_curr_dst = sle_dst->pt_start;
  pt_curr_src = sle_src->pt_start;

  pt_curr_dst->t = pt_curr_src->t;
  pt_curr_dst->u = pt_curr_src->u;
  pt_curr_dst->z = pt_curr_src->z;
  pt_curr_dst->prev = NULL;

  pt_curr_src = pt_curr_src->next;

  while(pt_curr_src != NULL){
    sle_pt_t *pt_buff = pt_curr_dst;
    pt_curr_dst->next = (sle_pt_t*)malloc(sizeof(sle_pt_t));
    pt_curr_dst = pt_curr_dst->next;
    pt_curr_dst->prev = pt_buff;

    pt_curr_dst->t = pt_curr_src->t;
    pt_curr_dst->u = pt_curr_src->u;
    pt_curr_dst->z = pt_curr_src->z;

    pt_curr_src = pt_curr_src->next;
  }

  pt_curr_dst->next = NULL;

  return sle_dst;
}

/* Generate a random SLE path */
void sle_gen_path(sle_t *sle, int rand_seed)
{
  sle_pt_t *pt_curr = sle->pt_start;

  sle_trim_path(sle);
  sle_set_sample_times(sle);
  sle_gen_driving_fn(sle, rand_seed);

  while(pt_curr != NULL){
    sle_compute_pt(sle, pt_curr);

    /* Adaptive refinement: ensure that distance between pt_curr->prev
     * and pt_curr is less than the parameter adapt_eps */
    if(pt_curr->prev){
      sle_pt_adapt(sle, pt_curr->prev, pt_curr);
    }

    pt_curr = pt_curr->next;
  }
}

/* Write SLE path as a list of "x, y" points to a .csv file */
void sle_print_path(sle_t *sle, FILE *fp)
{
  sle_pt_t *pt_curr = sle->pt_start;

  sle_print_param(sle, fp);

  while(pt_curr != NULL) {
    fprintf(fp, "%f,%f\n", creal(pt_curr->z), cimag(pt_curr->z));
    pt_curr = pt_curr->next;
  }
}

/* Print parameters to stdout */
void sle_print_param(sle_t *sle, FILE *fp)
{
  fprintf(fp, "# SLE parameters\n");
  fprintf(fp, "# type: ");
  switch(sle->param.type) {
    case(CHORDAL):
      fprintf(fp, "CHORDAL\n");
      break;
    case(RADIAL):
      fprintf(fp, "RADIAL\n");
      break;
    case(FULL_PLANE):
      fprintf(fp, "FULL_PLANE\n");
      break;
  }
  fprintf(fp, "# kappa: %f\n", sle->param.kappa);
  fprintf(fp, "# time_stop: %f\n", sle->param.time_stop);
  fprintf(fp, "# adapt_eps: %f\n", sle->param.adapt_eps);
  fprintf(fp, "# time_sample_rate: %d\n", sle->param.time_sample_rate);
}

/* Create SLE linked list and set the sample times for the SLE pts */
static void sle_alloc(sle_t *sle)
{
  int i;
  sle_pt_t *pt_curr;

  /* Create initial pt */
  sle->pt_start = (sle_pt_t*) malloc(sizeof(sle_pt_t));
  pt_curr = sle->pt_start;
  pt_curr->t = 0.0;
  pt_curr->prev = NULL;

  for(i = 1; i < floor(sle->param.time_sample_rate * sle->param.time_stop); ++i){
    /* Create successive pts */
    sle_pt_t *pt_buff = pt_curr;
    pt_curr->next = (sle_pt_t*)malloc(sizeof(sle_pt_t));
    pt_curr = pt_curr->next;
    pt_curr->prev = pt_buff;
  }

  pt_curr->next = NULL;
}

/* Generates the driving function, i.e. a sample path of
 * sqrt(kappa * Brownian motion). */
static void sle_gen_driving_fn(sle_t *sle, int rand_seed)
{
  sle_pt_t *pt_curr;

  srand48(rand_seed);
  pt_curr = sle->pt_start;
  pt_curr->u = 0.0;

  while(pt_curr->next != NULL){
    int incr;
    double u_next;

    /* Approximate Brownian motion by random walk with step size 1 */
    if(drand48() <= 0.5) {
      incr = -1;
    } else {
      incr = 1;
    }

    /* Compute value of driving function at SLE pt */
    u_next = pt_curr->u + incr * sqrt(sle->param.kappa *
             (1.0/sle->param.time_sample_rate));

    pt_curr = pt_curr->next;
    pt_curr->u = u_next;
  }
}

/* Chordal SLE paths start at the origin. Radial SLE paths
 * start at 1. Full plane SLE is the inversion of radial SLE,
 * so the initial starting point is also 1. */
static void sle_set_starting_z(complex *z, sle_type_t type)
{
  switch(type) {
    case(CHORDAL):
      *z = 0.0;
      break;
    case(RADIAL):
      *z = 1.0;
      break;
    case(FULL_PLANE):
      *z = 1.0;
      break;
  }
}

/* Computes the resulting z-value for an sle_pt under the appropriate
 * atomic conformal map. */
static void sle_compute_z(complex *z, sle_pt_t *pt, sle_type_t type)
{
  switch(type) {
    case(CHORDAL):
      *z = sle_cmap_slit_plane(pt->t - pt->prev->t,
                               pt->u - pt->prev->u,
                               *z);
      break;
    case(RADIAL):
      *z = sle_cmap_slit_disc(pt->t - pt->prev->t,
                              pt->u - pt->prev->u,
                              *z);
      break;
    case(FULL_PLANE):
      *z = 1.0/sle_cmap_slit_disc(pt->t - pt->prev->t,
                                  pt->u - pt->prev->u,
                                  1.0/(*z));
      break;
  }
}

/* Given values of t, u, for pt, and all SLE points on the linked
 * list prior to pt, this computes the value of z for pt */
static void sle_compute_pt(sle_t *sle, sle_pt_t *pt)
{
  complex z = 0;

  sle_set_starting_z(&z, sle->param.type);

  /* Uses the formula z = (f_1) o ...o (f_k)(starting point), where
     each of the f_i are the atomic maps approximated by slit maps
     (see README for derivation of this formula).
     Note that this cannot be memoized for performance, as first map
     applied (f_k) is different for each sle_pt */
  sle_pt_t *pt_iter = pt;
  while(pt_iter->prev != NULL) {
    sle_compute_z(&z, pt_iter, sle->param.type);
    pt_iter = pt_iter->prev;
  }

  pt->z = z;
}

/* Adaptive refinement of the SLE curve.
 * Recursively insert as many SLE midpoints between
 * pt_start and pt_end as necessary in order for the distance between
 * successive points to have distance at most sle->param.adapt_eps */
static void sle_pt_adapt(sle_t *sle, sle_pt_t *pt_start, sle_pt_t *pt_end)
{
  int incr;
  sle_pt_t *pt_mid;

  /* No refinement necessary */
  if(cabs(pt_end->z - pt_start->z) <= sle->param.adapt_eps) return;

  /* Avoid infinite loops caused by float imprecision */
  if(pt_end->t - pt_start->t <= 1.e-10) return;

  /* Create midpoint to be inserted */
  pt_mid = malloc(sizeof(sle_pt_t));
  pt_mid->t = 0.5 * (pt_start->t + pt_end->t);

  /* Update list pointers */
  pt_mid->prev = pt_start;
  pt_mid->next = pt_end;
  pt_start->next = pt_mid;
  pt_end->prev = pt_mid;

  /* The value of u for pt_mid is a sample of a Brownian bridge at the
   * midpoint, with values pt_start->u at time 0 and pt_end->u at time
   * (pt_end->t - pt_start->t) */

  /* Set random direction for random noise */
  if(drand48() <= 0.5) {
    incr = -1;
  } else {
    incr = 1;
  }

  /* Brownian bridge sample is given by the midpoint plus random noise
   * with variance proportional to sqrt(kappa) */
  pt_mid->u = 0.5 * (pt_start->u + pt_end->u)
              + incr * sqrt(sle->param.kappa
              * 0.5 * (pt_end->t - pt_start->t));

  /* Recompute SLE curve with new values */
  sle_compute_pt(sle, pt_mid);
  sle_compute_pt(sle, pt_end);

  /* Recursively adapt further, if necessary */
  sle_pt_adapt(sle, pt_start, pt_mid);
  sle_pt_adapt(sle, pt_mid, pt_end);
}

/* Removes excess nodes in the sle_pt linked list.
 * Since we reuse the same list multiple times, new nodes will be
 * inserted during the adaptive refinement step.
 * This resets the size of the list. */
static void sle_trim_path(sle_t *sle)
{
  int curr_index = 0;
  int list_size = floor(sle->param.time_sample_rate
                        * sle->param.time_stop);

  sle_pt_t *pt_curr = sle->pt_start, *pt_next = NULL;

  while(pt_curr != NULL){
    pt_next = pt_curr->next;

    if(curr_index >= list_size - 1){
      pt_curr->next = NULL;
    }

    if(curr_index >= list_size) {
      free(pt_curr);
    }

    pt_curr = pt_next;
    ++curr_index;
  }
}

/* Sample SLE path points uniformly at intervals of length
 * 1/time_sample_rate */
static void sle_set_sample_times(sle_t *sle)
{
  sle_pt_t *pt_curr = sle->pt_start;
  pt_curr->t = 0;
  pt_curr = pt_curr->next;

  while(pt_curr != NULL) {
    pt_curr->t = pt_curr->prev->t + 1.0/sle->param.time_sample_rate;
    pt_curr = pt_curr->next;
  }
}

/* Conformal map z -> sqrt(z^2 - 4t) + u
 * Takes the half plane minus a vertical slit at u with height 2*sqrt(t)
 * onto the upper half plane. Approximates the atomic maps for chordal
 * SLE. */
static complex sle_cmap_slit_plane(double t, double u, complex z)
{
  complex ret;
  double x = creal(z);
  double y = cimag(z);
  double v = x*x - y*y - 4*t;
  double w = 2*y*x;
  double d = sqrt(v*v + w*w);

  /* Speedup to compute real, imaginary parts of sqrt(z^2 - 4t)
     without arctan(), taken from Kennedy's code */
  x = sqrt((v + d)/2.0);
  y = sqrt((-v + d)/2.0);

  /* Note that sqrt(z^2 - 4t) takes right hand side to right hand side
   * and left to left. so flip the sign of the square root when z has
   * negative real part */
  if(creal(z) < 0){
    x = -x;
  }

  ret = x + y*I + u;
  return ret;
}

/* Conformal map that takes the unit disc onto the disc minus a slit
 * starting at cexp(I * u). Approximates the atomic maps for radial
 * SLE, and the reciprocal of this approximates the atomic maps for
 * full plane SLE. */
static complex sle_cmap_slit_disc(double t, double u, complex z)
{
  complex rotation = cexp((I * u));
  complex w = 4 * cexp(-t) * z;
  complex v = z + 1;
  return rotation * 1.0/w * (2 * v * v - w - 2 * v * csqrt(v * v - w));
}

