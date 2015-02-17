#include "libgrt_util.h"
#include "cmdline.h"

int main(int argc, char *argv[]) 
{
  static bool is_running = true;
  cmdline::parser c;

  c.add<int>   ("verbose",    'v', "verbosity level: 0-4", false, 0);
  c.add        ("help",       'h', "print this message");
  c.add<string>("type",       't', "force classification, regression or timeseries input", false, "", cmdline::oneof<string>("classification", "regression", "timeseries", "auto"));
  c.add<string>("title",      'T', "prepend the output with an optional title", false, "");
  c.footer     ("<classifier-model-file> [filename]...");

  /* parse the classifier-common arguments */
  if (!c.parse(argc,argv,true) || c.exist("help")) {
    cerr << c.usage() << "\n" << (c.exist("help") ? c.error() : "") << "\n" ;
    return -1;
  }

  /* handling of TERM and INT signal and set verbosity */
  //set_running_indicator(&is_running);
  set_verbosity(c.get<int>("verbose"));

  /* load a classification model */
  string model_file = c.rest().size() > 0 ? c.rest()[0] : "";

  /* prepare input */
  string data_type = c.get<string>("type");
  CsvIOSample io(data_type);
  istream &in = grt_fileinput(c,1);

  /* print title if any */
  string title = c.get<string>("title");
  if (title != "")
    cout << title << endl;

  /* read and predict on input */
  while( in >> io && is_running ) {
    UINT prediction = 0, label = 0;
    string s_prediction, s_label;

    /* load the classifier only after the first data has arrived, so
     * we give the preceding command (when used in a pipe) enough time
     * to write the classifier to disk */
    Classifier *classifier = NULL; // TODO this needs to be loaded from stdin for piped cases!
    for (int i=0; i<255 && classifier==NULL; i++, usleep(10*100))
      classifier = loadClassifierFromFile(model_file);

    if (classifier == NULL) {
      cerr << "unable to load classification model " << model_file << " giving up" << endl;
      return -1;
    }

    switch(io.type) {
    case TIMESERIES:
      classifier->predict(io.t_data.getData());
      label = io.t_data.getClassLabel();
      s_label = classifier->getClassNameForLabel(label);
      prediction = classifier->getPredictedClassLabel();
      s_prediction = classifier->getClassNameForLabel(prediction);
      break;
    case CLASSIFICATION:
      classifier->predict(io.c_data.getSample());
      label = io.c_data.getClassLabel();
      s_label = classifier->getClassNameForLabel(label);
      prediction   = classifier->getPredictedClassLabel();
      s_prediction = classifier->getClassNameForLabel(prediction);
      break;
    default:
      cerr << "unknown input type" << endl;
      return -1;
    }

    if (s_prediction != "" && s_label != "")
      cout << s_label << "\t" << s_prediction << endl;
    else
      cout << label << "\t" << prediction << endl;
  }

  cout << endl;
  return 0;
}
