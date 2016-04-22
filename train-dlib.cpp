#include <iostream>
#include <stdio.h>

#include "cmdline.h"
#include "dlib_trainers.h"

using namespace std;
using namespace dlib;


int main(int argc, const char *argv[])
{
  /*
   *  ARGUMENT PARSING
   */

  string input_file = "-";
  cmdline::parser c;

  c.add        ("help",    'h', "print this message");
  c.add        ("verbose",    'v', "be verbose");
  c.add<int>   ("cross-validate", 'c', "perform k-fold cross validation", false, 0);
  c.add<string>("output",  'o', "store trained classifier in file", false);
  c.add<string>("trainset",'n', "limit the training dataset to the first n samples, if n is less than or equal 1 it is interpreted the percentage of a stratified random split that is retained for training. If not a number it is interpreted as a filename containing training samples.", false, "1");
  c.footer     ("<classifier> [input-data]...");

  /* parse common arguments */
  if (!c.parse(argc, argv, false)) {
    cerr << c.error() << endl;
    return -1;
  }
  if (c.exist("help")) {
    cout << c.usage() << endl;
    cout << "Available Classifiers:" << endl;
    printTrainers();
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

  /* check if we can open the output file */
  ofstream fout(c.get<string>("output"), ios_base::out | ios_base::binary);
  ostream &output = c.exist("output") ? fout : cout;

  if (c.exist("output") && !output.good()) {
    cerr << c.usage() << endl << "unable to open \"" << c.get<string>("output") << "\" as output" << endl;
    return -1;
  }

  if (c.rest().size() > 1)
    input_file = c.rest()[1];

  /* do we read from a file or stdin? */
  ifstream fin; fin.open(input_file);
  istream &in = input_file=="-" ? cin : fin;

  if (!in.good()) {
    cerr << "unable to open input file " << input_file << endl;
    return -1;
  }


  /*
   *  READ TRAINING SAMPLES
   */

  v_sample_type samples;
  v_label_type labels;
  std::set<string> u_labels;

  string line, label;

  while (getline(in, line)) {
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
   *  TRAINING
   */
  trainer_template* trainer;

  // create trainer
  if (classifier_str == "ONE_VS_ONE")
    trainer = new ovo_trainer(c.exist("verbose"), 8);
  else if (classifier_str == "ONE_VS_ALL")
    trainer = new ova_trainer(c.exist("verbose"), 8);
  else if (classifier_str == "SVM_MULTICLASS_LINEAR")
    trainer = new svm_ml_trainer(c.exist("verbose"), 8);

  else {
    cout << "wtf" << endl;
    exit(-1);
  }


  // cross-validate, or train and serialize
  if (c.get<int>("cross-validate") > 0) {
    // randomize and cross-validate samples
    randomize_samples(samples, labels);
    matrix<double> cv_result = trainer->crossValidation(samples, labels, c.get<int>("cross-validate"));
    cout << c.get<int>("cross-validate") << "-fold cross-validation:" << endl << cv_result << endl;

    cout << "number of samples: " << samples.size() << endl;
    cout << "number of unique labels: " << u_labels.size() << endl << endl;

    cout << "accuracy: " << trace(cv_result) / sum(cv_result) << endl;
    cout << "F1-score: " << (2 * trace(cv_result)) / (trace(cv_result) + sum(cv_result)) << endl;
  }

  else if (classifier_str == "ONE_VS_ONE") {
      ovo_trained_function_type_df df = trainer->train(samples, labels).cast_to<ovo_trained_function_type>();
      serialize(df, output);
  }
  else if (classifier_str == "ONE_VS_ALL") {
      ova_trained_function_type_df df = trainer->train(samples, labels).cast_to<ova_trained_function_type>();
      serialize(df, output);
  }
  else if (classifier_str == "SVM_MULTICLASS_LINEAR") {
      svm_ml_trained_function_type df = trainer->train(samples, labels).cast_to<svm_ml_trained_function_type>();
      serialize(df, output);
  }


  if (!c.exist("output"))
    cout << endl; // mark the end of the classifier if piping
  else
    fout.close();
}
