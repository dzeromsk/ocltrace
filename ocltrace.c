// Copyright (c) 2016 Dominik Zeromski <dzeromsk@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#define _GNU_SOURCE /* for RTLD_NEXT */

#include <argp.h>
#include <dlfcn.h>
#include <linux/limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <unistd.h>
#include <inttypes.h>

#include "opencl.h"

extern void *_dl_sym(void *, const char *, void *);

extern void *opencl_fptr[];
extern void *trace_fptr[];
extern const char *func_names[];
extern struct stats func_stats[];

struct arguments {
  int verbose;
  int time;
  int stats;
  int time_spent;
  char *output_file;
  char *library;
  char **argv;
};

struct handle {
  void *handle;
  LIST_ENTRY(handle) l;
};

static LIST_HEAD(listhead, handle) handles;
FILE *fout = NULL;
int emit_time = 0;
int emit_time_spent = 0;
int emit_stats = 0;
pthread_mutex_t lock;
void *libopencl_handle = NULL;

const char *argp_program_version = "0.8";
const char *argp_program_bug_address = "Dominik Zeromski <dzeromsk@gmail.com>";

static char doc[] = "OpenCL call tracer -- like strace but for OpenCL API";

static char args_doc[] = "<PROG> [<ARGS>]";

static struct argp_option options[] = {
    {"output", 'o', "FILE", 0, "Output to FILE instead of standard output"},
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"library", 'l', "LIBRARY", 0, "Name of the OpenCL library"},
    {"time", 't', 0, 0, "Prefix each line of the trace with timestamp"},
    {0, 'T', 0, 0, "Show the time spent in system calls"},
    {"statistics", 'c', 0, 0,
     "Count time and calls for each func call and report a summary"},
    {0, 'C', 0, 0, "Like -c but also print regular output"},
    {0},
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
    case 'v':
      arguments->verbose = 1;
      break;
    case 't':
      arguments->time = 1;
      break;
    case 'T':
      arguments->time_spent = 1;
      break;
    case 'c':
      arguments->stats = 1;
      break;
    case 'C':
      arguments->stats = 2;
      break;
    case 'o':
      arguments->output_file = arg;
      break;
    case 'l':
      arguments->library = arg;
      break;
    case ARGP_KEY_NO_ARGS:
      argp_usage(state);
    case ARGP_KEY_ARG:
      arguments->argv = &state->argv[state->next - 1];
      state->next = state->argc;
    case ARGP_KEY_END:
      return 0;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

static void __attribute__((constructor)) init(void) {
  // initialize only when loaded as a LD_PRELOAD library
  if (getenv("LD_PRELOAD") == NULL || getenv("OCLTRACE") == NULL) {
    return;
  }

  if (pthread_mutex_init(&lock, NULL) != 0) {
    fprintf(stderr, "mutex_init error");
    return;
  }

  const char *opencl_library = getenv("OCLTRACE_LIBRARY");
  if (opencl_library == NULL) {
    opencl_library = "libOpenCL.so";

    void *addr = dlsym(RTLD_NEXT, "clGetPlatformIDs");
    if (addr != NULL) {
      Dl_info info;
      if (dladdr(addr, &info) != 0) {
        opencl_library = info.dli_fname;
      }
    }
  }

  // load target library
  void *opencl_handle = dlopen(opencl_library, RTLD_LAZY);
  if (opencl_handle == NULL) {
    fprintf(stderr, "dlopen error\n");
    pthread_mutex_unlock(&lock);
    exit(1);
  }

  // load intercept library
  void *trace_handle = dlopen(getenv("LD_PRELOAD"), RTLD_LAZY);
  if (trace_handle == NULL) {
    fprintf(stderr, "dlopen error\n");
    pthread_mutex_unlock(&lock);
    exit(1);
  }

  // setup function pointers
  int i;
  for (i = 0; func_names[i]; i++) {
    opencl_fptr[i] = dlsym(opencl_handle, func_names[i]);
    trace_fptr[i] = dlsym(trace_handle, func_names[i]);
  }

  libopencl_handle = opencl_handle;
  fout = stderr;

  // if specified write trace output to file
  char *output_file = getenv("OCLTRACE_OUTPUT_FILE");
  if (output_file != NULL) {
    FILE *f = fopen(output_file, "w");
    if (f != NULL) {
      fout = f;
    }
  }

  if (getenv("OCLTRACE_TIME") != NULL) {
    emit_time = 1;
  }

  if (getenv("OCLTRACE_TIME_SPENT") != NULL) {
    emit_time_spent = 1;
  }

  char *stats = getenv("OCLTRACE_STATS");
  if (stats != NULL) {
    emit_stats = atol(stats);
  }

  if (emit_stats == 1) {
    FILE *f = fopen("/dev/null", "w");
    if (f != NULL) {
      fout = f;
    }
  }
}

int stats_cmp(const void *l, const void *r) {
  struct stats *a = (struct stats *)l;
  struct stats *b = (struct stats *)r;

  if (a->seconds > b->seconds) return -1;
  if (a->seconds < b->seconds) return 1;
  return b->seconds - a->seconds;
}

static void __attribute__((destructor)) fini(void) {
  if (getenv("LD_PRELOAD") == NULL || getenv("OCLTRACE") == NULL) {
    return;
  }

  pthread_mutex_lock(&lock);
  struct handle *h = NULL;
  LIST_FOREACH(h, &handles, l) {
    LIST_REMOVE(h, l);
    free(h);
  }
  pthread_mutex_unlock(&lock);

  if (fout != NULL && fout != stderr) {
    fclose(fout);
  }

  pthread_mutex_destroy(&lock);

  if (emit_stats) {
    int i;
    struct stats sum = {0};

    fprintf(stderr, "Summary:\n");
    fprintf(stderr, "%% time     seconds      calls     errors function\n");
    fprintf(stderr,
            "------ ----------- ---------- ---------- "
            "------------------------------\n");
    for (i = 0; func_names[i]; i++) {
      if (func_stats[i].calls) {
        sum.calls += func_stats[i].calls;
        sum.errors += func_stats[i].errors;
        sum.seconds += func_stats[i].seconds;
      }
    }

    qsort(func_stats, i, sizeof(struct stats), stats_cmp);

    for (i = 0; func_names[i]; i++) {
      if (func_stats[i].calls) {
        fprintf(stderr, " %5.2f %11.6f %10d %10d %s\n",
                func_stats[i].seconds / sum.seconds * 100,
                func_stats[i].seconds, func_stats[i].calls,
                func_stats[i].errors, func_names[func_stats[i].name]);
      }
    }
    fprintf(stderr,
            "------ ----------- ---------- ---------- "
            "------------------------------\n");
    fprintf(stderr, "100.00 %11.6f %10d %10d total\n", sum.seconds, sum.calls,
            sum.errors);
  }
}

void *dlsym(void *handle, const char *name) {
  pthread_mutex_lock(&lock);
  // initialize on first call
  static void *(*libdl_dlsym)(void *, const char *) = NULL;
  if (libdl_dlsym == NULL) {
    libdl_dlsym = _dl_sym(RTLD_NEXT, "dlsym", dlsym);
  }

  if (handle == NULL) {
    // use the default library search order
    void *ret = libdl_dlsym(RTLD_DEFAULT, name);
    if (ret != NULL) {
      pthread_mutex_unlock(&lock);
      return ret;
    }

    // fall-back to iterating over all dlopened libraries
    struct handle *h = NULL;
    LIST_FOREACH(h, &handles, l) {
      ret = libdl_dlsym(h->handle, name);
      if (ret != NULL) {
        pthread_mutex_unlock(&lock);
        return ret;
      }
    }

    pthread_mutex_unlock(&lock);
    return libdl_dlsym(NULL, name);
  }

  // intercept only calls to libopencl
  if (handle != libopencl_handle) {
    pthread_mutex_unlock(&lock);
    return libdl_dlsym(handle, name);
  }

  // find if we support this function
  int i;
  for (i = 0; func_names[i]; i++) {
    if (!strcmp(name, func_names[i])) {
      pthread_mutex_unlock(&lock);
      return trace_fptr[i];
    }
  }

  pthread_mutex_unlock(&lock);
  return libdl_dlsym(handle, name);
}

void *dlopen(const char *filename, int flag) {
  // initialize on first call
  static void *(*libdl_dlopen)(const char *, int) = NULL;
  if (libdl_dlopen == NULL) {
    libdl_dlopen = _dl_sym(RTLD_NEXT, "dlopen", dlopen);
  }

  void *ret = libdl_dlopen(filename, flag);
  if (ret != NULL) {
    pthread_mutex_lock(&lock);
    struct handle *h = malloc(sizeof(struct handle));
    // assert(h != NULL)
    h->handle = ret;
    LIST_INSERT_HEAD(&handles, h, l);
    pthread_mutex_unlock(&lock);
  }

  return ret;
}

int dlclose(void *handle) {
  pthread_mutex_lock(&lock);
  // initialize on first call
  static int (*libdl_dlclose)(void *) = NULL;
  if (libdl_dlclose == NULL) {
    libdl_dlclose = _dl_sym(RTLD_NEXT, "dlclose", dlclose);
  }

  struct handle *h = NULL;
  LIST_FOREACH(h, &handles, l) {
    if (h->handle == handle) {
      LIST_REMOVE(h, l);
      free(h);
    }
  }

  pthread_mutex_unlock(&lock);
  return libdl_dlclose(handle);
}

int main(int argc, char **argv) {
  struct arguments arguments = {0};
  char path[PATH_MAX + 1] = {0};

  // parse known options
  argp_parse(&argp, argc, argv, ARGP_IN_ORDER, 0, &arguments);

  // configure ocltrace
  setenv("OCLTRACE", "1", 1);
  if (arguments.verbose) setenv("OCLTRACE_VERBOSE", "1", 1);
  if (arguments.time) setenv("OCLTRACE_TIME", "1", 1);
  if (arguments.time_spent) setenv("OCLTRACE_TIME_SPENT", "1", 1);
  if (arguments.stats == 1) setenv("OCLTRACE_STATS", "1", 1);
  if (arguments.stats == 2) setenv("OCLTRACE_STATS", "2", 1);
  if (arguments.library) setenv("OCLTRACE_LIBRARY", arguments.library, 1);
  if (arguments.output_file)
    setenv("OCLTRACE_OUTPUT_FILE", arguments.output_file, 1);

  // configure linker
  readlink("/proc/self/exe", path, PATH_MAX);
  setenv("LD_PRELOAD", path, 1);

  // call executable
  execvp(arguments.argv[0], &arguments.argv[0]);
  fprintf(stderr, "Failed to execv %s\n", arguments.argv[0]);
  return 255;
}
