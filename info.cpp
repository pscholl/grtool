#include "libgrt_util.h"
#include "cmdline.h"

int main(int argc, char *argv[]) 
{
  cmdline::parser c;
  c.add<int>   ("verbose",    'v', "verbosity level: 0-4", false, 0);
  c.add        ("help",       'h', "print this message");
  c.footer     ("[filename]...");

  /* parse the classifier-common arguments */
  if (!c.parse(argc,argv)) {
    cerr << c.usage() << endl << c.error() << endl;
    return -1;
  }

  /* handling of TERM and INT signal and set verbosity */
  set_verbosity(c.get<int>("verbose"));

  /* load the dataset and print its stats */
  istream &in = grt_fileinput(c);
  if (!in) return -1;

  CsvIOSample io("");
  CollectDataset dataset;

  while (in >> io)
    csvio_dispatch(io, dataset.add, io.labelset);

  cout << dataset.getStatsAsString() << endl;
  return 0;
}
