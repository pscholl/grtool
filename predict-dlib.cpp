#include <iostream>
#include <stdio.h>

#include "cmdline.h"
#include "dlib_trainers.h"

using namespace std;
using namespace dlib;

a_df df_from_model(string name, istream &model);

int main(int argc, char *argv[])
{
  /*
   * ARGUMENT PARSING
   */

  cmdline::parser c;

  c.add        ("help",    'h', "print this message");
  c.footer     ("<classifier> [classifier-model-file] [testsample-file]...");

  /* parse common arguments */
  if (!c.parse(argc, argv, false)) {
    cerr << c.error() << endl;
    return -1;
  }
  if (c.rest().size() == 0) {
    cout << c.usage() << endl;
    cout << "Available Classifiers:" << endl;
    printTrainers();
    return -1;
  }

  string classifier_str = c.rest()[0];

  if (!classifierExists(classifier_str)) {
    cout << c.usage() << endl;
    cout << "Available Classifiers:" << endl;
    printTrainers();
    return -1;
  }

  if (c.exist("help")) {
    cout << c.usage() << endl;
    cout << "Available Classifiers:" << endl;
    printTrainers();
    return 0;
  }

  string model_file = c.rest().size() > 1 ? c.rest()[1] : "";
  string tests_file = c.rest().size() > 2 ? c.rest()[2] : "";

  /* load a classification model */
  ifstream fin_model; fin_model.open(model_file);
  istream &model = c.rest().size() > 1 ? fin_model : cin;

  /* load test samples */
  ifstream fin_tests; fin_tests.open(tests_file);
  istream &tests = c.rest().size() > 2 ? fin_tests : cin;

  if (!model.good()) {
    cerr << "unable to open model input file: " << model_file << endl;
    return -1;
  }

  if (!tests.good()) {
    cerr << "unable to open tests input file: " << tests_file << endl;
    return -1;
  }

  a_df df = df_from_model(classifier_str, model);


  /*
   * READ PREDICTION SAMPLES
   */

  /* read samples */
  std::vector<sample_type> samples;
  std::vector<string> labels;
  std::set<string> u_labels;

  string line, label;

  while (getline(tests, line)) {
    stringstream ss(line);

    if (line.find_first_not_of(" \t") == string::npos) {
      if (samples.size() != 0)
        break;
      else
        continue;
    }

    if (line[0] == '#')
      continue;

    ss >> label;
    std::vector<double> sample;
    string val;
    while (ss >> val) // this also handles nan and infs correctly
      sample.push_back(strtod(val.c_str(), NULL));

    if (sample.size() == 0)
      continue;

    samples.push_back(mat(sample));
    labels.push_back(label);
    u_labels.insert(label);
  }





  /*
   * PREDICTION
   */

  for (size_t i = 0; i < samples.size(); ++i)
    cout << labels[i] << "\t" << df(samples[i]) << endl;


  cout << endl;
  return 0;
}



a_df df_from_model(string name, istream &model) {
  a_df df;

  if (name == TrainerName::ONE_VS_ONE) {
    df.get<ovo_trained_function_type_rbf_df>();
    deserialize(df.cast_to<ovo_trained_function_type_rbf_df>(), model);
  }
  else if (name == TrainerName::ONE_VS_ALL) {
    df.get<ova_trained_function_type_rbf_df>();
    deserialize(df.cast_to<ova_trained_function_type_rbf_df>(), model);
  }
  else if (name == TrainerName::SVM_MULTICLASS_LINEAR) {
    df.get<svm_ml_trained_function_type>();
    deserialize(df.cast_to<svm_ml_trained_function_type>(), model);
  }

  return df;
}
