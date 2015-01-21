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

int main(int argc, const char *argv[]) {
  static bool is_running = true;
  cmdline::parser c;

  c.add<int>   ("verbose",    'v', "verbosity level: 0-4", false, 0);
  c.add        ("help",       'h', "print this message");
  c.footer     ("<pre-processor> [<filename>] ");

  if (!c.parse(argc,argv) || c.exist("help")) {
    cerr << c.usage() << endl << c.error() << endl;
    exit(-1);
  }

  set_verbosity(c.get<int>("verbose"));

  string preproc_name = c.rest().size() > 0 ? c.rest()[0] : "list";
  PreProcessing *pp = PreProcessing::createInstanceFromString( preproc_name );

  if (preproc_name == "list") {
    cout << c.usage() << endl;
    cout << list_preprocessors();
    exit(0);
  }

  if (pp == NULL) {
    cout << c.usage() << endl;
    cout << list_preprocessors() << endl;
    cerr << "unable to load preprocessor " << preproc_name << endl;
    exit(-1);
  }

  /* do we read from a file or from stdin-? */
  string filename = c.rest().size() > 1 ? c.rest()[1] : "-";
  ifstream inf(filename);
  if (filename!="-" && !inf) {
    cerr << "unable to open file: " << filename << endl;
    return -1;
  }
  istream &in = filename != "-" ? inf : cin;

  string line; int linenum;
  while(getline(in,line)) {
    stringstream ss(line);

    if (line[0] == '#')
      continue;

    if (line.size() == 0 ) {
      cout << endl;
      continue;
    }

    try { string label; ss >> label; cout << label << "\t"; }
    catch (exception &e) { /* unlabeled data */ }

    vector<double> vals; double value;
    while (ss >> value)
      vals.push_back(value);

    if (linenum == 0)
      pp->setNumInputDimensions(vals.size());

    bool ok = pp->process(vals);
    if (!ok) {
      cerr << "unable to process line " << linenum << endl;
      exit(-1);
    }

    for(auto value : vals)
      cout << value << "\t";

    cout << endl;

    linenum++;
  }
}

// TODO add parameters for preprocessing modules
