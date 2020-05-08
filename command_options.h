#ifndef COMMAND_OPTIONS_H
#define COMMAND_OPTIONS_H 1

// Command options
#include <argp.h>
#include <stdbool.h>
#include <string.h>

const char *argp_program_version = "bigint_sort 1.0";

const char *argp_program_bug_address = "wjwray@gmail.com";

static char doc[] = "bigint_sort: Sort a long list of massive integers.";

static char args_doc[] = "Big sort";

static struct argp_option options[] = {
    { "interactive", 'i', 0, 0, "Interactive mode with text UI."},
    { "file", 'f', "filename", 0, "Input filename."},
    { "quicksort", 'q', 0, 0, "Set sort algo to quicksort."},
    { "mergesort", 'm', 0, 0, "Set sort algo to mergesort."},
    { "heapsort", 'h', 0, 0, "Set sort algo to heapsort."},
    { "pthreads", 'h', 0, 0, "Switch threading On/oFf."},
    { 0 } 
};

// arguments struct, also used by interactive user interface
//
typedef struct {
  char filename[64];
  enum { QUICKSORT = 'q', MERGESORT = 'm', HEAPSORT = 'h' } sort_algo;
  bool interactive;
  bool pthreaded;
} arguments;

arguments default_args() {
  arguments args = {
    .filename = {},
    .sort_algo = QUICKSORT,
    .interactive = false,
    .pthreaded = false
  };
  return args;
}

const char* get_filename(arguments* a) {
  return a->filename;
}

const char* set_sort_algo(arguments* a, char c) {
  switch(a->sort_algo = c) {
    default:
    case QUICKSORT: return "quicksort";
    case MERGESORT: return "mergesort";
    case HEAPSORT:  return "heapsort ";
  }
}

const char* get_sort_algo(arguments* a) {
  return set_sort_algo(a,a->sort_algo);
}

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  arguments *args = state->input;
  switch (key) {
    case 'i': args->interactive = true; 
              break;
    case 'f': if (strlen(arg) < 64) strcpy(args->filename, arg);
              break;
    case 'q':
    case 'm':
    case 'h': set_sort_algo(args,key);
              break;
    case 'p': args->pthreaded = ! args->pthreaded;
              break;
    case ARGP_KEY_ARG: return 0;
    default: return ARGP_ERR_UNKNOWN;
  }   
  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

#endif