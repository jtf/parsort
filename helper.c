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


int max_array(int * a, int num)
{
  int i, position=1, max=0;
  for (i=1; i<num; i++)
    {
      if (a[i]>max)
	{
	  max=a[i];
	  position=i;
	}
    }
  return(position);
}


//min nur nach max aufrufen!!
int min_array(int * a, int num, int maxPos)
{
	
  int i, position=1, min=a[maxPos];
  for (i=1; i<num; i++)
    {
      if ((a[i]<=min) && (a[i]>0))
	{
	  min=a[i];
	  position=i;
	}
    }
  return(position);
}


int slavesReady(int * a, int num)
{
  int ret=0, i;
  for(i=1; i<num; i++)
    {
      if(a[i]>0)
	ret++;
    }
  if(ret>=2)
    return 1;
  else
    return 0;
}


//InterSlaveMerge: mindestens 1 Slave noch beim Mergen?
int slaveBusy(int * a, int num)
{
  int i;
  for(i=1; i<num; i++)
    {
      if(a[i] == 0)
	return 1;
    }
  return 0;		
}


//InterSlaveMerge: mindestens 3 Slaves lebendig?
int moreThanTwoSlaves(int * a, int num)
{
  int rest=num-1, i;
  for(i=1; i<num; i++)
    {
      if(a[i] == SLAVE_DEAD)
	rest--;
    }
  if(rest == 2)
    return 0;
  else
    return 1;
}


//Mastermerge: Slave zurückgeben
int getReadySlave(int * a, int num)
{
  int i;
  for(i=0; i<num; i++)
    {
      if(a[i] > SLAVE_BUSY)
	{
	  return i;
	}
    }
}
