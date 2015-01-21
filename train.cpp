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
  c.add<float> ("num-samples",'n', "limit the training dataset to the first n samples, if n is less than or equal 1 it is interpreted as percentage of input data", false, 1.);
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
      Classifier *c = Classifier::createInstanceFromString(*it);
      cout << *it << (c->getTimeseriesCompatible() ? " (timeseries)" : "") << endl;
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

  /* now start to read input samples */
  string type = c.get<string>("type");
  CsvIOSample io(type);
  CollectDataset dataset;

  /* do we read from a file or from stdin-? */
  string filename = c.rest().size() > 0 ? c.rest()[0] : "-";
  ifstream inf(filename);
  if (filename!="-" && !inf) {
    cerr << "unable to open file: " << filename << endl;
    return -1;
  }
  istream &in = filename != "-" ? inf : cin;

  /* check if the number of input is limited */
  float input_limit   = c.get<float>("num-samples");
    int input_limit_i = input_limit <= 1 ? 0 : input_limit,
        num_samples   = 0;

  while (in >> io && is_running && (input_limit_i==0 || num_samples < input_limit_i) ) {
    bool ok = false; csvio_dispatch(io, ok=dataset.add, io.labelset);
    if (!ok) {
      cerr << "error at line " << io.linenum << endl;
      exit(-1);
    }
    num_samples++;
  }

  info << dataset.getStatsAsString() << endl;

  /* if we have percent input limit, we need to apply this now */
  TimeSeriesClassificationData t_testdata;
  ClassificationData c_testdata;

  if (input_limit < 1.) {
    switch(io.type) {
    case TIMESERIES:
      t_testdata = dataset.t_data.partition( input_limit * 100, true );
      break;
    case CLASSIFICATION:
      c_testdata = dataset.c_data.partition( input_limit * 100, true );
      break;
    default:
      err << "unknown data type" << endl;
      return -1;
    }
  }

  /* train and save classifier */
  bool ok; csvio_dispatch(dataset, ok=classifier->train);
  if (!ok) {
    err << "training failed" << endl;
    return -1;
  }

  if (!classifier->save(c.get<string>("model"))) {
    err << "saving to " << c.get<string>("model") << " failed" << endl;
    return -1;
  }

  /* if there is testdataset we need to print this now */
  if (input_limit < 1.) {
    switch(io.type) {
    case TIMESERIES:
      for (auto sample : t_testdata.getClassificationData()) {
        string label = t_testdata.getClassNameForCorrespondingClassLabel( sample.getClassLabel() );
        MatrixDouble &matrix = sample.getData();
        for (int i=0; i<matrix.getNumRows(); i++) {
          cout << label;
          for (int j=0; j<matrix.getNumCols(); j++)
            cout << "\t" << matrix[i][j];
          cout << endl;
        }
        cout << endl;
      }
      break;
    case CLASSIFICATION:
      for (auto sample : c_testdata.getClassificationData()) {
        string label = c_testdata.getClassNameForCorrespondingClassLabel( sample.getClassLabel() );
        cout << label;
        for (auto val : sample.getSample())
          cout << "\t" << val;
        cout << endl;
      }
      break;
    default:
      err << "unknown data type" << endl;
      return -1;
    }
  } else if (input_limit > 1.) {
    string line;
    while (getline(in, line))
      cout << line << endl;
  } else
    return 0;
}

void addClassifierArguments(cmdline::parser &c, string classifier)
{
  if ( "HMM" == classifier ) {
    c.add<string>("hmmtype",      'k', "either 'discrete_ergodic', 'discrete_leftright', 'continuous_ergodic' or 'continuous_leftright'", false, "continuous_leftright", cmdline::oneof<string>("continuous_leftright", "continuous_ergodic", "discrete_leftright", "discrete_ergodic"));
    c.add<int>   ("comitteesize", 's', "number of models used for prediction, default: 10", false, 10);
    c.add<double>("delta",        'd', "learning delta, default: 1", false, 1);
    c.add<int>   ("downsample",   'D', "downsample factor, default: 5", false, 5);
  } else if ( "KNN" == classifier ) {
    c.add<string>("distance",         'd', "either 'euclidean', 'cosine' or 'manhatten'", false, "euclidean", cmdline::oneof<string>("euclidean", "cosine", "manhattan"));
    c.add<double>("null_coefficient", 'N', "delta for NULL-class rejection, 0.0 means off", false, 0.0);
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
    checkedarg(h->setDownsampleFactor, int, "downsample");
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
