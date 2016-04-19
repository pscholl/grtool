#include <dlib/svm_threaded.h>

#include <iostream>
#include <stdio.h>
#include "cmdline.h"

using namespace std;
using namespace dlib;

typedef matrix<double, 6, 1> sample_type;

int main(int argc, const char *argv[])
{
  string input_file = "-";
  cmdline::parser c;

  c.add        ("help",    'h', "print this message");
  c.add<string>("output",  'o', "store trained classifier in file", false);
  c.add<string>("trainset",'n', "limit the training dataset to the first n samples, if n is less than or equal 1 it is interpreted the percentage of a stratified random split that is retained for training. If not a number it is interpreted as a filename containing training samples.", false, "1");
  c.footer     ("<classifier> [input-data]...");

  /* parse common arguments */
  bool parse_ok = c.parse(argc, argv, false) && !c.exist("help");

  if (!parse_ok) {
    cerr << c.usage() << endl << c.error() << endl;
    return -1;
  }

  /* check if we can open the output file */
  ofstream test(c.get<string>("output"), ios_base::out);
  ostream &output = c.exist("output") ? test : cout;

  if (c.exist("output") && !test.good()) {
    cerr << c.usage() << endl << "unable to open \"" << c.get<string>("output") << "\" as output" << endl;
    return -1;
  }

  if (c.rest().size() > 0)
    input_file = c.rest()[0];

  /* do we read from a file or stdin? */
  ifstream fin; fin.open(input_file);
  istream &in = input_file=="-" ? cin : fin;

  if (!in.good()) {
    cerr << "unable to open input file " << input_file << endl;
    return -1;
  }

  std::vector<sample_type> samples;
  std::vector<string> labels;

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
  }

  for (size_t i = 0; i < labels.size(); ++i)
    output << "Label: " << labels[i] << " | Values: " << endl << samples[i] << endl;

}
