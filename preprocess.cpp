#include <GRT.h>
#include <iostream>
#include "cmdline.h"
#include "libgrt_util.h"

using namespace GRT;
using namespace std;

InfoLog info;
ErrorLog err;

string list_preprocessors() {
  stringstream ss;

  for( auto name : PreProcessing::getRegisteredPreprocessors() )
    ss << name << endl;

  return ss.str();
}

PreProcessing *apply_cmdline_args(string, cmdline::parser&,int,string&);

int main(int argc, const char *argv[]) {
  static bool is_running = true;
  string input_file = "-";
  cmdline::parser c;

  c.add<int>   ("verbose",    'v', "verbosity level: 0-4", false, 0);
  c.add        ("help",       'h', "print this message");
  c.add<string>("type",       't', "force classification, regression or timeseries input", false, "", cmdline::oneof<string>("classification", "regression", "timeseries", "auto"));
  c.footer     ("<pre-processor> [<filename>] ");

  /* parse common options */
  bool parse_ok = c.parse(argc,argv,false) && !c.exist("help");
  set_verbosity(c.get<int>("verbose"));

  /* do we have a predictor? */
  string preproc_name = c.rest().size() > 0 ? c.rest()[0] : "list";
  if (preproc_name == "list") {
    cout << c.usage() << endl;
    cout << list_preprocessors();
    exit(0);
  }

  PreProcessing *pp = apply_cmdline_args(preproc_name,c,1,input_file);

  if (pp==NULL)
    exit(-1);

  if (!parse_ok) {
    cerr << c.usage() << endl << c.error() << endl;
    exit(-1);
  }

  /* do we read from a file or from stdin-? */
  ifstream fin; fin.open(input_file);
  istream &in = input_file=="-" ? cin : fin;

  string line; int linenum=0;
  while(getline(in,line)) {
    stringstream ss(line);

    if (line[0] == '#') {
      cout << line << endl;
      continue;
    }

    if (line.size() == 0) {
      cout << endl;
      continue;
    }

    try { string label; ss >> label; cout << label << "\t"; }
    catch (exception &e) { /* unlabeled data */ }

    vector<double> vals; double value;
    while (ss >> value)
      vals.push_back(value);

    if (linenum == 0) {
      // weird stuff, pp resets only when initialized, it only initialized once
      // data has been seen, and only set num outputdimenstion when reset so:
      pp->setNumInputDimensions(vals.size());
      pp->process(vector<double>(vals.size(), 1.));
      pp->reset();
    }

    bool ok = pp->process(vals);
    if (!ok) {
      cerr << "unable to process line " << linenum << endl;
      exit(-1);
    }

    for(auto value : pp->getProcessedData())
      cout << value << "\t";

    cout << endl;
    linenum++;
  }
}

PreProcessing *apply_cmdline_args(string type, cmdline::parser &c, int num_dimensions, string &input_file) {
  PreProcessing *pp;
  cmdline::parser p;

  if (type == "DeadZone") {
    p.add<double>("lower-limit", 'L', "lower limit for dead-zone", false, -.1);
    p.add<double>("upper-limit", 'U', "upper limit for dead-zone", false,  .1);
  } else if (type == "Derivative") {
    p.add<int>   ("order", 'O', "derivative order of the filter", false, 1, cmdline::oneof<int>(1,2));
    p.add<double>("delta", 'D', "time between in sampleRate/1000.", false, 1);
    p.add<int>   ("filter-size", 'F', "size of the filter, set to zero to disable", false, 3);
  } else if (type == "DoubleMovingAverageFilter" || type == "MovingAverageFilter" || type == "MedianFilter") {
    p.add<int>   ("filter-size", 'F', "size of the filter", false, 5);
  } else if (type == "FIRFilter") {
    p.add<string>("filter-type",  'T', "filter type, one of LPF, HPF, BPF", false, "LPF", cmdline::oneof<string>("LPF","HPF","BPF"));
    p.add<int>   ("num-taps   ",  'N', "number of filter taps", false, 50);
    p.add<double>("sample-duration",  'S', "sample rate of your data", true);
    p.add<double>("cutoff",       'C', "cutoff frequency of the filter", false, 10);
    p.add<double>("gain",         'G', "filter gain", false, 1);
  } else if (type == "HighPassFilter") {
    p.add<double>("factor",        'F', "the smaller this value the more smoothing is done", false, .1);
    p.add<double>("gain",          'G', "multiplies filtered values by this value", false, 1);
    p.add<double>("cutoff",        'C', "set the cutoff frequency in Hz", false, 50),
    p.add<double>("sample-duration",   'R', "set the sample rate of your data, as 1/SR", true);
  } else if (type == "LeakyIntegrator") {
    p.add<double>("leak-rate",     'L', "leak rate", false, 0.99, cmdline::range<double>(0,1.));
  } else if (type == "LowPassFilter") {
    p.add<double>("factor",        'F', "the smaller this value the more smoothing is done", false, .1);
    p.add<double>("gain",          'G', "multiplies filtered values by this value", false, 1);
    p.add<double>("cutoff",        'C', "set the cutoff frequency in Hz", false, 50),
    p.add<double>("sample-duration",   'R', "set the sample rate of your data, as 1/SR", true);
  } else if (type == "SavitzkyGolayFilter") {
    p.add<int>   ("left-hand",       'L', "number of left-hand points for filter design", false, 10);
    p.add<int>   ("right-hand",      'R', "number of right-hand points for filter design", false, 10);
    p.add<int>   ("order",           'O', "derivative order of the filter", false, 0);
    p.add<int>   ("smoothing-order", 'S', "smoothing order, must be one of 2 or 4", false, 2, cmdline::oneof<int>(2,4));
  } else {
    cout << c.usage() << endl;
    cout << list_preprocessors() << endl;
    cerr << "unable to load preprocessor " << type << endl;
    return NULL;
  }

  if (!p.parse(c.rest()) || c.exist("help")) {
    cerr << c.usage() << endl << "pre processing options:" << endl << p.str_options() << endl << p.error() << endl;
    exit(-1);
  }

  if (type == "DeadZone") {
    pp = new DeadZone(
        p.get<double>("lower-limit"),
        p.get<double>("upper-limit"),
        num_dimensions);
  } else if (type == "Derivative") {
    pp = new Derivative(
        p.get<int>   ("order"),
        p.get<double>("delta"),
        num_dimensions,
        p.get<int>  ("filter-size") != 0,
        p.get<int>  ("filter-size"));
  } else if (type == "DoubleMovingAverageFilter") {
    pp = new DoubleMovingAverageFilter(
        p.get<int>  ("filter-size"),
        num_dimensions);
  } else if (type == "FIRFilter") {
    vector<string> list = {"LPF","HPF","BPF"};
    pp = new FIRFilter(
        find(list.begin(),list.end(),p.get<string>("func")) - list.begin(),
        p.get<int>("num-taps"),
        p.get<double>("sample-duration"),
        p.get<double>("cutoff"),
        p.get<double>("gain"),
        num_dimensions);
  } else if (type == "HighPassFilter") {
    pp = new HighPassFilter(
        p.get<double>("factor"),
        p.get<double>("gain"),
        num_dimensions,
        p.get<double>("cutoff"),
        p.get<double>("sample-duration"));
  } else if (type == "LeakyIntegrator") {
    pp = new LeakyIntegrator(
        p.get<double>("leak-rate"),
        num_dimensions);
  } else if (type == "LowPassFilter") {
    pp = new LowPassFilter(
        p.get<double>("factor"),
        p.get<double>("gain"),
        num_dimensions,
        p.get<double>("cutoff"),
        p.get<double>("sample-duration"));
  } else if (type == "MedianFilter") {
    pp = new MedianFilter(
        p.get<int>("filter-size"),
        num_dimensions);
  } else if (type == "MovingAverageFilter") {
    pp = new MovingAverageFilter(
        p.get<int>("filter-size"),
        num_dimensions);
  } else if(type == "SavitzkyGolayFilter") {
    pp = new SavitzkyGolayFilter(
        p.get<int>("left-hand"),
        p.get<int>("right-hand"),
        p.get<int>("order"),
        p.get<int>("smoothing-order"),
        num_dimensions);
  }

  if (p.rest().size() > 0)
    input_file = p.rest()[0];

  return pp;
}
