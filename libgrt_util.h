#ifndef _CSVIO_H_
#define _CSVIO_H_

#include "cmdline.h"
#include <GRT.h>
#include <iostream>
#include <climits>
#include <locale> // for isspace
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

using namespace GRT;
using namespace std;

#define csvio_dispatch(io,func,args...) switch(io.type) {\
  case TIMESERIES:\
    func( io.t_data, ##args );\
    break;\
  case CLASSIFICATION:\
    func( io.c_data, ##args );\
    break;\
  default:\
    throw invalid_argument("unknown data type in dispatch");\
    break;\
}

//typedef enum { TIMESERIES, UNLABELLED, REGRESSION, CLASSIFICATION, UNKNOWN } csv_type_t;
typedef enum { TIMESERIES, CLASSIFICATION, UNKNOWN } csv_type_t;

class CsvIOSample {
  public:
    csv_type_t type;
    UnlabelledData u_data;
    RegressionData r_data;
    ClassificationSample c_data;
    TimeSeriesClassificationSample t_data;
    std::vector<string> labelset;
    bool has_NULL_label;
    int linenum;

    static bool iscomment(std::string line) {
      for (int i=0; i<line.length(); i++) {
        char c = line[i];
        if (std::isspace(c))
          continue;
        return c=='#';
      }
    }

    static bool isempty(std::string line) {
      for (int i=0; i<line.length(); i++) {
        char c=line[i];
        if (!std::isspace(c))
          return false;
      }
      return true;
    }

    int classkey(std::string label) {
      int i = std::find(labelset.begin(), labelset.end(), label) == labelset.end() ? -1 :
              std::find(labelset.begin(), labelset.end(), label) - labelset.begin();
      if (i==-1) {
        labelset.push_back(label);
        return labelset.size()-1;
      }
      if (!has_NULL_label && i==0)
        has_NULL_label = true;
      return i;
    }

    friend std::istream& operator>> (std::istream &in, CsvIOSample &o) 
    {
      using namespace std;

      string line, label;
      vector<vector<double>> data;
      double d;

      while (getline(in,line)) {
        stringstream ss(line);
        o.linenum++;

        if (line == "") {
          if (data.size()!=0)
            break;
          else
            continue;
        }

        if (line[0] == '#') {
          if (o.type==UNKNOWN) o.settype(line);
          continue;
        }

        if (o.type==UNKNOWN)
          o.type = CLASSIFICATION; // default to classificaion

        ss >> label;
        vector<double> sample;
        while (ss >> d)
          sample.push_back(d);

        if (sample.size() == 0)
          continue;

        data.push_back(sample);

        if (o.type!=TIMESERIES)
          break;
      }

      if (data.size() > 0) {
        switch(o.type) {
        case TIMESERIES: {
          MatrixDouble md(data.size(), data.back().size());
          md = data;
          o.t_data = TimeSeriesClassificationSample(o.classkey(label), md);
          break; }
        case CLASSIFICATION:
          o.c_data = ClassificationSample(o.classkey(label), data.back());
          break;
        default:
          throw invalid_argument("unknown data type");
        }
      }

      if (data.size() != 0) in.clear();
      return in;
    }

    CsvIOSample(const std::string &t) {
      settype(t);
      linenum = 0;
      has_NULL_label = false;
      labelset.push_back("NULL");
    }

  protected:
  void settype(const std::string &t) {
    using namespace std;
    if (t.find("classification") != string::npos)
      type = CLASSIFICATION;
      //else if ("unlabelled" == type)
      //  o.type = UNLABELLED;
      //else if ("regression" == type)
      //  type = REGRESSION;
    else if (t.find("timeseries") != string::npos)
      type =  TIMESERIES;
    else
      type = UNKNOWN;
  }
};

class CollectDataset
{
  public:
  TimeSeriesClassificationData t_data;
  ClassificationData c_data;
  csv_type_t type;

  CollectDataset() {
    t_data.setAllowNullGestureClass(true);
    c_data.setAllowNullGestureClass(true);
    type = UNKNOWN;
  }

  bool add(TimeSeriesClassificationSample &sample, vector<string> &labels) {
    type = TIMESERIES;
    UINT cl = sample.getClassLabel();

    if (t_data.getNumDimensions() == 0)
      t_data.setNumDimensions(sample.getData().getNumCols());

    if (!t_data.addSample(sample.getClassLabel(), sample.getData()))
      return false;
    t_data.setClassNameForCorrespondingClassLabel(labels[cl], cl);
    return true;
  }

  bool add(ClassificationSample &sample, vector<string> &labels) {
    type = CLASSIFICATION;
    UINT cl = sample.getClassLabel();

    if (c_data.getNumDimensions() == 0)
      c_data.setNumDimensions(sample.getSample().size());

    if (!c_data.addSample(sample.getClassLabel(), sample.getSample()))
      return false;
    c_data.setClassNameForCorrespondingClassLabel(labels[cl], cl);
    return true;
  }

  std::string getStatsAsString() {
    switch(type) {
    case TIMESERIES:
      return t_data.getStatsAsString();
    case CLASSIFICATION:
      return c_data.getStatsAsString();
    default:
      return "unknown datatype";
    }
  }
};

class CerrLogger : public Observer< GRT::TrainingLogMessage >,
                   public Observer< GRT::TestingLogMessage >,
                   public Observer< GRT::WarningLogMessage >,
                   public Observer< GRT::DebugLogMessage >,
                   public Observer< GRT::ErrorLogMessage >,
                   public Observer< GRT::InfoLogMessage >
{
  void notify(TrainingLogMessage &msg) {
    cerr << msg.getMessage() << endl;
  }

  void notify(TestingLogMessage &msg) {
    cerr << msg.getMessage() << endl;
  }

  void notify(DebugLogMessage &msg) {
    cerr << msg.getMessage() << endl;
  }

  void notify(InfoLogMessage &msg) {
    cerr << msg.getMessage() << endl;
  }

  void notify(WarningLogMessage &msg) {
    cerr << msg.getMessage() << endl;
  }

  void notify(ErrorLogMessage &msg) {
    cerr << msg.getMessage() << endl;
  }
};

static bool is_running = true;
  /* enable logging output */
void set_verbosity(int level) {
  static CerrLogger _cerr;
  DebugLog::enableLogging(false);
  TrainingLog::enableLogging(false);
  TestingLog::enableLogging(false);
  InfoLog::enableLogging(false);
  WarningLog::enableLogging(false);
  ErrorLog::enableLogging(false);
  //DebugLog::registerObserver( _cerr );
  //TrainingLog::registerObserver(_cerr);
  //TestingLog::registerObserver(_cerr);
  //InfoLog::registerObserver(_cerr);
  //WarningLog::registerObserver(_cerr);
  //ErrorLog::registerObserver(_cerr);
  switch(level) {
      case 4: DebugLog::enableLogging(true);
      case 3: TestingLog::enableLogging(true); TrainingLog::enableLogging(true);
      case 2: InfoLog::enableLogging(true);
      case 1: WarningLog::enableLogging(true);
      case 0: ErrorLog::enableLogging(true);
  }
}


static bool *is_running_indicator;

void sig_callback(int signum) {
  if (is_running_indicator)
    *is_running_indicator = false;
}

static void set_running_indicator(bool *is_running_ind)
{
    struct sigaction action;
    is_running_indicator = is_running_ind;
    action.sa_handler = sig_callback;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}

/* we just try to load with every avail classifier */
FeatureExtraction *loadFeatureExtractionFromFile(string &file) {
  FeatureExtraction *feature = NULL;
  ErrorLog::enableLogging(false);
  ErrorLog err;

  for (string c : FeatureExtraction::getRegisteredFeatureExtractors()) {
    feature = FeatureExtraction::createInstanceFromString(c);
    if (feature->loadModelFromFile(file))
      break;
    if (feature != NULL) delete feature;
    feature = NULL;
  }

  ErrorLog::enableLogging(true);
  return feature;
}

Classifier *loadClassifierFromFile(string &file)
{
  ErrorLog err;
  Classifier *classifier = NULL;
  bool errlog = err.getLoggingEnabled();
  ErrorLog::enableLogging(false);

  for (string c : Classifier::getRegisteredClassifiers()) {
    classifier = Classifier::createInstanceFromString(c);
    if (classifier->loadModelFromFile(file))
      break;
    if (classifier == NULL) delete classifier;
    classifier = NULL;
  }

  ErrorLog::enableLogging(true);
  return classifier;
}

istream&
grt_fileinput(cmdline::parser &c, int num=0) {
  static ifstream inf;
  string filename = c.rest().size() > num ? c.rest()[num] : "-";

  if (filename=="-")
    return cin;

  inf.open(filename);
  if(!inf) cerr << "unable to open file: " << filename << endl;
  return inf;
}


#endif
