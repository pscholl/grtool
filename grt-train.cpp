#include <GRT.h>
#include <iostream>
#include "cmdline.h"
#include "libgrt_util.h"

using namespace GRT;
using namespace std;

void addClassifierArguments(cmdline::parser &c, string classifier);
bool applyClassifierArguments(cmdline::parser &c, Classifier *o, string classifier);
InfoLog info;
ErrorLog err;

int main(int argc, const char *argv[])
{
  static bool is_running = true;
  cmdline::parser c;

  c.add<int>   ("verbose",    'v', "verbosity level: 0-4", false, 0);
  c.add        ("help",       'h', "print this message");
  c.add<string>("type",       't', "force classification, regression or timeseries input", false, "", cmdline::oneof<string>("classification", "regression", "timeseries", "auto"));
  c.add<string>("classifier", 'c', "classifier module", false);
  c.add<string>("model",      'm', "file to store trained classifier", true);
  c.footer     ("[filename]...");

  /* parse the classifier-common arguments */
  if (!c.parse(argc,argv,false)) {
    cerr << c.usage() << "\n" << c.error() << "\n" ;
    return -1;
  }

  /* handling of TERM and INT signal and set verbosity */
  set_running_indicator(&is_running);
  set_verbosity(c.get<int>("verbose"));

  /* got a classifier, or need to list them all? */
  string arg_classifier = c.get<string>("classifier");
  Classifier *classifier = Classifier::createInstanceFromString(arg_classifier);
  if (classifier == NULL) {
    cerr << c.usage() << endl;
    cerr << "classifier: " << arg_classifier << " not found" << endl;
    cout << "available classifiers:" << endl;
    vector<string> names = Classifier::getRegisteredClassifiers();
    for(vector<string>::iterator it = names.begin(); it != names.end(); ++it) {
      cout << *it << endl;
    }
    return -1;
  }

  /* add the classifier specific arguments */
  addClassifierArguments(c, arg_classifier);

  /* re-parse the classifier-specific arguments */
  if (!c.parse(argc,argv) || c.exist("help")) {
    cerr << c.usage() << "\n" << c.error() << "\n" ;
    return -1;
  }

  /* and apply them to the classifier instance */
  applyClassifierArguments(c, classifier, arg_classifier);

  string type = c.get<string>("type");
  CsvIOSample io(type);
  CollectDataset dataset;

  string filename = c.rest().size() > 0 ? c.rest()[0] : "-";
  ifstream inf(filename);
  if (filename!="-" && !inf) {
    cerr << "unable to open file: " << filename << endl;
    return -1;
  }
  istream &in = filename != "-" ? inf : cin;

  while (in >> io && is_running)
    csvio_dispatch(io, dataset.add, io.labelset);

  info << dataset.getStatsAsString() << endl;

  bool ok; csvio_dispatch(dataset, ok=classifier->train);
  if (!ok) {
    err << "training failed" << endl;
    return -1;
  }

  if (!classifier->save(c.get<string>("model"))) {
    err << "saving to " << c.get<string>("model") << " failed" << endl;
    return -1;
  } else
    return 0;
}

void addClassifierArguments(cmdline::parser &c, string classifier)
{
  if ( "HMM" == classifier ) {
    c.add<string>("hmmtype",      'k', "either 'discrete_ergodic', 'discrete_leftright', 'continuous_ergodic' or 'continuous_leftright'", false, "continuous_leftright", cmdline::oneof<string>("continuous_leftright", "continuous_ergodic", "discrete_leftright", "discrete_ergodic"));
    c.add<int>   ("comitteesize", 's', "number of models used for prediction, default: 10", false, 10);
    c.add<double>("delta",        'd', "learning delta, default: 1", false, 1);
  } else if ( "KNN" == classifier ) {
    c.add<string>("distance",         'd', "either 'euclidean', 'cosine' or 'manhatten'", false, "euclidean", cmdline::oneof<string>("euclidean", "cosine", "manhattan"));
    c.add<double>("null_coefficient", 'n', "delta for NULL-class rejection, 0.0 means off", false, 0.0);
    c.add<int>   ("k_neighbors",      'K', "number of neighbors used in classification", false, 5);
  }
}

#define checkedarg(func, type, name) if(!func(c.get<type>(name))) { cerr << "invalid value for" << name << " " << c.get<type>(name) << endl; return false; }

bool applyClassifierArguments(cmdline::parser &c, Classifier *o, string classifier)
{
  if ( "HMM" == classifier ) {
    HMM *h = (HMM*) o;

    string hmmtype = c.get<string>("hmmtype");
    if ( hmmtype.find("discrete") != string::npos )
      h->setHMMType( HMM_DISCRETE );
    else
      h->setHMMType( HMM_CONTINUOUS );
    if ( hmmtype.find("ergodic") != string::npos )
      h->setModelType( HMM_ERGODIC );
    else
      h->setModelType( HMM_LEFTRIGHT );

    checkedarg(h->setCommitteeSize, int, "comitteesize");
    checkedarg(h->setDelta, double, "delta");
  } else if ( "KNN" == classifier ) {
    KNN *k = (KNN*) o;

    if (c.get<double>("null_coefficient") != 0.0 ) {
      checkedarg(k->setNullRejectionCoeff, double, "null_coefficient");
      k->enableNullRejection(true);
    }

    checkedarg(k->setK, int, "k_neighbors");

    string distance = c.get<string>("distance");
    if ( "euclidean" == distance )
      k->setDistanceMethod(KNN::EUCLIDEAN_DISTANCE);
    else if ( "cosine" == distance )
      k->setDistanceMethod(KNN::COSINE_DISTANCE);
    else if ( "manhattan" == distance )
      k->setDistanceMethod(KNN::MANHATTAN_DISTANCE);
  }

  return true;
}
