#include <GRT.h>
#include <iostream>
#include "cmdline.h"
#include "libgrt_util.h"

using namespace GRT;
using namespace std;

string list_extractors();
bool apply_cmdline_args(string, FeatureExtraction*, cmdline::parser&);
void feature(FeatureExtraction *, std::string);

InfoLog info;
ErrorLog err;

int main(int argc, const char *argv[]) {
  static bool is_running = true;
  cmdline::parser c;

  c.add<int>   ("verbose",    'v', "verbosity level: 0-4", false, 0);
  c.add        ("help",       'h', "print this message");
  c.add<string>("type",       't', "force classification, regression or timeseries input", false, "", cmdline::oneof<string>("classification", "regression", "timeseries", "auto"));
  c.add<int>   ("num-samples",'n', "number of input samples to use for training", true, 0);
  c.footer     ("<feature-extractor> [<filename>] ");

  bool parse_ok = !c.parse(argc,argv) || c.exist("help");

  /* react on SIGINT and set verbosity */
  set_running_indicator(&is_running);
  set_verbosity(c.get<int>("verbose"));

  /* load the extractor and check wether we need to list them */
  string str_extractor = c.rest().size() > 0 ? c.rest()[0] : "list";
  if (str_extractor == "list") {
    cout << c.usage() << endl;
    cout << list_extractors();
    return 0;
  }

  /* try and create an instance */
  FeatureExtraction *f = FeatureExtraction::createInstanceFromString( str_extractor );
  if (f == NULL) {
    cout << c.usage() << endl;
    cout << list_extractors() << endl;
    err << "error: unable to load feature extractor" << str_extractor << endl;
    return -1;
  }

  /* create and apply extractor specific parameters */
  if (!apply_cmdline_args(str_extractor, f, c))
    return -1;

  if (!parse_ok) {
    err << c.usage() << endl << c.error() << endl;
    return -1;
  }

  /* do we read from a file or from stdin? */
  istream &in = grt_fileinput(c);

  /* check if the number of input is limited */
  int num_training_samples = c.get<int>("num-samples"), num_lines = 0;
  vector<string> lines; string line;
  MatrixDouble dataset;

  /* read the number of training samples */
  while (getline(in, line) && (num_training_samples==0 || num_lines < num_training_samples) && is_running) {
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

  /* now train the extractor with the data */
  f->train(dataset);

  /* and print what we already consumed */
  for (auto line : lines)
    feature( f, line );

  /* now transforms the rest of the input */
  while (getline(in, line) && is_running)
    feature( f, line );
}

string list_extractors() {
  stringstream ss;

  for( auto name : FeatureExtraction::getRegisteredFeatureExtractors())
    ss << name << endl;

  return ss.str();
}

bool
apply_cmdline_args(string type, FeatureExtraction* f, cmdline::parser &c) {
  cmdline::parser p;

  // TODO
  //  FFT
  //  FFTFeatures
  //  KMeansFeatures
  //  MovementIndex
  //  MovementTrajectoryFeatures
  //  TimeDomainFeatures
  //  TimeseriesBuffer
  //  ZeroCrossingCounter

  if ( type == "RBMQuantizer" ||
       type == "SOMQuantizer" ||
       type == "KMeansQuantizer" )
    p.add<int> ("num-clusters", 'k', "the number of clusters", true, 10);

  if (!p.parse(c.rest())) {
    err << c.usage() << endl << "feature extraction options:" << endl << p.str_options() << endl << p.error() << endl;
    return false;
  }

  if ( type == "RBMQuantizer" )
  {
    RBMQuantizer *q = (RBMQuantizer*) f;
    q->setNumClusters( p.get<int>("num-clusters") );
  }
  else if ( type == "SOMQuantizer" )
  {
    SOMQuantizer *q = (SOMQuantizer*) f;
    q->setNumClusters( p.get<int>("num-clusters") );
  }
  else if ( type == "KMeansQuantizer" )
  {
    KMeansQuantizer *q = (KMeansQuantizer *) f;
    q->setNumClusters( p.get<int>("num-clusters") );
  }

  c.rest() = p.rest();
  return true;
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
