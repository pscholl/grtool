#include "libgrt_util.h"
#include "cmdline.h"

int main(int argc, char *argv[]) 
{
  static bool is_running = true;
  cmdline::parser c;

  c.add<int>   ("verbose",    'v', "verbosity level: 0-4", false, 0);
  c.add        ("help",       'h', "print this message");
  c.add        ("likelihood", 'l', "print label_prediction likelihood instead of label and prediction");
  c.footer     ("[classifier-model-file] [filename]...");

  /* parse the classifier-common arguments */
  if (!c.parse(argc,argv,true) || c.exist("help")) {
    cerr << c.usage() << "\n" << (c.exist("help") ? c.error() : "") << "\n" ;
    return -1;
  }

  set_verbosity(c.get<int>("verbose"));

  /* load a classification model */
  ifstream fin; fin.open(c.rest().size() ? c.rest()[0] : "");
  istream &model = c.rest().size() ? fin : cin;

  /* read and predict on input */
  Classifier *classifier = loadClassifierFromFile(model);

  /* prepare input */
  string data_type = classifier->getTimeseriesCompatible() ? "timeseries" : "classification";
  CsvIOSample io(data_type);
  istream &in = grt_fileinput(c,1);

  while( in >> io && is_running ) {
    UINT prediction = 0, label = 0;
    string s_prediction, s_label;
    bool result = false;

    /* load the classifier only after the first data has arrived, so
     * we give the preceding command (when used in a pipe) enough time
     * to write the classifier to disk */
    if (classifier == NULL && c.rest().size() > 0)
      for (int i=0; i<255*255 && classifier==NULL; i++, usleep(10*100)) {
        ifstream fin(c.rest()[0]);
        classifier = loadClassifierFromFile(fin);
      }

    if (classifier == NULL) {
      cerr << "unable to load classification model giving up" << endl;
      return -1;
    }

    switch(io.type) {
    case TIMESERIES:
      result = classifier->predict(io.t_data.getData());
      label = io.t_data.getClassLabel();
      s_label = classifier->getClassNameForLabel(label);
      prediction = classifier->getPredictedClassLabel();
      s_prediction = classifier->getClassNameForLabel(prediction);
      break;
    case CLASSIFICATION:
      result = classifier->predict(io.c_data.getSample());
      label = io.c_data.getClassLabel();
      s_label = classifier->getClassNameForLabel(label);
      prediction   = classifier->getPredictedClassLabel();
      s_prediction = classifier->getClassNameForLabel(prediction);
      break;
    default:
      cerr << "unknown input type" << endl;
      return -1;
    }

    if (!result) {
      cerr << "prediction failed (wrong input type?)" << endl;
      return -1;
    }

    if (label == 0) s_label = "NULL";
    if (prediction == 0) s_prediction = "NULL";

    if (c.exist("likelihood"))
      cout << s_label << "_" << s_prediction << "\t" << classifier->getMaximumLikelihood() << endl;
    else
      cout << s_label << "\t" << s_prediction << endl;
  }

  cout << endl;
  return 0;
}
