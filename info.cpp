#include "libgrt_util.h"
#include "cmdline.h"

int main(int argc, char *argv[]) 
{
  cmdline::parser c;
  c.add<string>("type",       't', "force classification, regression or timeseries input", false, "", cmdline::oneof<string>("classification", "regression", "timeseries", "auto"));
  c.add<int>   ("verbose",    'v', "verbosity level: 0-4", false, 0);
  c.add        ("help",       'h', "print this message");
  c.footer     ("[filename]...");

  /* parse the classifier-common arguments */
  if (!c.parse(argc,argv)) {
    cerr << c.usage() << endl << c.error() << endl;
    return -1;
  }

  if (c.exist("help")) {
    cout << c.usage();
    return 0;
  }

  /* handling of TERM and INT signal and set verbosity */
  set_verbosity(c.get<int>("verbose"));

  /* load the dataset and print its stats */
  istream &in = grt_fileinput(c);
  if (!in) return -1;

  string type = c.get<string>("type");
  CsvIOSample io(type);
  CollectDataset dataset;

  while (in >> io)
    csvio_dispatch(io, dataset.add, io.labelset);

  cout << dataset.getStatsAsString();
  return 0;
}
