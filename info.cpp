#include "libgrt_util.h"
#include "cmdline.h"

int main(int argc, char *argv[]) 
{
  cmdline::parser c;

  c.add<int>   ("verbose",    'v', "verbosity level: 0-4", false, 0);
  c.add        ("help",       'h', "print this message");
  c.add<string>("model",      'm', "file to store trained classifier", true);
  c.footer     ("[filename]...");

  /* parse the classifier-common arguments */
  if (!c.parse(argc,argv,false)) {
    cerr << c.usage() << "\n" << c.error() << "\n" ;
    return -1;
  }

  /* handling of TERM and INT signal and set verbosity */
  set_verbosity(c.get<int>("verbose"));

  /* load a classification model */
  string model_file = c.get<string>("model");
  Classifier *classifier = loadFromFile(model_file);

  if (classifier == NULL) {
    cerr << "unable to load classification model " << model_file << " giving up" << endl;
    return -1;
  }

  cout << classifier->getClassifierType() << endl;

  return 0;
}
