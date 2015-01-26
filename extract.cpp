#include <GRT.h>
#include <iostream>
#include "cmdline.h"
#include "libgrt_util.h"

using namespace GRT;
using namespace std;

string list_extractors();
FeatureExtraction* apply_cmdline_args(string, cmdline::parser&);
void feature(FeatureExtraction *, std::string);

InfoLog info;

int main(int argc, const char *argv[]) {
  cmdline::parser c;

  c.add<int>   ("verbose",    'v', "verbosity level: 0-4", false, 0);
  c.add        ("help",       'h', "print this message");
  c.add<string>("type",       't', "force classification, regression or timeseries input", false, "", cmdline::oneof<string>("classification", "regression", "timeseries", "auto"));
  c.add<int>   ("num-samples",'n', "number of input samples to use for training", true, 0);
  c.add<string>("model",      'o', "if given store the model in file, for later usage", false);
  c.footer     ("<feature-extractor or input file name> [<filename>] ");

  bool parse_ok = !c.parse(argc,argv) || c.exist("help");

  /* react on SIGINT and set verbosity */
  set_verbosity(c.get<int>("verbose"));

  /* load the extractor and check wether we need to list them */
  string str_extractor = c.rest().size() > 0 ? c.rest()[0] : "list";
  if (str_extractor == "list") {
    cout << c.usage() << endl;
    cout << list_extractors();
    return 0;
  }

  /* try and create an instance */
  FeatureExtraction* f = apply_cmdline_args(str_extractor, c);
  ifstream maybefile( str_extractor );
  if (f == NULL && maybefile.good())
    f = loadFeatureExtractionFromFile( str_extractor );

  if (f == NULL) {
    cout << c.usage() << endl;
    cout << list_extractors() << endl;
    cerr << "error: unable to load feature extractor: " << str_extractor << endl;
    return -1;
  }

  /* create and apply extractor specific parameters */
  if (f == NULL) return -1;

  if (!parse_ok) {
    cerr << c.usage() << endl << c.error() << endl;
    return -1;
  }

  /* do we read from a file or from stdin? */
  istream &in = grt_fileinput(c);

  /* check if the number of input is limited */
  int num_training_samples = c.get<int>("num-samples"), num_lines = 0;
  vector<string> lines; string line;
  MatrixDouble dataset;

  if (f->getTrained() && c.exist("model")) {
    cerr << "refusing to load and store already trained model" << endl;
    return -1;
  }

  if (f->getTrained() && c.exist("num-samples")) {
    cerr << "limiting the number of training samples on already trained exrtactor makes no sense" << endl;
    return -1;
  }

  /* read the number of training samples */
  if (!f->getTrained()) {
    while (getline(in, line) && (num_training_samples==0 || num_lines < num_training_samples)) {
      stringstream ss(line);
      vector<double> data; double value;
      string label;

      lines.push_back(line);

      if (line=="" || line[0]=='#')
        ;

      num_lines++;

      ss >> label;
      while (ss >> value)
        data.push_back(value);

      dataset.push_back(data);
    }

    f->train(dataset);

    /* if there is a model file store it */
    if (c.exist("model"))
      f->save(c.get<string>("model"));

    /* and print what we already consumed */
    for (auto line : lines)
      feature( f, line );
  }

  /* now transforms the rest of the input */
  while (getline(in, line))
    feature( f, line );
}

string list_extractors() {
  stringstream ss;

  for( auto name : FeatureExtraction::getRegisteredFeatureExtractors())
    ss << name << endl;

  return ss.str();
}

FeatureExtraction*
apply_cmdline_args(string type, cmdline::parser &c) {
  FeatureExtraction *f;
  cmdline::parser p;

  if ( type == "RBMQuantizer" || type == "SOMQuantizer" || type == "KMeansQuantizer" )
    p.add<int> ("clusters", 'K', "the number of clusters", false, 10);

  else if ( type == "FFT" ) {
    p.add<int>    ("window",       'W', "window size in samples (should be power of two)", false, 512);
    p.add<int>    ("hop",          'H', "every Hth sample the FFT will be computed", false, 1);
    p.add<string> ("func",         'F', "window function, one of 'rect', 'bartlett','hamming','hanning'", false, "rect", cmdline::oneof<string>("rect","bartlett","hamming","hanning"));
    p.add         ("no-magnitude", 'M', "if magnitude should not be computed");
    p.add         ("no-phase",     'P', "set if phase should not be computed");
  }

  else if ( type == "FFTFeatures" ) {
    p.add<int>    ("window",          'W', "window size in samples (should be power of two)", false, 512);
    p.add         ("no-max-freq",     'N', "do not compute largest frequency");
    p.add         ("no-max-freq-spec",'M', "do not compute maximum-frequency spectrum frequency feature");
    p.add         ("no-centroid"   ,  'C', "do not compute center frequency");
    p.add<int>    ("top-K",           'K', "compute the top-K frequencies (0 disabels this feature)", false, 10);
  }

  else if ( type == "KMeansFeatures" ) 
    p.add<string> ("clusters",   'K', "list of cluster numbers as a comma-separated list", false, "1,100");

  else if ( type == "MovementIndex" )
    p.add<int>   ("samples",     'K', "number of samples hold to compute movement index", false, 100);

  else if ( type == "MovementTrajectoryFeatures" ) {
    p.add<int>   ("samples",     'K', "number of sample for trajectory computation", false, 100);
    p.add<int>   ("centroids",   'C', "number of centroids to calculate", false, 10);
    p.add<string>("feature-mode",'M', "feature mode: centroid,normalized,derivative,angle_2d", false, "centroid", cmdline::oneof<string>("centroid","normalized","derivative","angle_2d"));
    p.add<int>   ("histograms",  'H', "number of histogram bins", false, 10);
    p.add        ("start-end",   'S', "use trajectory start and end values");
    p.add        ("no-weighted-magnitude", 'W', "do not weight the computed magnitude");
  }

  else if ( type == "TimeDomainFeatures" ) {
    p.add<int>   ("samples",     'K', "number of sample for computation", false, 100);
    p.add<int>   ("frames",      'F', "frames for computation", false, 10);
    p.add        ("offset",      'O', "wether to offset the input");
    p.add        ("no-mean",     'M', "do not compute mean");
    p.add        ("no-stddev",   'S', "do not compute standard deviation");
    p.add        ("no-euclidean",'E', "do not compute euclidean norm");
    p.add        ("no-rms",      'R', "do not compute root mean squared error");
  }

  else if ( type == "TimeseriesBuffer" ) {
    p.add<int>   ("samples",     'K', "number of sample for computation", false, 100);
  }

  else if ( type == "ZeroCrossingCounter" ) {
    p.add<int>   ("window",      'W', "size of search window in samples", false, 20);
    p.add<double>("deadzone",    'D', "deadzone threshold", false, 0.01);
    p.add        ("combined",    'C', "wether zero-crossing are calculated independently");
  }

  if (!p.parse(c.rest()) || c.exist("help")) {
    cerr << c.usage() << endl << "feature extraction options:" << endl << p.str_options() << endl << p.error() << endl;
    return NULL;
  }

  if ( type == "RBMQuantizer" )
    f = new RBMQuantizer( p.get<int>("clusters") );

  else if ( type == "SOMQuantizer" )
    f = new SOMQuantizer( p.get<int>("clusters") );

  else if ( type == "KMeansQuantizer" )
    f = new KMeansQuantizer( p.get<int>("clusters") );

  else if ( type == "FFT" ) {
    vector<string> list = {"rect","bartlett","hamming","hanning"};
    f = new FFT(
        p.get<int>("window"),
        p.get<int>("hop"),
        find(list.begin(),list.end(),p.get<string>("func")) - list.begin(),
        !p.exist("no-magnitude"),
        !p.exist("no-phase"));

  } else if ( type == "FFTFeatures" ) {
    f = new FFTFeatures(
        p.get<int>("window"),
        1,
        !p.exist("no-max-freq"),
        !p.exist("no-max-freq-spec"),
        !p.exist("no-centroid"),
        p.get<int>("top-K")!=0,
        p.get<int>("top-K"));

  } else if ( type == "KMeansFeatures" ) {
    stringstream ss(p.get<string>("clusters"));
    vector<unsigned int> list;
    string token;
    while(getline(ss,token,','))
      list.push_back(stoi(token));
    f = new KMeansFeatures(list);

  } else if ( type == "MovementIndex" ) {
    f = new MovementIndex(p.get<int>("samples"));

  } else if ( type == "MovementTrajectoryFeatures" ) {
    vector<string> list = {"centroid","normalized","derivative","angle_2d"};
    f = new MovementTrajectoryFeatures(
        p.get<int>("samples"),
        p.get<int>("centroids"),
        find(list.begin(), list.end(), p.get<string>("feature-mode")) - list.begin(),
        p.get<int>("histograms"),
        1,
        p.exist("start-end"),
        p.exist("no-weighted-magnitude"));
  
  } else if ( type == "TimeDomainFeatures" ) {
    f = new TimeDomainFeatures(
        p.get<int>("samples"),
        p.get<int>("frames"),
        p.exist("offset"),
        !p.exist("no-mean"),
        !p.exist("no-stddev"),
        !p.exist("no-euclidean"),
        !p.exist("no-rms"));

  } else if ( type == "TimeseriesBuffer" ) {
    f = new TimeseriesBuffer(
        p.get<int>("samples"));

  } else if ( type == "ZeroCrossingCounter" ) {
    f = new ZeroCrossingCounter(
        p.get<int>("window"),
        p.get<double>("deadzone"),
        p.exist("combined"));
  }

  c.rest() = p.rest();
  return f;
}

void feature(FeatureExtraction *f, std::string line)
{
  stringstream ss(line);
  vector<double> data;
  double value;
  string label;

  if (line=="" || line[0]=='#') {
    cout << line << endl;
    return;
  }

  ss >> label;
  while (ss >> value)
    data.push_back(value);

  f->computeFeatures(data);

  cout << label;
  for (auto val : f->getFeatureVector())
    cout << "\t" << val;
  cout << endl;
}
