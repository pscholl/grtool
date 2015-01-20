#include "libgrt_util.h"
#include "cmdline.h"

int main(int argc, char *argv[]) 
{
  static bool is_running = true;
  cmdline::parser c;

  c.add<int>   ("verbose",    'v', "verbosity level: 0-4", false, 0);
  c.add        ("help",       'h', "print this message");
  c.add<string>("model",      'm', "file to store trained classifier", true);
  c.add<string>("type",       't', "force classification, regression or timeseries input", false, "", cmdline::oneof<string>("classification", "regression", "timeseries", "auto"));
  c.footer     ("[filename]...");

  /* parse the classifier-common arguments */
  if (!c.parse(argc,argv,false)) {
    cerr << c.usage() << "\n" << c.error() << "\n" ;
    return -1;
  }

  /* handling of TERM and INT signal and set verbosity */
  set_running_indicator(&is_running);
  set_verbosity(c.get<int>("verbose"));

  /* load a classification model */
  string model_file = c.get<string>("model");
  Classifier *classifier = loadFromFile(model_file);

  if (classifier == NULL) {
    cerr << "unable to load classification model " << model_file << " giving up" << endl;
    return -1;
  }

  /* prepare input */
  string data_type = c.get<string>("type");
  CsvIOSample io(data_type);

  string filename = c.rest().size() > 0 ? c.rest()[0] : "-";
  ifstream inf(filename);
  if (filename!="-" && !inf) {
    cerr << "unable to open file: " << filename << endl;
    return -1;
  }
  istream &in = filename != "-" ? inf : cin;

  /* read and predict on input */
  while( in >> io && is_running ) {
    UINT prediction = 0, label = 0;

    switch(io.type) {
    case TIMESERIES:
      classifier->predict(io.t_data.getData());
      label = io.t_data.getClassLabel();
      prediction = classifier->getPredictedClassLabel();
      break;
    case CLASSIFICATION:
      classifier->predict(io.c_data.getSample());
      label = io.c_data.getClassLabel();
      prediction = classifier->getPredictedClassLabel();
      break;
    default:
      cerr << "unknown input type" << endl;
      return -1;
    }

    cout << label << "\t" << prediction << endl;
  }

  return 0;
}
