#include <dlib/svm_threaded.h>

#include <iostream>
#include <stdio.h>
#include "cmdline.h"

using namespace std;
using namespace dlib;


typedef matrix<double, 0, 1> sample_type;  // init as 0,1 ; can be cast to arbitrary num_rows

int main(int argc, char *argv[])
{
  /*
   * ARGUMENT PARSING
   */

  cmdline::parser c;

  c.add        ("help",    'h', "print this message");
  c.footer     ("[classifier-model-file] [testsample-file]...");

  /* parse common arguments */
  bool parse_ok = c.parse(argc, argv, false) && !c.exist("help");

  if (!parse_ok) {
    cerr << c.usage() << endl << c.error() << endl;
    return -1;
  }

  string model_file = c.rest().size() > 0 ? c.rest()[0] : "";
  string tests_file = c.rest().size() > 1 ? c.rest()[1] : "";

  /* load a classification model */
  ifstream fin_model; fin_model.open(model_file);
  istream &model = c.rest().size() > 0 ? fin_model : cin;

  /* load test samples */
  ifstream fin_tests; fin_tests.open(tests_file);
  istream &tests = c.rest().size() > 1 ? fin_tests : cin;

  if (!model.good()) {
    cerr << "unable to open model input file: " << model_file << endl;
    return -1;
  }

  if (!tests.good()) {
    cerr << "unable to open tests input file: " << tests_file << endl;
    return -1;
  }



  /*
   * READ PREDICTION SAMPLES AND MODEL
   */

  /* read model */
  one_vs_one_decision_function<one_vs_one_trainer<any_trainer<sample_type>, string>, decision_function<radial_basis_kernel<sample_type>>> df;
  deserialize(df, model);


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
