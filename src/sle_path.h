/*
 * This contains functions for generating random
 * SLE paths and computing various random variables.
 * See main file for examples of its use.
 *
 */

#ifndef SLE_PATH_H
#define SLE_PATH_H

#include <complex.h>

typedef struct sle sle_t;
typedef struct sle_param sle_param_t;
typedef struct sle_pt sle_pt_t;

typedef enum {
  CHORDAL,
  RADIAL,
  FULL_PLANE
} sle_type_t;

struct sle_param {
  /* Usual kappa parameter for SLE, controls the variation
   * of the path */
  double kappa;

  /* Generate Brownian motion from time 0 until time_stop */
  double time_stop;

  /* Consecutive points on SLE path are of distance at most
   * adapt_eps from each other. Otherwise, an adaptive
   * refinement is called */
  double adapt_eps;

  /* Number of samples of Brownian motion per unit time */
  unsigned int time_sample_rate;

  sle_type_t type;
};

/* A discrete SLE path is represented as a doubly linked list of points */
struct sle_pt {
  sle_pt_t *next;       /* Next point on SLE path, i.e. pt at time
                           t + 1/time_sample_rate */

  sle_pt_t *prev;       /* Prev point on SLE path, i.e. pt at time
                           t - 1/time_sample_rate */

  double t;             /* Time value corresponding to pt */
  double u;             /* Value of driving function at time t */
  complex z;            /* Tip of SLE path in complex plane at time t */
};

struct sle {
  sle_pt_t *pt_start;   /* Head of the SLE linked list */
  sle_param_t param;
};

void sle_init(sle_t *sle, sle_param_t sle_param);
void sle_free(sle_t *sle);

sle_t* sle_deep_copy(sle_t *sle_src);

void sle_gen_path(sle_t *sle, int rand_seed);
void sle_print_path(sle_t *sle, FILE *fp);
void sle_print_param(sle_t *sle, FILE *fp);

#endif /* SLE_PATH_H */
