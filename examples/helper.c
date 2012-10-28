#include <stdlib.h>
#include <argp.h>

/* Used by main to communicate with parse_opt. */
struct config
{
  int volumesize;
  int chunksize;
  int cachesize;
  int qsort;
  int msort;
  int isort;
};

/* The options we understand. */
// OPTIONS  -- A pointer to a vector of struct argp_option
static struct argp_option options[] = {
  {"volume", 'v', "VOLUMESIZE", 0, "volume size (k * chunk size, k>4)", 1},
  {"chunk",  'c', "CHUNKSIZE",  0, "chunk size > 3", 1},
  {"cache",  'C', "CACHESIZE", 0, "L3 cache size for insertion sort", 2 },
  {"qsort",  'q', 0, 0, "use local quick sort", 3},
  {"msort",  'm', 0, 0, "use local merge sort", 3},
  {"misort", 'i', 0, 0, "use local merge with insertion sort", 3},
  { 0 }
};

/* Program documentation. */
static char doc[] = "MPI based merge sort on multiple nodes\v(c)2012";
     
/* A description of the non-option arguments we accept. */
static char args_doc[] = "";
const char *argp_program_version = "1.0";
const char *argp_program_bug_address = "<bug@example.com>";
  
/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct config *cfg = state->input;
     
  switch(key)
    {
    case 'v':// case 's':
      cfg->volumesize = atoi(arg);
      break;
    case 'c':
      cfg->chunksize = atoi(arg);
      break;
    case 'C':
      cfg->cachesize = atoi(arg);
      break;
      /* alle Argumente geparst*/
    case ARGP_KEY_END:
      if (cfg->volumesize < 0 || cfg->chunksize <0
	  || (cfg->volumesize % cfg->chunksize) != 0)
	{
	  printf("Illegal values for chunk and volume sizes\n");
	  argp_usage(state);
	}
      else if (cfg->volumesize <= cfg->chunksize)
	{
	  printf("VOLUMESIZE <= CHUNKSIZE\n");
	  argp_usage(state);
	}
	break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Our argp parser. */
static struct argp argopts = {options, parse_opt, args_doc, doc};

void argparse(int argc, char **argv, struct config *cfg)
{
  /* Parse our arguments; every option seen by parse_opt will
     be reflected in arguments. */
  argp_parse(&argopts, argc, argv, ARGP_NO_ARGS, 0, cfg);
}

