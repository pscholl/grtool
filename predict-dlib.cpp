#include <iostream>
#include <stdio.h>

#include "cmdline.h"
#include "dlib_trainers.h"

using namespace std;
using namespace dlib;

a_df df_from_stream(istream &model);

//_______________________________________________________________________________________________________
int main(int argc, char *argv[])
{

  /*
      ########     ###    ########   ######  #### ##    ##  ######
      ##     ##   ## ##   ##     ## ##    ##  ##  ###   ## ##    ##
      ##     ##  ##   ##  ##     ## ##        ##  ####  ## ##
      ########  ##     ## ########   ######   ##  ## ## ## ##   ####
      ##        ######### ##   ##         ##  ##  ##  #### ##    ##
      ##        ##     ## ##    ##  ##    ##  ##  ##   ### ##    ##
      ##        ##     ## ##     ##  ######  #### ##    ##  ######
  */

  cmdline::parser c;

  c.add        ("help",    'h', "print this message");
  c.footer     ("<classifier> [classifier-model-file] [testsample-file]...");

  /* parse common arguments */
  if (!c.parse(argc, argv, false)) {
    cerr << c.error() << endl;
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

  a_df df = df_from_stream(model);



  /*
      ########  ########    ###    ########      ######     ###    ##     ## ########  ##       ########  ######
      ##     ## ##         ## ##   ##     ##    ##    ##   ## ##   ###   ### ##     ## ##       ##       ##    ##
      ##     ## ##        ##   ##  ##     ##    ##        ##   ##  #### #### ##     ## ##       ##       ##
      ########  ######   ##     ## ##     ##     ######  ##     ## ## ### ## ########  ##       ######    ######
      ##   ##   ##       ######### ##     ##          ## ######### ##     ## ##        ##       ##             ##
      ##    ##  ##       ##     ## ##     ##    ##    ## ##     ## ##     ## ##        ##       ##       ##    ##
      ##     ## ######## ##     ## ########      ######  ##     ## ##     ## ##        ######## ########  ######
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





//_______________________________________________________________________________________________________
a_df df_from_stream(istream &model) {
  a_df df;

  char t[32], k[32];
  model.getline(t, 32);
  model.getline(k, 32);

  string trainer(t), kernel(k);

  if (trainer == TrainerName::ONE_VS_ONE) {
    if (kernel == "hist") {
      df.get<ovo_trained_function_type_hist_df>();
      deserialize(df.cast_to<ovo_trained_function_type_hist_df>(), model);
    }
    else if (kernel == "lin") {
      df.get<ovo_trained_function_type_lin_df>();
      deserialize(df.cast_to<ovo_trained_function_type_lin_df>(), model);
    }
    else if (kernel == "rbf") {
      df.get<ovo_trained_function_type_rbf_df>();
      deserialize(df.cast_to<ovo_trained_function_type_rbf_df>(), model);
    }
    else if (kernel == "poly") {
      df.get<ovo_trained_function_type_poly_df>();
      deserialize(df.cast_to<ovo_trained_function_type_poly_df>(), model);
    }
    else if (kernel == "sig") {
      df.get<ovo_trained_function_type_sig_df>();
      deserialize(df.cast_to<ovo_trained_function_type_sig_df>(), model);
    }
    else {
      df.get<ovo_trained_function_type>();
      deserialize(df.cast_to<ovo_trained_function_type>(), model);
    }
  }
  else if (trainer == TrainerName::ONE_VS_ALL) {
    if (kernel == "hist") {
      df.get<ova_trained_function_type_hist_df>();
      deserialize(df.cast_to<ova_trained_function_type_hist_df>(), model);
    }
    else if (kernel == "lin") {
      df.get<ova_trained_function_type_lin_df>();
      deserialize(df.cast_to<ova_trained_function_type_lin_df>(), model);
    }
    else if (kernel == "rbf") {
      df.get<ova_trained_function_type_rbf_df>();
      deserialize(df.cast_to<ova_trained_function_type_rbf_df>(), model);
    }
    else if (kernel == "poly") {
      df.get<ova_trained_function_type_poly_df>();
      deserialize(df.cast_to<ova_trained_function_type_poly_df>(), model);
    }
    else if (kernel == "sig") {
      df.get<ova_trained_function_type_sig_df>();
      deserialize(df.cast_to<ova_trained_function_type_sig_df>(), model);
    }
    else {
      df.get<ova_trained_function_type>();
      deserialize(df.cast_to<ova_trained_function_type>(), model);
    }
  }
  else if (trainer == TrainerName::SVM_MULTICLASS_LINEAR) {
    df.get<svm_ml_trained_function_type>();
    deserialize(df.cast_to<svm_ml_trained_function_type>(), model);
  }

  return df;
}
