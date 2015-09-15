#include "cmdline.h"
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>

using namespace std;

typedef struct matrix {
  size_t dimv, diml, nlabels, allocd;
  double *vals;
  char   **labels;
  char   **labelset;
} matrix_t;

matrix_t*
read_matrix(vector<string> filenames, matrix_t *m)
{
  #define DELIM " \t"
  static size_t i   = 0;
  static FILE *file = NULL;

  if (file == NULL) {
    static string filename = filenames[i];
    file = filename=="-" ? stdin : fopen(filename.c_str(), "r");

    if (file == NULL) {
      fprintf(stderr, "unable to open file: %s\n%s\n", filename.c_str(), strerror(errno));
      exit(-1);
    }
  }

  char l[LINE_MAX],c;
  size_t dim=0;

  // reset the read line counter
  m->diml = 0;

  while ( fgets(l, sizeof(l), file) ) {
    char *saveptr=l, *tok=strsep(&saveptr,DELIM);

    for(; tok && !strlen(tok); tok=strsep(&saveptr,DELIM))
      ; // remove all delims at the start

    // return on emtpy line and ignore comments
    if (tok[0]=='\n') return m;
    if (tok[0]=='#')  continue;

    // resize storage space if required
    if (m->allocd <= m->diml) {
      m->allocd = m->allocd==0 ? 1 : m->allocd*2;
      m->labels = (char**) realloc(m->labels, m->allocd * sizeof(m->labels[0]));
      m->vals   = (double*) realloc(m->vals, m->allocd * m->dimv * sizeof(m->vals[0]));
    }

    // first field is always a label, check if we've
    // already seen this one and store accordingly
    for (i=0; i<m->nlabels; i++)
      if (strcmp(m->labelset[i],tok)==0) {
        m->labels[m->diml] = m->labelset[i];
        break;
      }

    // labelset does not contain label yet
    if (i==m->nlabels) {
      m->labelset = (char**) realloc(m->labelset, (m->nlabels+1) * sizeof(m->labelset[0]));
      m->labels[m->diml] = strdup(tok);
      m->labelset[m->nlabels++] = m->labels[m->diml];
    }

    // now we read all the floats into the data array
    dim = 0; while(tok=strsep(&saveptr, DELIM"\n")) {
      if (tok[0]=='#') break; // ignore comments
      if (!strlen(tok)) continue; // multiple DELIMS

      // parse value
      errno = 0; double v = strtod(tok,NULL);

      // check for parse error
      if (errno != 0) {
        fprintf(stderr, "ERR: unable to convert to float on line %d: %s\n", m->diml, strerror(errno));
        exit(-1);
      }

      // on the first line, resize storage, else error and exit
      if (dim==m->dimv && m->diml==0) {
        m->vals = (double*) realloc(m->vals, m->allocd * ++m->dimv * sizeof(m->vals[0]));
      } else if (dim==m->dimv) {
        fprintf(stderr, "ERR: got more than %d values on line %d\n", m->dimv, m->diml);
        exit(-1);
      }

      m->vals[m->diml*m->dimv + dim++] = v;
    }

    // ready to read the next line
    m->diml++;

    // additional check if there is enough data on this line!
    if (dim!=m->dimv) {
      fprintf(stderr, "ERR: not enough fields (need %d got %d) on line %d\n", m->dimv, dim, m->diml);
      exit(-1);
    }
  }

  return m->diml==0 ? NULL : m;
}

char*
zcr(matrix_t *m, char *s, size_t max)
{
  double results[m->dimv] = {0};
    bool signs[m->dimv];
  size_t n = 0;

  for (size_t j=0; j<m->dimv; j++)
    signs[j] = signbit(m->vals[j]);

  for (size_t i=0; i<m->diml; i++)
    for (size_t j=0; j<m->dimv; j++) {
      bool sign = signbit(m->vals[i*m->dimv + j]);
      results[j] += signs[j] != sign;
      signs[j] = sign;
    }

  for (size_t j=0; j<m->dimv; j++)
    n += snprintf(s+n, max-n, "%g\t", results[j]);

  return s;
}

double*
_mean(matrix_t *m, double *results)
{
  for (size_t i=0; i<m->diml; i++)
    for (size_t j=0; j<m->dimv; j++)
      results[j] += m->vals[i*m->dimv + j];

  for (size_t j=0; j<m->dimv; j++)
    results[j] /= m->diml;

  return results;
}

char*
mean(matrix_t *m, char *s, size_t max)
{
  double results[m->dimv] = {0}; _mean(m, results);
  size_t n=0;

  for (size_t j=0; j<m->dimv; j++)
    n += snprintf(s+n, max-n, "%g\t", results[j]);

  return s;
}

double*
_variance(matrix_t *m, double *results)
{
  double mean[m->dimv]={0}; _mean(m, mean);

  for (size_t j=0; j<m->dimv; j++)
    results[j] = 0;

  // calc variance now
  for (size_t i=0; i<m->diml; i++)
    for (size_t j=0; j<m->dimv; j++)
      results[j] += pow(m->vals[i*m->dimv + j] - mean[j], 2);

  for (size_t j=0; j<m->dimv; j++)
    results[j] /= m->diml;

  return results;
}

char*
variance(matrix_t *m, char* s, size_t max)
{
  double var[m->dimv]={0}; _variance(m, var);
  size_t n=0;

  // and convert to string
  for (size_t j=0; j<m->dimv; j++)
    n += snprintf(s+n, max-n, "%g\t", var[j]);

  return s;
}

char*
range(matrix_t *m, char* s, size_t max)
{
  size_t n=0;
  double maximum[m->dimv] = {-INFINITY},
         minimum[m->dimv] = { INFINITY},
         span[m->dimv];

  for (size_t i=0; i<m->diml; i++)
    for (size_t j=0; j<m->dimv; j++) {
      double value = m->vals[i*m->dimv + j];
      maximum[j] = maximum[j]<value ? value : maximum[j];
      minimum[j] = minimum[j]>value ? value : minimum[j];
    }

  for (size_t j=0; j<m->dimv; j++)
    span[j] = maximum[j] - minimum[j];

  for (size_t j=0; j<m->dimv; j++)
    n += snprintf(s+n, max-n, "%g\t%g\t%g\t", maximum[j],minimum[j],span[j]);

  return s;
}

/*
 *  This Quickselect routine is based on the algorithm described in
 *  "Numerical recipes in C", Second Edition,
 *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
 *  This code by Nicolas Devillard - 1998. Public domain.
 */
#define ELEM_SWAP(a,b) { register double t=(a);(a)=(b);(b)=t; }
double quickselect(double arr[], size_t n, size_t step) 
{
    int low, high ;
    int median;
    int middle, ll, hh;

    low = 0 ; high = n-1 ; median = (low + high) / 2;
    for (;;) {
        if (high <= low) /* One element only */
            return arr[median] ;

        if (high == low + 1) {  /* Two elements only */
            if (arr[low] > arr[high])
                ELEM_SWAP(arr[low], arr[high]) ;
            return arr[median] ;
        }

    /* Find median of low, middle and high items; swap into position low */
    middle = (low + high) / 2;
    if (arr[middle] > arr[high])    ELEM_SWAP(arr[middle], arr[high]) ;
    if (arr[low] > arr[high])       ELEM_SWAP(arr[low], arr[high]) ;
    if (arr[middle] > arr[low])     ELEM_SWAP(arr[middle], arr[low]) ;

    /* Swap low item (now in position middle) into position (low+1) */
    ELEM_SWAP(arr[middle], arr[low+1]) ;

    /* Nibble from each end towards middle, swapping items when stuck */
    ll = low + 1;
    hh = high;
    for (;;) {
        do ll++; while (arr[low] > arr[ll]) ;
        do hh--; while (arr[hh]  > arr[low]) ;

        if (hh < ll)
        break;

        ELEM_SWAP(arr[ll], arr[hh]) ;
    }

    /* Swap middle item (in position low) back into correct position */
    ELEM_SWAP(arr[low], arr[hh]) ;

    /* Re-set active partition */
    if (hh <= median)
        low = ll;
        if (hh >= median)
        high = hh - 1;
    }
}
#undef ELEM_SWAP

char*
median(matrix_t *m, char* s, size_t max)
{
  double results[m->dimv];
  size_t n=0;

  for (size_t j=0; j<m->dimv; j++)
    results[j] = quickselect(m->vals+j, m->diml, m->dimv);

  for (size_t j=0; j<m->dimv; j++)
    n += snprintf(s+n, max-n, "%g\t", results[j]);

  return s;
}

char*
rms(matrix_t *m, char* s, size_t max)
{
  double results[m->dimv+1];
  size_t n=0;

  for (size_t i=0; i<m->diml; i++) {
    for (size_t j=0; j<m->dimv; j++)
      results[j] += pow(m->vals[i*m->dimv + j],2);

    for (size_t j=0; j<m->dimv; j++)
      results[m->dimv] += results[j];
  }

  for (size_t j=0; j<m->dimv+1; j++)
    results[j] = sqrt(results[j]);

  for (size_t j=0; j<m->dimv+1; j++)
    n += snprintf(s+n, max-n, "%g\t", results[j]);

  return s;
}

char*
timedomain(matrix_t *m, char* s, size_t max)
{
# define calc(f) f(m, s+strlen(s), max-strlen(s))

  mean(m, s, max);
  calc(variance);
  calc(range);
  calc(median);
  calc(zcr);
  calc(rms);

  return s;
}

matrix_t*
z_normalize(matrix_t *m)
{
  double mean[m->dimv]; _mean(m,mean);
  double std[m->dimv];  _variance(m,std);

  for (size_t i=0; i<m->dimv; i++)
    std[i] = sqrt(std[i]);

  for (size_t i=0; i<m->diml; i++)
    for (size_t j=0; j<m->dimv; j++)
      m->vals[i*m->dimv + j] = std[j]==0 ? 0. : (m->vals[i*m->dimv + j] - mean[j]) / std[j] ;

  return m;
}

matrix_t*
o_normalize(matrix_t *m)
{
  double offset[m->dimv];

  for (size_t j=0; j<m->dimv; j++)
    offset[j] = m->vals[j];

  for (size_t i=0; i<m->diml; i++)
    for (size_t j=0; j<m->dimv; j++)
      m->vals[i*m->dimv + j] -= offset[j];

  return m;
}

typedef char* (*process_call_t)(matrix_t*, char*, size_t);
struct extractor {
  const char *shorthand, *name, *desc;
  process_call_t call;
} extractors[] = {
  {"m", "mean", "compute mean/average of each axis", mean},
  {"r", "range", "compute range (min/max and their difference", range},
  {"v", "variance", "compute variance of each axis", variance},
  {"e", "median", "compute median of each axis", median},
  {"z", "zcr", "zero-crossing rate", zcr},
  {"s", "rms", "root-mean squared over each and all axis", rms},
  {"t", "time", "shorthand for all time-domain features: mean,variance,range,median", timedomain}
};
// a list of active extractors
size_t num_processors=0;
struct extractor processors[sizeof(extractors)/sizeof(extractors[0]) * sizeof(process_call_t)];

int main(int argc, const char *argv[]) {
  cmdline::parser c;
  int buffer_size=0;

  c.add<int>   ("verbose",    'v', "verbosity level: 0-4", false, 0);
  c.add        ("help",       'h', "print this message");
  c.add        ("no-header",  'q', "do not print the header");
  c.add        ("z-normalize", 'z', "z-normalize ( (x-mean(x))/std(x) ) all samples");
  c.add        ("o-normalize", 'o', "o-normalize, compute x_i - x_0, i.e. remove the first component from each sample");
  c.footer     ("<feature-extractor>");

  bool parse_ok = c.parse(argc, argv, false)  && !c.exist("help");

  if (c.rest().size()==0)
    c.rest().push_back("list");

  // either list all extractor and exit or build a functional
  // to compute the features
  for (uint32_t j=0; j<c.rest().size(); j++) {
    string str_extractor = c.rest()[j];

    if (str_extractor == "list") {
      cout << c.usage() << endl;
      printf("Available Extractors:\n\n");
      for (size_t i=0; i<sizeof(extractors)/sizeof(extractors[0]); i++) {
        struct extractor e = extractors[i];
        printf(" %s (%s): %s\n", e.name, e.shorthand, e.desc);
      }
      return 0;
    } else {
      uint32_t i;

      for (i=0; i<sizeof(extractors)/sizeof(extractors[0]); i++) {
        struct extractor e = extractors[i];
        if (strcmp(str_extractor.c_str(),e.shorthand)==0 ||
            strcmp(str_extractor.c_str(),e.name)==0) {
          processors[num_processors++] = e;
          break;
        }
      }

      if (i==sizeof(extractors)/sizeof(extractors[0])) {
        fprintf(stderr, "ERR: unknown extractor: '%s'\n", str_extractor.c_str());
        exit(-1);
      }
    }
  }

  // parsing cmdline alright?
  if (!parse_ok) {
    cerr << c.usage() << endl << c.error() << endl;
    return -1;
  }

  // check if we've got work
  if (num_processors == 0) {
    fprintf(stderr, "no extractors active, please specify at least one\n");
    exit(-1);
  }

  // some sanity checks for options
  if (c.exist("z-normalize") && c.exist("o-normalize")) {
    fprintf(stderr, "z- and o- normalization can not be done simultaneously\n");
    exit(-1);
  }

  matrix_t m = {0};
  char out[LINE_MAX], l[LINE_MAX];

  // print optional header
  if (!c.exist("no-header")) {
    size_t n=0;
    for(size_t i=0; i<num_processors; i++)
      n += snprintf(out+n,sizeof(out)-n,"%s\t", processors[i].name);
    printf("# %s\n",out);
  }

  while ( read_matrix({"-"},&m) )
  {
    size_t n=0;
    if (m.diml == 0)
      printf("\n");
    else {
      if (c.exist("z-normalize"))
        z_normalize(&m);

      else if (c.exist("o-normalize"))
        o_normalize(&m);

      for(size_t i=0; i<num_processors; i++)
        n += snprintf(out+n,sizeof(out)-n,"%s", processors[i].call(&m,l,sizeof(l)));

      printf("%s\t%s\n", m.labels[m.diml-1], out);
    }
  }
}
