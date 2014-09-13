#include <string.h>

#include "sle_arg.h"

static error_t parse_opt(int key, char *arg, struct argp_state *state);

/* Wrapper for argp_parse */
error_t sle_parse(int argc, char *argv[], sle_arg_t *sle_arg)
{
  const char args_doc[] = "SLE_PATH_FILE RV_VALUE_FILE";
  const char doc[] = "sle_mc -- A program for Monte "
               "Carlo Simulations of SLE Curves";
  argp_parser_t parser = &parse_opt;

  struct argp_option options[] =
    {
      {"verbose",          'v', 0, 0, "Produce verbose output"},
      {"samples_total",    's', "FLOAT", 0,
       "Sets the total number of samples to generate"},
      {"threads_total",    't', "FLOAT", 0,
       "Sets the total number of threads to use"},
      {"kappa",            'k', "FLOAT", 0,
       "Sets the value of SLE parameter kappa"},
      {"time_sample_rate", 'r', "FLOAT", 0,
       "Sets how many samples of the curve per unit time"},
      {"time_stop",        'o', "FLOAT", 0,
       "Sets the time to stop the SLE curve"},
      {"adapt_eps",        'e', "FLOAT", 0,
       "Sets the adaptive refinement parameter"},
      {"type",             'y', "INT", 0,
       "Sets the type of SLE curve"},
      {0}
    };

  struct argp argp = {options, parser, args_doc, doc};

  /* Default file I/O */
  sle_arg->args[0] = "path.csv";
  sle_arg->args[1] = "rv.csv";

  /* Default Monte Carlo parameters */
  sle_arg->samples_total = 100;
  sle_arg->threads_total = 1;

  /* Default SLE path parameters */
  sle_arg->sle_param.kappa = 8.0/3.0;
  sle_arg->sle_param.time_sample_rate = 1000;
  sle_arg->sle_param.time_stop = 1.0;
  sle_arg->sle_param.adapt_eps = 0.01;
  sle_arg->sle_param.type = CHORDAL;

  return argp_parse(&argp, argc, argv, 0, 0, sle_arg);
}

/* Standard parse_opt function for argp_parse */
error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  sle_arg_t *sle_arg_curr = state->input;
  char sle_type[16] = "";

  switch(key) {
    case 'v':
      sle_arg_curr->verbose = 1;
      break;
    case 's':
      sscanf(arg, "%d", &sle_arg_curr->samples_total);
      break;
    case 't':
      sscanf(arg, "%d", &sle_arg_curr->threads_total);
      break;
    case 'k':
      sscanf(arg, "%lf", &sle_arg_curr->sle_param.kappa);
      break;
    case 'r':
      sscanf(arg, "%d", &sle_arg_curr->sle_param.time_sample_rate);
      break;
    case 'o':
      sscanf(arg, "%lf", &sle_arg_curr->sle_param.time_stop);
      break;
    case 'e':
      sscanf(arg, "%lf", &sle_arg_curr->sle_param.adapt_eps);
      break;
    case 'y':
      sscanf(arg, "%s", sle_type);
      /* Use strstr because sometimes the argument may be
       * "=CHORDAL" as opposed to "CHORDAL", depending
       * on if --type or -y is used */
      if(strstr(sle_type, "CHORDAL")){
        sle_arg_curr->sle_param.type = CHORDAL;
      } else if(strstr(sle_type, "RADIAL")){
        sle_arg_curr->sle_param.type = RADIAL;
      } else if(strstr(sle_type, "FULL_PLANE")){
        sle_arg_curr->sle_param.type = FULL_PLANE;
      }
      break;
    case ARGP_KEY_ARG:
      if(state->arg_num >=2) {
        /* Too many arguments, state usage and exit*/
        argp_usage(state);
      } else {
        /* Set appropriate argument values */
        sle_arg_curr->args[state->arg_num] = arg;
      }
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}
