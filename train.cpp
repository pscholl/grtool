#include <GRT.h>
#include <iostream>
#include <stdio.h>
#include "cmdline.h"
#include "libgrt_util.h"

using namespace GRT;
using namespace std;

Classifier *apply_cmdline_args(string,cmdline::parser&,int,string&);
string list_classifiers();
InfoLog info;

int main(int argc, const char *argv[])
{
  Classifier *classifier = NULL;
  string input_file = "-";
  cmdline::parser c;

  c.add<int>   ("verbose", 'v', "verbosity level: 0-4", false, 1);
  c.add        ("help",    'h', "print this message");
  c.add<string>("output",  'o', "store trained classifier in file", false);
  c.add<string>("trainset",'n', "limit the training dataset to the first n samples, if n is less than or equal 1 it is interpreted the percentage of a stratified random split that is retained for training. If not a number it is interpreted as a filename containing training samples.", false, "1");
  c.footer     ("<classifier> [input-data]...");

  /* parse common arguments */
  bool parse_ok = c.parse(argc, argv, false)  && !c.exist("help");
  set_verbosity(c.get<int>("verbose"));

  /* got a trainable classifier? */
  string str_classifier = c.rest().size() > 0 ? c.rest()[0] : "list";
  if (str_classifier == "list") {
    cout << c.usage() << endl;
    cout << list_classifiers();
    return 0;
  }

  /* add the classifier specific arguments */
  classifier = apply_cmdline_args(str_classifier,c,1,input_file);

  if (!parse_ok) {
    cerr << c.usage() << endl << c.error() << endl;
    return -1;
  }

  if (classifier == NULL) {
    cerr << "error: unable to load/create algorithm: " << str_classifier << endl;
    return -1;
  }

  /* check if we can open the output file */
  ofstream test(c.get<string>("output"), ios_base::out);
  ostream &output = c.exist("output") ? test : cout;

  if (c.exist("output") && !test.good()) {
    cerr << c.usage() << endl << "unable to open \"" << c.get<string>("output") << "\" as output" << endl;
    return -1;
  }

  /* do we read from a file or stdin? */
  ifstream fin; fin.open(input_file);
  istream &in = input_file=="-" ? cin : fin;

  if (!in.good()) {
    cerr << "unable to open input file " << input_file << endl;
    return -1;
  }

  /* now start to read input samples */
  CsvIOSample io( classifier->getTimeseriesCompatible() ? "timeseries" : "classification" );
  CollectDataset dataset;

  /* check if the number of input is limited */
  string il  = c.get<string>("trainset");
  double ild = strtod(il.c_str(), NULL);
  int    ili = (int) ild,
         num_samples = 0;

  /* if we have percent input limit, we need to apply this now */
  TimeSeriesClassificationData t_testdata;
  ClassificationData c_testdata;

  if (ild == 0) { // got an input filename
    ifstream tin; tin.open(il);

    while (tin >> io) {
      bool ok=false; csvio_dispatch(io, ok=dataset.add, io.labelset);

      if (!ok) {
        cerr << "error at line " << io.linenum << endl;
        exit(-1);
      }
    }
  } else {

    while ((ili == 0 || num_samples < ili) && in >> io) {
      bool ok = false; csvio_dispatch(io, ok=dataset.add, io.labelset);
      if (!ok) {
        cerr << "error at line " << io.linenum << endl;
        exit(-1);
      }
      num_samples++;
    }

    if (num_samples==0)
      return 0;

    if (ild < 1.) {
      switch(io.type) {
      case TIMESERIES:
        t_testdata = dataset.t_data.partition( ild * 100, true );
        break;
      case CLASSIFICATION:
        c_testdata = dataset.c_data.partition( ild * 100, true );
        break;
      default:
        cerr << "unknown data type" << endl;
        return -1;
      }
    }
  }

  info << dataset.getStatsAsString() << endl;

  /* train and save classifier */
  bool ok; csvio_dispatch(dataset, ok=classifier->train);
  if (!ok) {
    cerr << "training failed" << endl;
    return -1;
  }

  /* propagate the classlabel names also */
  for (size_t i=!io.has_NULL_label; i<io.labelset.size(); i++) {
    classifier->setClassNameForLabel(i, io.labelset[i]);
  }

  if (!classifier->saveModelToFile(output)){
    cerr << "saving to " << c.get<string>("output") << " failed" << endl;
    return -1;
  }

  if (!c.exist("output"))
    cout << endl; // mark the end of the classifier if piping
  else
    test.close();

  /* if there is testdataset we need to print this now */
  bool first = true;
  if (ild > 0 && ild < 1.) {
    switch(io.type) {
    case TIMESERIES:
      for (auto sample : t_testdata.getClassificationData()) {
        string label = t_testdata.getClassNameForCorrespondingClassLabel( sample.getClassLabel() );
        MatrixDouble &matrix = sample.getData();
        for (int i=0; i<matrix.getNumRows(); i++) {
          if (first) {cout << label; first = false;}
          else {cout << endl << label;}
          for (int j=0; j<matrix.getNumCols(); j++)
            cout << "\t" << matrix[i][j];
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
      cerr << "unknown data type" << endl;
      return -1;
    }
  } else {
    string line;
    while (getline(in, line))
      cout << line << endl;
  }
}

string list_classifiers() {
  vector<string> exclude = {"HMM", "BAG", "SwipeDetector"};
  vector<string> names = Classifier::getRegisteredClassifiers();
  stringstream ss;
  string name;

  cout << "HMM (timeseries)" << endl;
  cout << "cHMM (timeseries)" << endl;

  for (auto name : names)  {
    if (find(exclude.begin(),exclude.end(),name)!=exclude.end())
      continue;
    Classifier *c = Classifier::createInstanceFromString(name);
    if (c->getTimeseriesCompatible())
      ss << name << " (timeseries" << (c->getSupportsNullRejection() ? ",null rejection)" : ")") << endl;
  }

  for (auto name : names)  {
    Classifier *c = Classifier::createInstanceFromString(name);
    if (find(exclude.begin(),exclude.end(),name)!=exclude.end())
      continue;
    if (!c->getTimeseriesCompatible())
      ss << name << (c->getSupportsNullRejection() ? " (null rejection)" : "") << endl;
  }

  return ss.str();
}

#define checkedarg(func, type, name) if(!func(p.get<type>(name))) { cerr << "invalid value for" << name << " " << p.get<type>(name) << endl; return NULL; }

Classifier *apply_cmdline_args(string name,cmdline::parser& c,int num_dimensions,string &input_file)
{
  cmdline::parser p;
  Classifier *o = NULL;

  if ( "HMM" == name ) {
#   define HMM_TYPE "ergodic", "leftright"
    p.add<string>("hmmtype",      'T', "either 'ergodic' or 'leftright' (default: ergodic)",  false, "leftright", cmdline::oneof<string>(HMM_TYPE));

    p.add<double>("delta",          0, "delta for leftright model, default: 1", false, 1);
    p.add<int>   ("num-states",   'S', "number of states", false, 10);
    p.add<int>   ("num-symbols",  'N', "number of symbols", false, 20);
    p.add<int>   ("max-epochs",     0, "maximum number of epochs during training", false, 1000);
    p.add<float> ("min-change",     0, "minimum change before abortion", false, 1.0e-5);
  } else if ( "cHMM" == name ) {
    p.add<string>("hmmtype",      'T', "either 'ergodic' or 'leftright' (default: ergodic)",  false, "ergodic", cmdline::oneof<string>(HMM_TYPE));

    p.add<int>   ("comitteesize",  0, "number of models used for prediction, default: 10", false, 10);
    p.add<double>("delta",         0, "delta for leftright model, default: 1", false, 1);
    p.add<int>   ("downsample",    0, "downsample factor, default: 5", false, 5);
  } else if ( "KNN" == name ) {
#   define KNN_DISTANCE "euclidean", "cosine", "manhattan"
    p.add<string>("distance",         'D', "either 'euclidean', 'cosine' or 'manhatten'", false, "euclidean", cmdline::oneof<string>(KNN_DISTANCE));
    p.add<double>("null-coefficient", 'N', "delta for NULL-class rejection, 0.0 means off", false, 0.0);
    p.add<int>   ("K-neighbors",      'K', "number of neighbors used in classification (if 0 search for optimum)", false, 0);
    p.add<int>   ("min-K",             0, "only used during search", false, 2);
    p.add<int>   ("max-K",             0, "only used during search", false, 20);
  } else if ( "DTW" == name ) {
#   define DTW_REJECTION_MODE "template", "class", "template_class"
    p.add<double>("null-coefficient", 'N', "multiplier for NULL-class rejection, 0.0 means off", false, 0.0);
    p.add<double>("null-threshold",   'T', "likelihood threshold for CLASS rejection modes, 0.0 means off", false, 0.0);
    p.add<string>("rejection-mode",   'R', "NULL-class rejection mode", false, "template", cmdline::oneof<string>(DTW_REJECTION_MODE));
    p.add<double>("warping-radius",   'W', "limit the warping to this radius (0 means disabled, 1 is maximum)", false, 0, cmdline::range(0.,1.));
  } else if ( "FiniteStateMachine" == name ) {
    p.add<int>   ("num-particles",        'N', "number of particles", false, 200);
    p.add<int>   ("num-clusters",         'M', "number of clusters per state", false, 10);
    p.add<double>("transition-smoothing", 'T', "state transition smoothing", false, 0);
    p.add<double>("measurement-noise",    'S', "measurement noise", false, 10.);
  } else if ( "ParticleClassifier" == name ) {
    p.add<int>   ("num-particles",        'N', "number of particles", false, 200);
    p.add<double>("measurement-noise",    'S', "measurement noise", false, 10.);
    p.add<double>("transition-sigma",     'T', "transition sigma", false, 0.005);
    p.add<double>("phase-sigma",          'P', "phase sigma", false, 0.1);
    p.add<double>("velocity-sigma",       'V', "velocity sigma", false, 0.01);
  } else if ( "RandomForests" == name) {
#   define RF_TRAINING "random", "iterative"
    p.add<int>   ("forest-size",          'N', "number of trees in the forest", false, 10);
    p.add<int>   ("num-split",            'S', "number of split to search", false, 100);
    p.add<int>   ("num-samples",          'M', "number of samples for non-leaf nodes", false, 5);
    p.add<int>   ("max-depth",            'D', "maximum depth of the tree", false, 10);
    p.add<string>("training-mode",        'T', "training mode", false, "random", cmdline::oneof<string>(RF_TRAINING));
    p.add        ("remove-features",      'F', "remove features at each split");
  } else if ( "SVM" == name ) {
#   define SVM_KERNELS "linear","poly","rbf","sigmoid","precomputed"
#   define SVM_TYPES   "C_SVC","NU_SVC","ONE_CLASS","EPSILON_SVR","NU_SVR"
    p.add<string>("kernel",               'K', "kernel type", false, "linear", cmdline::oneof<string>(SVM_KERNELS));
    p.add<string>("type",                 'T', "svm type", false,"C_SVC", cmdline::oneof<string>(SVM_TYPES));
    p.add<double>("gamma",                'G', "set to 0. to auto-calculate", false, 0, cmdline::range(0,1));
    p.add<int>   ("degree",               'D', "SVM degree parameter", false, 3);
    p.add<double>("coef0",                'O', "SVM coef0 parameter", false, 0);
    p.add<double>("nu",                   'M', "SVM nu parameter", false, 0.5);
    p.add<double>("C",                    'C', "SVM C parameter", false, 1);
  } else if ( "ANBC" == name ) {
    p.add<double>("null-coef",            'N', "null rejection coefficient, default: 0 (not used)", false, 0);
  } else if ( "GMM" == name ) {
    p.add<int>   ("mixtures",             'M', "num of mixtures", false, 3);
    p.add<double>("null-coef",            'N', "null rejection coefficient, default: 0 (not used)", false, 0);
    p.add<int>   ("max-iterations",       'I', "num of iterations", false, 10000);
    p.add<double>("epsilon",              'E', "minimum change between iteration", false, .1);
  } else if ( "AdaBoost" == name ) {
#   define ADABOOST_TYPES "max_positive", "max"
#   define ADABOOST_CLASS "DS","RBF"
    p.add<double>("null-coef",            'N', "null rejection coefficient, default: 0 (not used)", false, 0);
    p.add<int>   ("max-iterations",       'I', "num of iterations", false, 10000);
    p.add<string>("prediction-type",     'T', "predicition method" , false,"MAX_POSITIVE_VALUE", cmdline::oneof<string>(ADABOOST_TYPES));
    p.add<string>("weak-classifier",      'C', "weak classifier to be boosted", false, "DS", cmdline::oneof<string>(ADABOOST_CLASS));

    p.add<int>   ("num-steps",            'S', "(RBF/DS) number of steps for rbf", false, 100);

    p.add<double>("pos-tresh",            'P', "(RBF) positive classification treshhold", false, .9);
    p.add<double>("min-alpha",            'L', "(RBF) lower alpha threshold", false, .001);
    p.add<double>("max-alpha",            'H', "(RBF) higher alpha threshold", false, 1);
  } else if ( "DecisionTree" == name ) {
#   define DT_TRAINING "iterative", "random"
    p.add<int>   ("min-samples-per-node", 'M', "minimum number of samples per node before becoming a lead node", false, 5);
    p.add<int>   ("max-depth",            'D', "maximum depth of the tree", false, 10);
    p.add        ("remove-features",      'F', "remove features at each split");
    p.add<string>("training-mode",        'T', "training mode", false, "iterative", cmdline::oneof<string>(DT_TRAINING));
    p.add<int>   ("num-split",            'S', "number of splitting nodes to search", false, 100);
  } else if ( "MinDist" == name ) {
    p.add<double>("null-coef",            'N', "null rejection coefficient", false, 10);
    p.add<int>("num-clusters",            'C', "number of clusters", false, 10);
  } else if ( "Softmax" == name ) {
    p.add<double>("learning-rate",        'R', "learning rate for training", false, .1);
    p.add<double>("min-change",           'C', "minimum change between steps", false, 1e-10);
    p.add<double>("max-epochs",           'E', "maximum number of epochs", false, 1000);
  }

  if (c.exist("help")) {
    cerr << c.usage() << endl << name << " options:" << endl << p.str_options();
    exit(0);
  }

  if (!p.parse(c.rest())) {
    cerr << c.usage() << endl << name << " options:" << endl << p.str_options() << endl << p.error() << endl;
    exit(-1);
  }

  if ( "HMM" == name ) {
    vector<string> list = {HMM_TYPE};

    HMM *h = new HMM(
      /* hmmtype */ HMM_DISCRETE,
      /* hmmodel */ find(list.begin(), list.end(), p.get<string>("hmmtype")) - list.begin(),
      /* delta */   p.get<double>("delta"),
      /* scaling */ false,
      /* useNull */ true);

    checkedarg(h->setNumStates, int, "num-states");
    checkedarg(h->setNumSymbols, int, "num-symbols");
    checkedarg(h->setMaxNumEpochs, int, "max-epochs");
    checkedarg(h->setMinChange, float, "min-change");

    o = h;
  } else if ( "cHMM" == name ) {
    vector<string> list = {HMM_TYPE};

    HMM *h = new HMM(
      /* hmmtype */ HMM_CONTINUOUS,
      /* hmmodel */ find(list.begin(), list.end(), p.get<string>("hmmtype")) - list.begin(),
      /* delta */   p.get<double>("delta"),
      /* scaling */ false,
      /* useNull */ false);

    checkedarg(h->setCommitteeSize, int, "comitteesize");
    checkedarg(h->setDownsampleFactor, int, "downsample");

    o = h;
  } else if ( "KNN" == name ) {
    KNN *k = new KNN(
      /* K */           p.get<int>("K-neighbors"),
      /* useScaling */  false,
      /* nullReject */  p.get<double>("null-coefficient") != 0,
      /* coeff */       p.get<double>("null-coefficient"),
      /* search */      p.get<int>("K-neighbors")==0,
      /* minK */        p.get<int>("min-K"),
      /* maxK */        p.get<int>("max-K"));

    string distance = p.get<string>("distance");
    if      ( "euclidean" == distance ) k->setDistanceMethod(KNN::EUCLIDEAN_DISTANCE);
    else if ( "cosine" == distance )    k->setDistanceMethod(KNN::COSINE_DISTANCE);
    else if ( "manhattan" == distance ) k->setDistanceMethod(KNN::MANHATTAN_DISTANCE);

    o = k;
  } else if ( "DTW" == name ) {
    vector<string> list = {DTW_REJECTION_MODE};
    o = new DTW(
      /* useScaling */ false,
      /* useNullRejection */ p.get<double>("null-coefficient")!=0,
      /* nullRejectionCoeff */ p.get<double>("null-coefficient"),
      /* rejectionMode */ find(list.begin(), list.end(), p.get<string>("rejection-mode")) - list.begin(),
      /* constrainWarpingPath */ p.get<double>("warping-radius")!=0,
      /* radius */ p.get<double>("warping-radius"),
      /* offsetUsingFirstSample */ false,
      /* useSmoothing */ false,
      /* smoothingFactor */ 0,
      /* nullRjectionLikelihoodThreshold */ p.get<double>("null-threshold"));
  } else if ( "FiniteStateMachine" == name ) {
    o = new FiniteStateMachine(
      p.get<int>("num-particles"),
      p.get<int>("num-clusters"),
      p.get<double>("transition-smoothing"),
      p.get<double>("measurement-noise"));
  } else if ( "ParticleClassifier" == name ) {
    o = new ParticleClassifier(
      p.get<int>("num-particles"),
      p.get<double>("measurement-noise"),
      p.get<double>("transition-sigma"),
      p.get<double>("phase-sigma"),
      p.get<double>("velocity-sigma"));
  } else if ( "RandomForests" == name ) {
    vector<string> list = {RF_TRAINING};
    o = new RandomForests(
      DecisionTreeClusterNode(),
      p.get<int>   ("forest-size"),
      p.get<int>   ("num-split"),
      p.get<int>   ("num-samples"),
      p.get<int>   ("max-depth"),
      find(list.begin(), list.end(), p.get<string>("training-mode")) - list.begin(),
      p.exist("remove-features"),
      true);
  } else if ( "SVM" == name ) {
    vector<string> kernel_list = {SVM_KERNELS};
    vector<string> type_list = {SVM_TYPES};
    o = new SVM(
      find(kernel_list.begin(), kernel_list.end(), p.get<string>("kernel")) - kernel_list.begin(),
      find(type_list.begin(), type_list.end(), p.get<string>("type")) - type_list.begin(),
      true,
      true,
      p.get<double>("gamma") == 0,
      p.get<double>("gamma"),
      p.get<int>("degree"),
      p.get<double>("coef0"),
      p.get<double>("nu"),
      p.get<double>("C"),
      false, 0);
  } else if ( "ANBC" == name ) {
    o = new ANBC(true,p.get<double>("null-coef")!=0,p.get<double>("null-coef"));
  } else if ( "GMM" == name ) {
    o = new GMM(p.get<int>("mixtures"),
      true,
      p.get<double>("null-coef")!=0,
      p.get<double>("null-coef"),
      p.get<int>("max-iterations"),
      p.get<double>("epsilon"));
  } else if ( "AdaBoost" == name ) {
    vector<string> types = {ADABOOST_TYPES};
    UINT type = find(types.begin(),types.end(),p.get<string>("prediction-type")) - types.begin();

    if( "DS" == p.get<string>("weak-classifier") ) {
      o = new AdaBoost(
        DecisionStump(p.get<int>("num-steps")),
        true,
        p.get<double>("null-coef")!=0,
        p.get<double>("null-coef"),
        p.get<int>("max-iterations"),
        type);
    } else if ("RBF" == p.get<string>("weak-classifier") ) {
      o = new AdaBoost(
        RadialBasisFunction(
          p.get<int>("num-steps"),
          p.get<double>("pos-tresh"),
          p.get<double>("min-alpha"),
          p.get<double>("max-alpha")),
        true,
        p.get<double>("null-coef")!=0,
        p.get<double>("null-coef"),
        p.get<int>("max-iterations"),
        type);
    } else {
      cerr << "unknown weak classifier in AdaBoost got: " << p.get<string>("weak-classifier") << endl;
      exit(-1);
    }

  } else if ( "DecisionTree" == name ) {
    vector<string> list = {DT_TRAINING};

    o = new DecisionTree(DecisionTreeNode(),
        p.get<int>("min-samples-per-node"),
        p.get<int>("max-depth"),
        p.exist("remove-features"),
        find(list.begin(), list.end(), p.get<string>("training-mode")) - list.begin(),
        p.get<int>("num-split"),
        false);

  } else if ( "MinDist" == name ) {
    o = new MinDist(false,
        p.get<double>("null-coef")!=0,
        p.get<double>("null-coef"),
        p.get<int>("num-clusters"));
  } else if ( "Softmax" == name ) {
    o = new Softmax(false,
        p.get<double>("learning-rate"),
        p.get<double>("min-change"),
        p.get<double>("max-epochs"));
  } else {
    fstream fin; fin.open(name);
    o = loadClassifierFromFile(fin);
    fin.close();
  }

  if (o != NULL)
    o->setNumInputDimensions(num_dimensions);

  if (p.rest().size() > 0)
    input_file = p.rest()[0];

  return o;
}
