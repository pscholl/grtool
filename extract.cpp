#include "cmdline.h"
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

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
    char *tok = strtok(l, DELIM);

    // return on emtpy line and ignore comments
    if (tok[0]=='\n') return m;
    if (tok[0]=='#') continue;

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
    dim = 0; while (tok=strtok(NULL, DELIM)) {
      if (tok[0]=='#') break; // ignore comments

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
        fprintf(stderr, "ERR: too many values (max: %d) on line %d\n", m->dimv, m->diml);
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
mean(matrix_t *m, char *s, size_t max)
{
  double results[m->dimv];
  size_t n=0;

  for (size_t i=0; i<m->diml; i++)
    for (size_t j=0; j<m->dimv; j++)
      results[j] += m->vals[i*m->dimv + j];

  for (size_t j=0; j<m->dimv; j++)
    n += snprintf(s+n, max-n, "%g\t", results[j]/m->diml);

  return s;
}

char*
variance(matrix_t *m, char* s, size_t max)
{
  double results[m->dimv];
  double mean[m->dimv];
  size_t n=0;

  // calculate mean first
  for (size_t i=0; i<m->diml; i++)
    for (size_t j=0; j<m->dimv; j++)
      mean[j] += m->vals[i*m->dimv + j];

  for (size_t j=0; j<m->dimv; j++)
    mean[j] = mean[j]/m->diml;

  // calc variance now
  for (size_t i=0; i<m->diml; i++)
    for (size_t j=0; j<m->dimv; j++)
      results[j] += pow(m->vals[i*m->dimv + j] - mean[j], 2);

  // and convert to string
  for (size_t j=0; j<m->dimv; j++)
    n += snprintf(s+n, max-n, "%g\t", results[j]/m->diml);

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
    for (int j=0; j<m->dimv; j++) {
      double value = m->vals[i*m->dimv + j];
      maximum[j] = maximum[j]<value ? value : maximum[j];
      minimum[j] = minimum[j]>value ? value : minimum[j];
    }

  for (int j=0; j<m->dimv; j++)
    span[j] = maximum[j] - minimum[j];

  for (int j=0; j<m->dimv; j++)
    n += snprintf(s+n, max-n, "%g\t%g\t%g\t", maximum[j],minimum[j],span[j]);

  return s;
}


typedef char* (*process_call_t)(matrix_t*, char*, size_t);
struct extractor {
  const char *shorthand, *name, *desc;
  process_call_t call;
} extractors[] = {
  {"m", "mean", "compute mean/average of each axis", mean},
  {"r", "range", "compute range (min/max and their difference", range},
  {"v", "variance", "computer variance of each axis", variance}
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
  c.footer     ("<feature-extractor>");

  bool parse_ok = c.parse(argc, argv, false)  && !c.exist("help");

  /* load the extractor and check wether we need to list them */
  string str_extractor = "list",
            input_file = "-";

  if (c.rest().size() > 0) {
    str_extractor = c.rest()[0];
    c.rest().erase(c.rest().begin());
  }

  // either list all extractor and exit or build a functional 
  // to compute the features
  if (str_extractor == "list") {
    cout << c.usage() << endl;
    printf("Available Extractors:\n\n");
    for (size_t i=0; i<sizeof(extractors)/sizeof(extractors[0]); i++) {
      struct extractor e = extractors[i];
      printf(" %s (%s): %s\n", e.name, e.shorthand, e.desc);
    }
    return 0;
  } else {
    char *tmp = strdup(str_extractor.c_str());
    for (char *tok=strtok(tmp,DELIM);tok; tok=strtok(NULL, DELIM))
    {
      size_t i=0;
      for (i=0; i<sizeof(extractors)/sizeof(extractors[0]); i++) {
        struct extractor e = extractors[i];
        if (strcmp(tok,e.shorthand)==0 || strcmp(tok,e.name)==0) {
          processors[num_processors++] = e;
          break;
        }
      }

      if (i==sizeof(extractors)/sizeof(extractors[0])) {
        fprintf(stderr, "ERR: unknown extractor: '%s'\n", tok);
        exit(-1);
      }
    }
    free(tmp);
  }

  // parsing cmdline alright?
  if (!parse_ok) {
    cerr << c.usage() << endl << c.error() << endl;
    return -1;
  }

  // check if we've work
  if (num_processors == 0) {
    fprintf(stderr, "no extractors active, please specify at least one\n");
    exit(-1);
  }

  // if there is not input file, use stdin
  if (c.rest().size()==0)
    c.rest().push_back("-");

  matrix_t m = {0};
  char out[LINE_MAX], l[LINE_MAX];

  // print optional header
  if (!c.exist("no-header")) {
    size_t n=0;
    for(size_t i=0; i<num_processors; i++)
      n += snprintf(out+n,sizeof(out)-n,"%s\t", processors[i].name);
    printf("# %s\n",out);
  }

  while ( read_matrix(c.rest(),&m) )
  {
    size_t n=0;
    if (m.diml == 0)
      printf("\n");
    else {
      for(size_t i=0; i<num_processors; i++)
        n += snprintf(out+n,sizeof(out)-n,"%s", processors[i].call(&m,l,sizeof(l)));
      printf("%s\t%s\n", m.labels[m.diml-1], out);
    }
  }
}
